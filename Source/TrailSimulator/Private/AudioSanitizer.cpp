#include "AudioSanitizer.h"
#include "CesiumGeoreference.h"
#include "AudioDevice.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundSubmix.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

std::atomic<int64> FSubmixEffectSanitizer::BadSamples{0};
std::atomic<int64> FSubmixEffectSanitizer::ClippedSamples{0};
std::atomic<float> FSubmixEffectSanitizer::TargetGain{1.f};

// Ouverture du son : instant de la premiere teleportation et derniers NaN vus (un seul monde de jeu a la fois)
static double MovedTime = 0.0;
static double LastBadTime = 0.0;
static bool bHasLastPose = false;
static FVector LastCamPos = FVector::ZeroVector;
// Gain du mix une fois ouvert : le mix UDS sature souvent (pluie + vent + ambiance)
static constexpr float OpenGain = 0.7f;

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
	// Gain general (son coupe avant la premiere course), rampe lineaire sur le tampon
	const float Target = TargetGain.load();
	if (CurrentGain != Target || Target != 1.f)
	{
		const float Step = Target < CurrentGain ? Target - CurrentGain : FMath::Min(Target - CurrentGain, 0.02f);
		const int32 NumChannels = FMath::Max(1, InData.NumChannels);
		const int32 Frames = Num / NumChannels;
		for (int32 f = 0; f < Frames; ++f)
		{
			const float G = CurrentGain + Step * float(f) / float(FMath::Max(1, Frames));
			for (int32 ch = 0; ch < NumChannels; ++ch)
			{
				Out[f * NumChannels + ch] *= G;
			}
		}
		CurrentGain += Step;
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

// Coupe tout le son du monde des sa creation : les sons UDS demarrent a l'enregistrement de leurs
// composants, avant tout BeginPlay, et buzzent 1 a 2 s a la position de depart
static void SetWorldMute(UWorld* World, bool bMute)
{
	if (World)
	{
		if (FAudioDeviceHandle Device = World->GetAudioDevice())
		{
			Device->SetTransientPrimaryVolume(bMute ? 0.f : 1.f);
		}
	}
}

void UAudioSanitizerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SetWorldMute(GetWorld(), true);
}

void UAudioSanitizerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	Preset = NewObject<USubmixEffectSanitizerPreset>(this);
	UAudioMixerBlueprintLibrary::AddMasterSubmixEffect(&InWorld, Preset);
	// Le submix general reinjecte une part du signal brut (« dry », -96 dB) apres les effets : avec
	// des valeurs aberrantes (1e6) ou des NaN, ce petit reste suffisait a buzzer et a bloquer OWL.
	// Dry a 0 : seule la sortie du filtre est entendue et envoyee au flux.
	if (USoundSubmix* Master = Cast<USoundSubmix>(GetDefault<UAudioSettings>()->MasterSubmix.TryLoad()))
	{
		Master->SetSubmixDryLevel(&InWorld, 0.f);
	}
	FSubmixEffectSanitizer::BadSamples = 0;

	// Avant la premiere course, la camera est a sa position de depart (hors tuiles Cesium) et les
	// sons UDS y buzzent : son coupe jusqu'a ce que l'origine Cesium change (teleportation)
	FSubmixEffectSanitizer::TargetGain = 0.f;
	bSoundOpened = false;
	BeginPlayTime = FPlatformTime::Seconds();
	MovedTime = 0.0;
	LastBadTime = 0.0;
	bHasLastPose = false;
	if (ACesiumGeoreference* Geo = ACesiumGeoreference::GetDefaultGeoreference(&InWorld))
	{
		InitialOrigin = Geo->GetOriginLongitudeLatitudeHeight();
	}
	UE_LOG(LogTemp, Log, TEXT("[Audio] Son coupe jusqu'a la premiere course"));

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
		// L'ambiance d'UDS se trompe de moment de la journee (22:51 = matin) : on la debranche
		// d'UDS et on la joue nous-memes (voir UpdateForest)
		for (TFieldIterator<FObjectPropertyBase> PropIt(It->GetClass()); PropIt; ++PropIt)
		{
			if (PropIt->GetAuthoredName() == TEXT("Environment Sound"))
			{
				PropIt->SetObjectPropertyValue_InContainer(*It, nullptr);
				UE_LOG(LogTemp, Log, TEXT("[Audio] Ambiance UDS debranchee de %s (jouee par le jeu)"), *It->GetName());
			}
		}
	}

	InWorld.GetTimerManager().SetTimer(CheckTimer, FTimerDelegate::CreateUObject(this, &UAudioSanitizerSubsystem::CheckNaN), 0.25f, true);
}

static FString SoundName(const UAudioComponent* Comp)
{
	return Comp ? FString::Printf(TEXT("%s (son %s)"), *Comp->GetPathName(), Comp->Sound ? *Comp->Sound->GetName() : TEXT("aucun")) : TEXT("?");
}

// Ambiance foret (MetaSound UDS Forest_Example), jouee par le jeu et non par UDS. Son entree
// « Time » est le moment de la journee : 0 avant l'aube, 1 matin, 2 midi, 3 soir, 4 apres le
// crepuscule, 5 nuit ; « Time Interp » la duree du fondu (s). Calcule d'apres l'heure du ciel UDS.
static constexpr float ForestVolume = 0.5f;
static const TCHAR* ForestSoundPath = TEXT("/Game/UltraDynamicSky/Sound/Environment/Forest_Example/Forest_Example.Forest_Example");

static int32 DayPhase(double Hour)
{
	if (Hour >= 4.5 && Hour < 6.5) return 0;
	if (Hour >= 6.5 && Hour < 11.0) return 1;
	if (Hour >= 11.0 && Hour < 16.0) return 2;
	if (Hour >= 16.0 && Hour < 18.5) return 3;
	if (Hour >= 18.5 && Hour < 20.5) return 4;
	return 5;
}

void UAudioSanitizerSubsystem::UpdateForest()
{
	UWorld* World = GetWorld();
	// Heure du ciel UDS (variable Blueprint TimeOfDay, heures x 100)
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
	const int32 DayIndex = DayPhase(Hour);

	// UDS a pu lancer sa propre ambiance avant qu'on la debranche (BeginPlay des acteurs passe
	// avant celui du sous-systeme) : on arrete tout autre composant qui la joue
	for (TObjectIterator<UAudioComponent> It; It; ++It)
	{
		if (*It != ForestComp && It->GetWorld() == World && It->Sound && It->Sound->GetName() == TEXT("Forest_Example") && It->IsPlaying())
		{
			UE_LOG(LogTemp, Log, TEXT("[Audio] Ambiance UDS arretee : %s"), *It->GetPathName());
			It->Stop();
		}
	}

	if (!ForestComp)
	{
		USoundBase* Sound = LoadObject<USoundBase>(nullptr, ForestSoundPath);
		if (!Sound)
		{
			return;
		}
		ForestComp = UGameplayStatics::SpawnSound2D(World, Sound, ForestVolume);
		LastForestPhase = -1;
	}
	if (ForestComp && DayIndex != LastForestPhase)
	{
		// Premier reglage quasi instantane, ensuite fondu de 10 s (jamais 0 : le fondu de la
		// MetaSound divise par cette duree et sortait des NaN, ambiance entiere en NaN pour la session)
		ForestComp->SetParameters({
			FAudioParameter(TEXT("Time Interp"), LastForestPhase < 0 ? 0.5f : 10.f),
			FAudioParameter(TEXT("Time"), float(DayIndex))});
		static const TCHAR* Names[6] = {TEXT("avant l'aube"), TEXT("matin"), TEXT("midi"), TEXT("soir"), TEXT("apres le crepuscule"), TEXT("nuit")};
		UE_LOG(LogTemp, Log, TEXT("[Audio] Ambiance foret : %02d:%02d (ciel UDS) -> %s"),
			int32(Hour), int32(FMath::Fmod(Hour, 1.0) * 60.0), Names[DayIndex]);
		LastForestPhase = DayIndex;
	}
}

void UAudioSanitizerSubsystem::CheckNaN()
{
	// Toutes les 2 s : moment de la journee de l'ambiance foret
	if (++ForestTick % 8 == 1)
	{
		UpdateForest();
	}

	const int64 Bad = FSubmixEffectSanitizer::BadSamples.exchange(0);
	const double Now = FPlatformTime::Seconds();

	// Teleportation (changement d'origine Cesium ou saut de camera de plus de 2 km) : les sons UDS
	// buzzent quelques secondes (position aberrante, tuiles en chargement). Son coupe net, puis rouvert
	// en fondu 4 s apres, une fois 2 s passees sans NaN. Aussi au lancement, jusqu'a la 1re course.
	{
		UWorld* World = GetWorld();
		// Position geographique de la camera : l'origine Cesium se decale en continu avec la camera
		// (OriginShift), seul un saut reel de plus de 2 km entre deux verifications compte
		const ACesiumGeoreference* Geo = ACesiumGeoreference::GetDefaultGeoreference(World);
		const APlayerCameraManager* Cam = World ? UGameplayStatics::GetPlayerCameraManager(World, 0) : nullptr;
		bool bJump = false;
		if (Geo && Cam)
		{
			const FVector LLH = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(Cam->GetCameraLocation());
			if (bHasLastPose)
			{
				const double DLatKm = (LLH.Y - LastCamPos.Y) * 111.0;
				const double DLonKm = (LLH.X - LastCamPos.X) * 111.0 * FMath::Cos(FMath::DegreesToRadians(LLH.Y));
				bJump = FMath::Sqrt(DLatKm * DLatKm + DLonKm * DLonKm) > 2.0;
			}
			LastCamPos = LLH;
			bHasLastPose = true;
		}
		if (bJump)
		{
			MovedTime = Now;
			// Le volume general coupe met les sons en veille : on le rouvre des maintenant pour que
			// leur demarrage (et ses valeurs aberrantes) se fasse pendant que le filtre est a 0
			SetWorldMute(World, false);
			if (bSoundOpened)
			{
				bSoundOpened = false;
				FSubmixEffectSanitizer::TargetGain = 0.f;
				UE_LOG(LogTemp, Log, TEXT("[Audio] Teleportation : son coupe le temps du chargement"));
			}
		}
	}
	if (Bad > 0)
	{
		LastBadTime = Now;
	}
	if (!bSoundOpened)
	{
		const bool bReady = MovedTime > 0.0 && Now - MovedTime > 4.0 && Now - LastBadTime > 2.0;
		if (bReady || (Now - BeginPlayTime > 120.0 && Now - MovedTime > 4.0))
		{
			bSoundOpened = true;
			SetWorldMute(GetWorld(), false);
			FSubmixEffectSanitizer::TargetGain = OpenGain;
			UE_LOG(LogTemp, Log, TEXT("[Audio] Son ouvert (%s)"), bReady ? TEXT("course chargee") : TEXT("delai de securite"));
		}
	}

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

	if (!bSoundOpened && Phase == ENaNHuntPhase::Idle)
	{
		// Son coupe (lancement, teleportation) : NaN et buzz n'atteignent pas la sortie, inutile de
		// couper des sons ; l'ouverture attend 2 s sans NaN
		return;
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
	// Ne jamais laisser l'editeur muet apres un PIE
	SetWorldMute(GetWorld(), false);
	if (USoundSubmix* Master = Cast<USoundSubmix>(GetDefault<UAudioSettings>()->MasterSubmix.TryLoad()))
	{
		Master->SetSubmixDryLevel(GetWorld(), 1.f);
	}
	FSubmixEffectSanitizer::TargetGain = 1.f;
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
