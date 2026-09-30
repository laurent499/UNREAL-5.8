// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Poi.h"
#include "Poi_Nike.generated.h"

UCLASS()
class TRAILSIMULATOR_API APoi_Nike : public APoi
{
    GENERATED_BODY()

public:
    APoi_Nike();
    virtual void UpdatePoi(FRacePOI NewRacePoi, FRaceSetup NewRaceSetup) override;
    UPROPERTY()
    TObjectPtr<class UTrailTextWidgetComponent> NameWidgetComponent;
    UPROPERTY()
    TObjectPtr<class UTrailTextWidgetComponent> Infos1WidgetComponent;
    UPROPERTY()
    TObjectPtr<class UTrailTextWidgetComponent> Infos2WidgetComponent;
    
    UPROPERTY()
    TObjectPtr<class USplineComponent> NameSplineComponent;
    UPROPERTY()
    TObjectPtr<class USplineComponent> Infos1SplineComponent;
    UPROPERTY()
    TObjectPtr<class USplineComponent> Infos2SplineComponent;
    UPROPERTY(EditAnywhere)
    TObjectPtr<class USplineMeshComponent> Infos1BkgComponent;
    UPROPERTY(EditAnywhere)
    TObjectPtr<class USplineMeshComponent> Infos2BkgComponent;

protected:
    
    UPROPERTY()
    TObjectPtr<class UFont> NameFont;
    UPROPERTY()
    TObjectPtr<class UFont> InfosFont;
    
    UPROPERTY()
    FVector StartLocation;
    UPROPERTY()
    FVector LastLocation;
};
