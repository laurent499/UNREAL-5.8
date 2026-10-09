#include "AudioSanitizer.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "OWLMediaOutput.h"
#include "Misc/OutputDeviceRedirector.h"

// Repere dans le log les erreurs d'encodage audio OWL « Input contains (near) NaN/+-Inf »
struct FOWLNaNLogWatcher : public FOutputDevice
{
	std::atomic<bool> bSeen{false};

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		static const FName OWLCategory(TEXT("LogOWLMedia"));
		if (Category == OWLCategory && FCString::Strstr(V, TEXT("NaN")))
		{
			bSeen = true;
		}
	}
	virtual bool CanBeUsedOnAnyThread() const override { return true; }
	virtual bool CanBeUsedOnMultipleThreads() const override { return true; }
};

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

	OWLLogWatcher = new FOWLNaNLogWatcher();
	GLog->AddOutputDevice(OWLLogWatcher);

	InWorld.GetTimerManager().SetTimer(CheckTimer, FTimerDelegate::CreateUObject(this, &UAudioSanitizerSubsystem::CheckNaN), 1.f, true);
}

void UAudioSanitizerSubsystem::CheckNaN()
{
	// Erreurs NaN de l'encodeur OWL : relance de la sortie (5 fois max, 5 s mini entre deux)
	if (OWLLogWatcher && OWLLogWatcher->bSeen.exchange(false))
	{
		const double Now = FPlatformTime::Seconds();
		if (OWLRestarts < 5 && Now - LastOWLRestartTime > 5.0)
		{
			LastOWLRestartTime = Now;
			RestartOWLOutput();
		}
	}

	const int64 Bad = FSubmixEffectSanitizer::BadSamples.exchange(0);
	if (Bad == 0)
	{
		if (LastRestarted.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Audio] NaN disparus apres relance de %s (son %s)"),
				*LastRestarted->GetPathName(), LastRestarted->Sound ? *LastRestarted->Sound->GetName() : TEXT("aucun"));
			LastRestarted = nullptr;
			RestartIndex = 0;
		}
		return;
	}

	// Composants audio en cours de lecture, dans un ordre stable
	TArray<UAudioComponent*> Playing;
	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		if (It->GetWorld() == GetWorld() && It->IsPlaying())
		{
			Playing.Add(*It);
		}
	}
	Playing.Sort([](const UAudioComponent& A, const UAudioComponent& B) { return A.GetPathName() < B.GetPathName(); });
	if (Playing.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Audio] %lld echantillons NaN/Inf dans le mix general, aucun composant audio actif"), Bad);
		return;
	}

	UAudioComponent* Comp = Playing[RestartIndex % Playing.Num()];
	++RestartIndex;
	UE_LOG(LogTemp, Warning, TEXT("[Audio] %lld echantillons NaN/Inf dans le mix general : relance de %s (son %s)"),
		Bad, *Comp->GetPathName(), Comp->Sound ? *Comp->Sound->GetName() : TEXT("aucun"));
	Comp->Stop();
	Comp->Play();
	LastRestarted = Comp;
}

void UAudioSanitizerSubsystem::RestartOWLOutput()
{
	AOWLMediaOutput* Output = Cast<AOWLMediaOutput>(UGameplayStatics::GetActorOfClass(GetWorld(), AOWLMediaOutput::StaticClass()));
	if (!Output)
	{
		return;
	}
	++OWLRestarts;
	UE_LOG(LogTemp, Warning, TEXT("[Audio] Encodeur audio OWL bloque par des NaN : relance de la sortie OWL (%d/5)"), OWLRestarts);
	Output->Stop();
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(Output, [Output]()
	{
		Output->Start();
	}), 1.f, false);
}

void UAudioSanitizerSubsystem::Deinitialize()
{
	if (OWLLogWatcher)
	{
		GLog->RemoveOutputDevice(OWLLogWatcher);
		delete OWLLogWatcher;
		OWLLogWatcher = nullptr;
	}
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
