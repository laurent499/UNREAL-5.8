// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Checkpoint.h"
#include "Checkpoint_Nike.generated.h"

UCLASS()
class TRAILSIMULATOR_API ACheckpoint_Nike : public ACheckpoint
{
    GENERATED_BODY()

public:
    ACheckpoint_Nike();
    
    virtual void UpdateCheckpoint(FRaceCheckpoint NewCheckpointDatas, FRaceSetup NewRaceSetup) override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UTrailTextWidgetComponent> NameWidgetComponent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UTrailTextWidgetComponent> Infos1WidgetComponent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UTrailTextWidgetComponent> Infos2WidgetComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class USplineComponent> NameSplineComponent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class USplineComponent> Infos1SplineComponent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class USplineComponent> Infos2SplineComponent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class USplineMeshComponent> Infos1BkgComponent;
    UPROPERTY(EditAnywhere)
    TObjectPtr<class USplineMeshComponent> Infos2BkgComponent;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UFont> NameFont;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UFont> InfosFont;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartLocation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastLocation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LeftY;
};
