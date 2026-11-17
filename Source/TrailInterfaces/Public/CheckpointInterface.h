// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TrailSharedTypes.h"
#include "CheckpointInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCheckpointInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TRAILINTERFACES_API ICheckpointInterface
{
	GENERATED_BODY()
public:
	virtual void UpdateCheckpoint(FRaceCheckpoint CheckpointDatas, FRaceSetup RaceSetup) = 0;
	virtual void UpdateCheckpointWeather(FOpenWeatherResponse WeatherDatas) = 0;

	virtual void ToggleCheckpointWeather(bool bShow) = 0;
	virtual void StartAnimation(float RotationSpeed) = 0;
	virtual void StopAnimation() = 0;

	// Panneau du checkpoint (sans le pied) : ses enfants visibles forment le rectangle que les
	// cartes des coureurs evitent a l'ecran. nullptr = pas d'evitement.
	virtual USceneComponent* GetPanelComponent() const { return nullptr; }
	
	virtual void UpdateDayNight(bool bIsDay) = 0;
	
};
