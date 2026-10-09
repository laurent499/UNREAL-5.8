#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundEffectSubmix.h"
#include "Subsystems/WorldSubsystem.h"
#include "AudioSanitizer.generated.h"

class UAudioComponent;
struct FOWLNaNLogWatcher;

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

// Installe le filtre sur le mix general de chaque monde de jeu (jeu et PIE), et tant que des NaN
// arrivent, relance les composants audio actifs un par un (une seconde chacun) pour trouver et
// reinitialiser la source fautive.
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
	// OWL peut aussi produire des NaN de lui-meme (au hasard des lancements, mix UE sain) : son
	// encodeur reste alors bloque toute la session. On relance la sortie OWL quand son log le signale.
	void RestartOWLOutput();

	UPROPERTY()
	TObjectPtr<USubmixEffectSanitizerPreset> Preset;

	// Composant relance a la verification precedente, et position dans la liste des composants
	TWeakObjectPtr<UAudioComponent> LastRestarted;
	int32 RestartIndex = 0;
	FTimerHandle CheckTimer;

	// Possede par le sous-systeme (cree au BeginPlay, detruit dans Deinitialize)
	FOWLNaNLogWatcher* OWLLogWatcher = nullptr;
	int32 OWLRestarts = 0;
	double LastOWLRestartTime = 0.0;
};
