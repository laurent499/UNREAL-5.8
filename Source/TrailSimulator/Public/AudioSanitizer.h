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

// Installe le filtre sur le mix general de chaque monde de jeu (jeu et PIE). Tant que des NaN
// arrivent : relance chaque son actif une fois, puis les coupe un par un jusqu'a ce que les NaN
// cessent (le dernier coupe est le fautif, il reste coupe, les autres repartent), et en dernier
// recours vide les sound mix (volume de classe global). Chaque etape est journalisee [Audio].
UENUM()
enum class ENaNHuntPhase : uint8
{
	Idle,
	Restart,
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

	UPROPERTY()
	TObjectPtr<USubmixEffectSanitizerPreset> Preset;

	ENaNHuntPhase Phase = ENaNHuntPhase::Idle;
	// Sons actifs au moment de la detection, et position dans la liste
	TArray<TWeakObjectPtr<UAudioComponent>> Suspects;
	int32 Step = 0;
	double LastDoneLogTime = 0.0;
	FTimerHandle CheckTimer;
};
