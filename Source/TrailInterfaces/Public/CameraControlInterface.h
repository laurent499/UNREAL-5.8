// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CameraControlInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCameraControlInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * 
 */
class TRAILINTERFACES_API ICameraControlInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_Height(float Value);
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_SaveHeight();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_Pitch(float Value);
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_SavePitch();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_Fwd(float Value);
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_SaveFwd();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_Reset();
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_GetLength();
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_GetPitch();
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Camera Control")
    void CameraControl_GetHeight();
};
