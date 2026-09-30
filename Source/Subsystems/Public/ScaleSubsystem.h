// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TrailSharedTypes.h"
#include "ScaleInterface.h"
#include "TrailSharedTypes.h"
#include "ScaleSubsystem.generated.h"

UCLASS()
class SUBSYSTEMS_API UScaleSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// --- UWorldRickableSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override { return false; }

	// --- API
	UFUNCTION()
	void RegisterScalableActor(AActor* Actor);
	// UFUNCTION()
	// void RegisterScalableActorWithConfig(AActor* Actor, const FDistanceScaleConfig& OverrideConfig);
	UFUNCTION()
	void UnregisterScalableActor(AActor* Actor);
	UFUNCTION()
	void SetGlobalConfig(const FDistanceScaleConfig& NewConfig);
	UFUNCTION()
	void SetNearFar(float NewNear, float NewFar);
	UFUNCTION()
	float GetNear() const;
	UFUNCTION()
	float GetFar() const;
	UFUNCTION()
	void SetActorScaleMinMax(AActor* Actor, const FMinMax& NewMinMax);
	void SetGlobalMinMax(float MinValue, float MaxValue);
	UFUNCTION()
	FDistanceScaleConfig GetGlobalConfig() const { return GlobalConfig; }
	UFUNCTION()
	int32 GetRegisteredCount() const { return Entries.Num(); }	
	
private:
	
	UPROPERTY()
	FDistanceScaleConfig GlobalConfig;

	UPROPERTY()
	TArray<FEntry> Entries;

	UFUNCTION()
	void HandleActorEndPlay(AActor* Actor, EEndPlayReason::Type Reason);

	bool GetCameraLocation(FVector& OutCamLoc) const;

	FVector ComputeTargetScale(const FEntry& Entry, const FVector& CamLoc, const FVector& ActorLoc) const;
};
