#include "AudioSanitizer.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

std::atomic<int64> FSubmixEffectSanitizer::BadSamples{0};

void FSubmixEffectSanitizer::OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData)
{
	const float* In = InData.AudioBuffer->GetData();
	float* Out = OutData.AudioBuffer->GetData();
	const int32 Num = FMath::Min(InData.AudioBuffer->Num(), OutData.AudioBuffer->Num());
	int64 Bad = 0;
	for (int32 i = 0; i < Num; ++i)
	{
		const float Sample = In[i];
		if (FMath::IsFinite(Sample))
		{
			Out[i] = Sample;
		}
		else
		{
			Out[i] = 0.f;
			++Bad;
		}
	}
	if (Bad > 0)
	{
		BadSamples += Bad;
	}
}

bool UAudioSanitizerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UAudioSanitizerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	Preset = NewObject<USubmixEffectSanitizerPreset>(this);
	UAudioMixerBlueprintLibrary::AddMasterSubmixEffect(&InWorld, Preset);
	FSubmixEffectSanitizer::BadSamples = 0;

	// Occlusion des sons UDS (traces pour les interieurs) inutile en plein air ; ses valeurs au
	// demarrage, avant le chargement des tuiles Cesium, sont suspectes de produire les NaN.
	// Coupee avant le BeginPlay des acteurs (variable Blueprint, retrouvee par son nom affiche).
	for (TActorIterator<AActor> It(&InWorld); It; ++It)
	{
		if (!It->GetClass()->GetName().StartsWith(TEXT("Ultra_Dynamic_Weather")))
		{
			continue;
		}
		for (TFieldIterator<FBoolProperty> PropIt(It->GetClass()); PropIt; ++PropIt)
		{
			if (PropIt->GetAuthoredName() == TEXT("Use Occlusion to Attenuate Sounds in Interiors"))
			{
				PropIt->SetPropertyValue_InContainer(*It, false);
				UE_LOG(LogTemp, Log, TEXT("[Audio] Occlusion des sons UDS coupee sur %s"), *It->GetName());
			}
		}
	}

	InWorld.GetTimerManager().SetTimer(CheckTimer, FTimerDelegate::CreateUObject(this, &UAudioSanitizerSubsystem::CheckNaN), 1.f, true);
}

static FString SoundName(const UAudioComponent* Comp)
{
	return Comp ? FString::Printf(TEXT("%s (son %s)"), *Comp->GetPathName(), Comp->Sound ? *Comp->Sound->GetName() : TEXT("aucun")) : TEXT("?");
}

void UAudioSanitizerSubsystem::CheckNaN()
{
	const int64 Bad = FSubmixEffectSanitizer::BadSamples.exchange(0);
	if (Bad == 0)
	{
		if (Phase == ENaNHuntPhase::Restart || Phase == ENaNHuntPhase::Stop)
		{
			UAudioComponent* Culprit = Suspects.IsValidIndex(Step - 1) ? Suspects[Step - 1].Get() : nullptr;
			UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN disparus apres %s de %s"),
				Phase == ENaNHuntPhase::Restart ? TEXT("relance") : TEXT("coupure"), *SoundName(Culprit));
			if (Phase == ENaNHuntPhase::Stop)
			{
				// Les sons coupes avant le fautif etaient innocents : on les relance
				for (int32 i = 0; i < Step - 1; ++i)
				{
					if (UAudioComponent* Comp = Suspects[i].Get())
					{
						Comp->Play();
					}
				}
			}
		}
		if (Phase != ENaNHuntPhase::Idle)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Audio] Mix general sain"));
		}
		Phase = ENaNHuntPhase::Idle;
		return;
	}

	if (Phase == ENaNHuntPhase::Idle)
	{
		// Sons en cours de lecture, dans un ordre stable
		TArray<UAudioComponent*> Playing;
		for (TObjectIterator<UAudioComponent> It; It; ++It)
		{
			if (It->GetWorld() == GetWorld() && It->IsPlaying())
			{
				Playing.Add(*It);
			}
		}
		Playing.Sort([](const UAudioComponent& A, const UAudioComponent& B) { return A.GetPathName() < B.GetPathName(); });
		Suspects.Reset();
		for (UAudioComponent* Comp : Playing)
		{
			Suspects.Add(Comp);
		}
		Phase = ENaNHuntPhase::Restart;
		Step = 0;
		UE_LOG(LogTemp, Warning, TEXT("[Audio] %lld echantillons NaN/Inf dans le mix general, %d sons actifs : recherche de la source"), Bad, Suspects.Num());
	}

	if (Phase == ENaNHuntPhase::Restart)
	{
		if (Step < Suspects.Num())
		{
			UAudioComponent* Comp = Suspects[Step++].Get();
			UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN : relance de %s"), *SoundName(Comp));
			if (Comp)
			{
				Comp->Stop();
				Comp->Play();
			}
			return;
		}
		Phase = ENaNHuntPhase::Stop;
		Step = 0;
	}

	if (Phase == ENaNHuntPhase::Stop)
	{
		if (Step < Suspects.Num())
		{
			UAudioComponent* Comp = Suspects[Step++].Get();
			UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN : coupure de %s"), *SoundName(Comp));
			if (Comp)
			{
				Comp->Stop();
			}
			return;
		}
		// Tous les sons coupes et toujours des NaN : volume global (sound mix) suspect
		UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN malgre tous les sons coupes : vidage des sound mix"));
		UGameplayStatics::ClearSoundMixModifiers(GetWorld());
		for (const TWeakObjectPtr<UAudioComponent>& Weak : Suspects)
		{
			if (UAudioComponent* Comp = Weak.Get())
			{
				Comp->Play();
			}
		}
		Phase = ENaNHuntPhase::Done;
		LastDoneLogTime = FPlatformTime::Seconds();
		return;
	}

	// Done : source introuvable, on se contente de signaler toutes les 10 s
	const double Now = FPlatformTime::Seconds();
	if (Now - LastDoneLogTime > 10.0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN persistants dans le mix general (%lld/s), source non trouvee"), Bad);
		LastDoneLogTime = Now;
	}
}

void UAudioSanitizerSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CheckTimer);
		if (Preset)
		{
			UAudioMixerBlueprintLibrary::RemoveMasterSubmixEffect(World, Preset);
		}
	}
	Preset = nullptr;
	Super::Deinitialize();
}
