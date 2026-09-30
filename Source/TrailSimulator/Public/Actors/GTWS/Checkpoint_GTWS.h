// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Checkpoint.h"
#include "Checkpoint_GTWS.generated.h"

UCLASS()
class TRAILSIMULATOR_API ACheckpoint_GTWS : public ACheckpoint
{
	GENERATED_BODY()

public:
	
	
	virtual void UpdateCheckpoint(FRaceCheckpoint NewCheckpointDatas, FRaceSetup NewRaceSetup) override;

	virtual void UpdateDayNight(bool bIsDay) override;

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;

private:
	UPROPERTY()
	UMaterialInterface* MainPictoMat;
	UPROPERTY()
	UMaterialInstanceDynamic* MainPictoMID;
	UPROPERTY()
	UMaterialInterface* DistPictoMat;
	UPROPERTY()
	UMaterialInstanceDynamic* DistPictoMID;
	UPROPERTY()
	UMaterialInterface* AltPictoMat;
	UPROPERTY()
	UMaterialInstanceDynamic* AltPictoMID;
	UPROPERTY()
	UMaterialInterface* WeatherPictoMat;
	UPROPERTY()
	UMaterialInstanceDynamic* WeatherPictoMID;
};
