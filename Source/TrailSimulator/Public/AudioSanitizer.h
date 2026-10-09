#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundEffectSubmix.h"
#include "Subsystems/WorldSubsystem.h"
#include "AudioSanitizer.generated.h"

class UAudioComponent;

// Effet place sur le mix general : remplace toute valeur NaN/Inf par du silence et le signale.
// Une source qui sort des NaN (filtre MetaSound UDS parti en NaN au demarrage, au hasard des
// lancements) bloque l'encodeur audio OWL (« Input contains (near) NaN/+-Inf ») et ses images en
// attente retardent le flux SRT. Le niveau « dry » du submix reinjecte une part du signal brut,
// donc ce filtre seul ne protege pas OWL : le sous-systeme ci-dessous relance la source fautive.
USTRUCT(BlueprintType)
struct FSubmixEffectSanitizerSettings
{
	GENERATED_BODY()
};

class FSubmixEffectSanitizer : public FSoundEffectSubmix
{
public:
	virtual void Init(const FSoundEffectSubmixInitData& InData) override {}
	virtual void OnPresetChanged() override {}
	virtual void OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData) override;

	// Echantillons invalides vus depuis la derniere lecture (thread audio -> thread de jeu)
	static std::atomic<int64> BadSamples;
	// Echantillons au-dela de 0,9 (adoucis) depuis la derniere lecture
	static std::atomic<int64> ClippedSamples;
	// Gain voulu sur le mix general (0 = son coupe), atteint en rampe
	static std::atomic<float> TargetGain;

private:
	float CurrentGain = 0.f;
};

UCLASS()
class USubmixEffectSanitizerPreset : public USoundEffectSubmixPreset
{
	GENERATED_BODY()

public:
	EFFECT_PRESET_METHODS(SubmixEffectSanitizer)

	UPROPERTY()
	FSubmixEffectSanitizerSettings Settings;
};

// Installe le filtre sur le mix general de chaque monde de jeu (jeu et PIE). Des qu'il voit des
// NaN (verification toutes les 0,25 s), coupe les sons actifs un par un, le son directionnel UDS
// en premier (source constatee : UDS_Directional_WeatherSounds part en NaN au demarrage), jusqu'a ce
// que les NaN cessent. Le dernier coupe est le fautif : les autres repartent, lui est relance plus
// tard (10 s, puis 20, 40...) et recoupe si les NaN reviennent. En dernier recours, vide les sound mix.
UENUM()
enum class ENaNHuntPhase : uint8
{
	Idle,
	Stop,
	Done
};

UCLASS()
class UAudioSanitizerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	void CheckNaN();
	// Ambiance foret jouee par le jeu (voir AudioSanitizer.cpp)
	void UpdateForest();

	UPROPERTY()
	TObjectPtr<USubmixEffectSanitizerPreset> Preset;

	ENaNHuntPhase Phase = ENaNHuntPhase::Idle;
	// Sons actifs au moment de la detection, et position dans la liste
	TArray<TWeakObjectPtr<UAudioComponent>> Suspects;
	int32 Step = 0;
	double LastDoneLogTime = 0.0;
	FTimerHandle CheckTimer;

	// Son fautif coupe, et prochaine tentative de relance
	TWeakObjectPtr<UAudioComponent> Culprit;
	double CulpritRetryTime = 0.0;
	double CulpritRetryDelay = 10.0;
	bool bCulpritRetrying = false;
	int32 ForestTick = 0;
	UPROPERTY()
	TObjectPtr<UAudioComponent> ForestComp;
	int32 LastForestPhase = -1;
	bool bSoundOpened = false;
	double BeginPlayTime = 0.0;
	FVector InitialOrigin = FVector::ZeroVector;
	int64 ClippedSinceLog = 0;
	double LastClipLogTime = 0.0;
};
