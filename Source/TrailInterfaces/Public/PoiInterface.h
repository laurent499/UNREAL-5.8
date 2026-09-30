// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TrailSharedTypes.h"
#include "PoiInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPoiInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TRAILINTERFACES_API IPoiInterface
{
	GENERATED_BODY()

public:
	virtual void UpdatePoi(FRacePOI PoiData, FRaceSetup RaceSetup) = 0;
	virtual void UpdatePoiWeather(FOpenWeatherResponse WeatherDatas) = 0;
	
	virtual void TogglePoiWeather(bool bShow) = 0;
	
	virtual void StartAnimation(float RotationSpeed) = 0;
	virtual void StopAnimation() = 0;
	
	virtual void UpdateDayNight(bool bIsDay) = 0;

};
