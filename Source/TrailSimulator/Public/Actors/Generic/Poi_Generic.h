// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Poi.h"
#include "Poi_Generic.generated.h"

UCLASS()
class TRAILSIMULATOR_API APoi_Generic : public APoi
{
	GENERATED_BODY()

public:
	APoi_Generic();
	virtual void UpdatePoi(FRacePOI PoiData, FRaceSetup RaceSetup) override;
		
	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;
	virtual void UpdateDayNight(bool bIsDay) override;

private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UStaticMeshComponent> NameLeftComponent;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UStaticMeshComponent> NameRightComponent;
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
