// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Poi.h"
#include "Poi_UTMB.generated.h"

UCLASS()
class TRAILSIMULATOR_API APoi_UTMB : public APoi
{
	GENERATED_BODY()

public:
	virtual void UpdatePoi(FRacePOI PoiData, FRaceSetup RaceSetup) override;
	virtual void UpdateDayNight(bool bIsDay) override;
	
	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	UMaterialInterface* MainPictoMat;
	UPROPERTY()
	UMaterialInstanceDynamic* MainPictoMID;
	UPROPERTY()
	UMaterialInterface* DistPictoMat;
	UPROPERTY()
	UMaterialInterface* AltPictoMat;
	UPROPERTY()
	UMaterialInstanceDynamic* AltPictoMID;
	UPROPERTY()
	UMaterialInterface* WeatherPictoMat;
	UPROPERTY()
	UMaterialInstanceDynamic* WeatherPictoMID;
};
