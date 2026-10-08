// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldHeadlamps.generated.h"

class UPointLightComponent;
class UWorldAmbienceSubsystem;

/**
 * @brief Frontales des coureurs la nuit : une lumiere ponctuelle sans ombre au-dessus de chaque coureur
 * affiche, qui eclaire le terrain autour de lui (une flaque de lumiere, pas de boule visible).
 * Le rayon grandit avec la distance a la camera pour que la lueur reste lisible en plan aerien, et
 * l'intensite suit pour garder le meme eclat. Allumage progressif au crepuscule selon la nuit de
 * MPC_World et le reglage regie (bHeadlamps, HeadlampIntensity, HeadlampSize).
 */
UCLASS(NotPlaceable, Transient)
class SUBSYSTEMS_API AWorldHeadlamps : public AActor
{
	GENERATED_BODY()

public:
	AWorldHeadlamps();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	int32 GetLitCount() const { return LitCount; }

private:
	void RefreshRunners();
	UPointLightComponent* GetLamp(int32 Index);

	UPROPERTY()
	TArray<TObjectPtr<UPointLightComponent>> Lamps;

	TWeakObjectPtr<UWorldAmbienceSubsystem> Ambience;
	TArray<TWeakObjectPtr<AActor>> Runners;
	float RefreshTimer = 0.f;
	int32 LitCount = 0;
};
