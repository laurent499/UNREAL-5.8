#include "AudioSanitizer.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

std::atomic<int64> FSubmixEffectSanitizer::BadSamples{0};
std::atomic<int64> FSubmixEffectSanitizer::ClippedSamples{0};

void FSubmixEffectSanitizer::OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData)
{
	const float* In = InData.AudioBuffer->GetData();
	float* Out = OutData.AudioBuffer->GetData();
	const int32 Num = FMath::Min(InData.AudioBuffer->Num(), OutData.AudioBuffer->Num());
	int64 Bad = 0;
	int64 Clipped = 0;
	for (int32 i = 0; i < Num; ++i)
	{
		const float Sample = In[i];
		if (FMath::IsFinite(Sample))
		{
			// Ecretage doux au-dela de 0,9 : evite la saturation dure quand pluie, vent et ambiance
			// se cumulent (l'encodeur et la sortie ecretent net a 1)
			const float Abs = FMath::Abs(Sample);
			if (Abs > 0.9f)
			{
				++Clipped;
				Out[i] = FMath::Sign(Sample) * (0.9f + 0.1f * FMath::Tanh((Abs - 0.9f) / 0.1f));
			}
			else
			{
				Out[i] = Sample;
			}
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
	if (Clipped > 0)
	{
		ClippedSamples += Clipped;
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

	InWorld.GetTimerManager().SetTimer(CheckTimer, FTimerDelegate::CreateUObject(this, &UAudioSanitizerSubsystem::CheckNaN), 0.25f, true);
}

static FString SoundName(const UAudioComponent* Comp)
{
	return Comp ? FString::Printf(TEXT("%s (son %s)"), *Comp->GetPathName(), Comp->Sound ? *Comp->Sound->GetName() : TEXT("aucun")) : TEXT("?");
}

// Ambiance foret UDS (MetaSound Forest_Example) : ses entrees « Bird/Insects Time Volumes » donnent
// un volume par moment (avant l'aube, matin, midi, soir, apres le crepuscule, nuit), mais UDS lui
// transmet toujours « nuit » dans ce projet (grillons et pas d'oiseaux, meme a midi). On calcule donc
// le volume pour l'heure reelle du ciel UDS et on le pousse a l'identique dans les 6 cases.
static float TimeVolume(double Hour, const float (&Levels)[6])
{
	// Centre de chaque moment (heures) ; interpolation lineaire entre deux moments voisins
	static const double Centers[6] = {5.0, 8.5, 13.5, 17.5, 20.0, 1.0 + 24.0};
	const double H = Hour < Centers[0] ? Hour + 24.0 : Hour;
	for (int32 i = 0; i < 6; ++i)
	{
		const int32 j = (i + 1) % 6;
		const double C0 = Centers[i];
		const double C1 = j == 0 ? Centers[0] + 24.0 : Centers[j];
		if (H >= C0 && H < C1)
		{
			return FMath::Lerp(Levels[i], Levels[j], float((H - C0) / (C1 - C0)));
		}
	}
	return Levels[5];
}

static void ApplyForestVolumes(UWorld* World)
{
	// Heure du ciel UDS (variable Blueprint TimeOfDay, 0-2400)
	double TimeOfDay = -1.0;
	for (TActorIterator<AActor> It(World); It && TimeOfDay < 0.0; ++It)
	{
		if (It->GetClass()->GetName().StartsWith(TEXT("Ultra_Dynamic_Sky")))
		{
			if (const FDoubleProperty* Prop = FindFProperty<FDoubleProperty>(It->GetClass(), TEXT("TimeOfDay")))
			{
				TimeOfDay = Prop->GetPropertyValue_InContainer(*It);
			}
		}
	}
	if (TimeOfDay < 0.0)
	{
		return;
	}
	const double Hour = FMath::Fmod(TimeOfDay / 100.0, 24.0);

	// Avant l'aube, matin, midi, soir, apres le crepuscule, nuit
	static const float BirdLevels[6] = {0.6f, 2.f, 1.6f, 1.2f, 0.2f, 0.f};
	static const float InsectLevels[6] = {0.4f, 0.f, 0.f, 0.2f, 1.f, 1.f};
	TArray<float> Birds, Insects;
	Birds.Init(TimeVolume(Hour, BirdLevels), 6);
	Insects.Init(TimeVolume(Hour, InsectLevels), 6);

	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		if (It->GetWorld() == World && It->Sound && It->Sound->GetName() == TEXT("Forest_Example"))
		{
			It->SetParameters({
				FAudioParameter(TEXT("Bird Time Volumes"), Birds),
				FAudioParameter(TEXT("Insects Time Volumes"), Insects)});
		}
	}
}

void UAudioSanitizerSubsystem::CheckNaN()
{
	// Toutes les 2 s : reglages de l'ambiance foret (UDS peut recreer son composant)
	if (++ForestTick % 8 == 1)
	{
		ApplyForestVolumes(GetWorld());
	}

	const int64 Bad = FSubmixEffectSanitizer::BadSamples.exchange(0);
	const double Now = FPlatformTime::Seconds();

	// Saturation : un log toutes les 10 s au plus
	ClippedSinceLog += FSubmixEffectSanitizer::ClippedSamples.exchange(0);
	if (ClippedSinceLog > 0 && Now - LastClipLogTime > 10.0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Audio] %lld echantillons au-dela de 0,9 adoucis (mix trop fort)"), ClippedSinceLog);
		ClippedSinceLog = 0;
		LastClipLogTime = Now;
	}

	// Relance du fautif deja trouve : s'il ressort des NaN, on le recoupe et on attend plus longtemps
	if (bCulpritRetrying)
	{
		bCulpritRetrying = false;
		if (Bad > 0)
		{
			if (UAudioComponent* Comp = Culprit.Get())
			{
				Comp->Stop();
			}
			CulpritRetryDelay *= 2.0;
			CulpritRetryTime = Now + CulpritRetryDelay;
			UE_LOG(LogTemp, Warning, TEXT("[Audio] %s produit encore des NaN : recoupe, nouvel essai dans %.0f s"), *SoundName(Culprit.Get()), CulpritRetryDelay);
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[Audio] %s relance sans NaN"), *SoundName(Culprit.Get()));
		Culprit = nullptr;
	}

	if (Bad == 0)
	{
		if (Phase == ENaNHuntPhase::Stop)
		{
			UAudioComponent* Found = Suspects.IsValidIndex(Step - 1) ? Suspects[Step - 1].Get() : nullptr;
			UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN disparus apres coupure de %s"), *SoundName(Found));
			// Les sons coupes avant le fautif etaient innocents : on les relance
			for (int32 i = 0; i < Step - 1; ++i)
			{
				if (UAudioComponent* Comp = Suspects[i].Get())
				{
					Comp->Play();
				}
			}
			Culprit = Found;
			CulpritRetryDelay = 10.0;
			CulpritRetryTime = Now + CulpritRetryDelay;
		}
		Phase = ENaNHuntPhase::Idle;

		if (Culprit.IsValid() && Now >= CulpritRetryTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Audio] Nouvel essai de %s"), *SoundName(Culprit.Get()));
			Culprit->Play();
			bCulpritRetrying = true;
		}
		return;
	}

	if (Phase == ENaNHuntPhase::Idle)
	{
		// Sons en cours de lecture, le son directionnel UDS en premier, puis ordre stable
		TArray<UAudioComponent*> Playing;
		for (TObjectIterator<UAudioComponent> It; It; ++It)
		{
			if (It->GetWorld() == GetWorld() && It->IsPlaying())
			{
				Playing.Add(*It);
			}
		}
		Playing.Sort([](const UAudioComponent& A, const UAudioComponent& B)
		{
			const bool bA = A.GetName() == TEXT("Sound_Directional");
			const bool bB = B.GetName() == TEXT("Sound_Directional");
			return bA != bB ? bA : A.GetPathName() < B.GetPathName();
		});
		Suspects.Reset();
		for (UAudioComponent* Comp : Playing)
		{
			Suspects.Add(Comp);
		}
		Phase = ENaNHuntPhase::Stop;
		Step = 0;
		UE_LOG(LogTemp, Warning, TEXT("[Audio] %lld echantillons NaN/Inf dans le mix general, %d sons actifs : recherche de la source"), Bad, Suspects.Num());
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
		LastDoneLogTime = Now;
		return;
	}

	// Done : source introuvable, on se contente de signaler toutes les 10 s
	if (Now - LastDoneLogTime > 10.0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN persistants dans le mix general, source non trouvee"));
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
