// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SeaLevelSubsystem.generated.h"

/**
 * Donne au materiau des tuiles Cesium (M_CesiumGlobalWater) le niveau de la mer sous la camera, pour qu'il
 * remonte a ce niveau tout sommet situe plus bas (fonds marins => mers et oceans plats).
 * Ecrit dans MPC_Trail, deux fois par seconde :
 *  - SeaLevelOrigin : xyz = point au niveau de la mer (geoide EGM96) sous la camera, en coordonnees Unreal (cm) ; w = rayon de la Terre (cm)
 *  - SeaLevelUp     : xyz = verticale locale ; w = 1 actif / 0 coupe (CVar ts.Sea.Flatten)
 */
UCLASS()
class TRAILSIMULATOR_API USeaLevelSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	float TimeUntilUpdate = 0.f;

	UPROPERTY()
	TObjectPtr<class UMaterialParameterCollection> MPC;

	TWeakObjectPtr<class ACesiumGeoreference> Georeference;
};
