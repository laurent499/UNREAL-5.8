// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CesiumBudgetSubsystem.generated.h"

/**
 * Limite le temps que Cesium passe chaque image sur le thread de jeu a construire et a liberer des tuiles.
 * Cesium fixe ces limites a 5 ms chacune a la creation du tileset ; on les reapplique periodiquement
 * (le tileset peut etre recree) avec les valeurs des variables console ts.Cesium.*.
 */
UCLASS()
class TRAILSIMULATOR_API UCesiumBudgetSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	float TimeUntilApply = 0.f;
};
