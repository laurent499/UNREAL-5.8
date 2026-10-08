// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldHeadlamps.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UWorldAmbienceSubsystem;

/**
 * @brief Frontales des coureurs la nuit : un point lumineux emissif (sans ombre ni lumiere dynamique)
 * a 3 m au-dessus de chaque coureur affiche. Sa taille suit la distance a la camera pour rester
 * visible en plan aerien, le bloom fait le halo. Allume au crepuscule selon la nuit de MPC_World
 * et le reglage regie (bHeadlamps, HeadlampIntensity).
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
	UStaticMeshComponent* GetLamp(int32 Index);

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Lamps;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> LampMaterial;
	UPROPERTY()
	TObjectPtr<UStaticMesh> LampMesh;

	TWeakObjectPtr<UWorldAmbienceSubsystem> Ambience;
	TArray<TWeakObjectPtr<AActor>> Runners;
	float RefreshTimer = 0.f;
	float AppliedGlow = -1.f;
	int32 LitCount = 0;
};
