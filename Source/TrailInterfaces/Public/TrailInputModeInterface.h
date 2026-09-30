// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TrailInputModeInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UTrailInputModeInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * 
 */
class TRAILINTERFACES_API ITrailInputModeInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Input")
    void RequestAnimationInput(AActor* Source);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Input")
    void ReleaseAnimationInput(AActor* Source);
};
