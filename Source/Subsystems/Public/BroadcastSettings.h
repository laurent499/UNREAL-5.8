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

	// Affiche SharedRT dans le viewport du jeu et coupe le rendu du monde principal :
	// la scène n'est plus rendue deux fois (viewport + capture OWL).
	// Surcharge à chaud : trail.Broadcast.MirrorViewport 0/1
	UPROPERTY(EditAnywhere, Config, Category="Capture")
	bool bMirrorCaptureToViewport = true;
};