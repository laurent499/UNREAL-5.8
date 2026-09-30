// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Checkpoint.h"
#include "Checkpoint_Generic.generated.h"

UCLASS()
class TRAILSIMULATOR_API ACheckpoint_Generic : public ACheckpoint
{
	GENERATED_BODY()

public:
	ACheckpoint_Generic();
	virtual void Tick( float DeltaTime ) override;
	
	virtual void UpdateCheckpoint(FRaceCheckpoint NewCheckpointDatas, FRaceSetup NewRaceSetup) override;
	virtual void UpdateDayNight(bool bIsDay) override;
	
	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;
protected:
	
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
