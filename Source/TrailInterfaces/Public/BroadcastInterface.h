// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BroadcastInterface.generated.h"



UINTERFACE(Blueprintable)
class UBroadcastInterface : public UInterface
{
	GENERATED_BODY()
};


class TRAILINTERFACES_API IBroadcastInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Broadcast")
	void SetBroadcastCaptureEnabled(bool bEnabled, class UTextureRenderTarget2D* SharedRT);
};
