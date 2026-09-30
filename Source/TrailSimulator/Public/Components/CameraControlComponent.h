// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraControlComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UCameraControlComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UCameraControlComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class USpringArmComponent> SpringArmComponent;
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UCineCameraComponent> CameraComponent;
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UOWLCaptureComponent> OwlCapture;
    
    UFUNCTION()
    void Pitch(float NewAngle);
    UFUNCTION()
    void Pan(float NewDirection);
    UFUNCTION()
    void Zoom(float NewDirection);
    UFUNCTION()
    void Reset();
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override;


};
