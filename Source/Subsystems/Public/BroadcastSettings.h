// BroadcastSettings.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BroadcastSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Broadcast"))
class SUBSYSTEMS_API UBroadcastSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category="Capture")
	TSoftObjectPtr<class UTextureRenderTarget2D> SharedRT;
};