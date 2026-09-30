// BroadcastCaptureSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "BroadcastCaptureSubsystem.generated.h"

class UTextureRenderTarget2D;
class APlayerController;

UCLASS()
class SUBSYSTEMS_API UBroadcastCaptureSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(EditAnywhere, Category="Broadcast")
	TSoftObjectPtr<UTextureRenderTarget2D> SharedRTAsset;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> SharedRT;

	// Si tu veux limiter le coût : check à 10Hz au lieu de chaque frame
	UPROPERTY(EditAnywhere, Category="Broadcast")
	float MonitorHz = 30.f;

	// Wrapper conseillé pour tes switches explicites (C++/BP)
	UFUNCTION(BlueprintCallable, Category="Broadcast")
	void RequestViewTarget(APlayerController* PC, AActor* NewTarget, float BlendTime = 0.f);

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBroadcastCaptureSubsystem, STATGROUP_Tickables); }

private:
	TWeakObjectPtr<APlayerController> CachedPC;
	TWeakObjectPtr<AActor> ActiveSource;
	TWeakObjectPtr<AActor> DefaultSource;

	float Accum = 0.f;

	void ActivateSource(AActor* Source);
	AActor* ResolveCaptureSourceFromViewTarget(AActor* ViewTarget) const;
};