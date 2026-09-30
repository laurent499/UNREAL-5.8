#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TrailSharedTypes.h"
#include "RunnerStackingSubsystem.generated.h"

UCLASS()
class SUBSYSTEMS_API URunnerStackingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Runner lifecycle
	UFUNCTION()
	void RegisterRunner(AActor* Runner);
	UFUNCTION()
	void UnregisterRunner(AActor* Runner);

	// À appeler depuis ton système d’update (même si le runner est stacké)
	UFUNCTION()
	void UpdateRunnerTrackState(AActor* Runner, const FTransform& TrackTransform, float TrackDistanceMeters);
	UFUNCTION()
	void MakeDirty();

	// Config
	UFUNCTION()
	void SetConfig(const FRunnerStackingConfig& NewConfig);
	FRunnerStackingConfig GetConfig() const { return Config; }

	// Debug
	UFUNCTION()
	int32 GetRegisteredCount() const { return Entries.Num(); }

private:
	
	UPROPERTY()
	FVector LastCamLoc = FVector::ZeroVector;
	UPROPERTY()
	bool bHasLastCamLoc = false;
	UPROPERTY()
	float CameraMoveThresholdCm = 10.f; // 1m (à ajuster)
	
	UPROPERTY()
	FRunnerStackingConfig Config;

	UPROPERTY()
	TArray<FStackEntry> Entries;

	FTimerHandle UpdateTimerHandle;
	bool bDirty = false;

	UFUNCTION()
	void HandleActorEndPlay(AActor* Actor, EEndPlayReason::Type Reason);

	void StartTimer();
	void StopTimer();

	void TickSubsystem();

	// Core
	void CleanupInvalid();
	int32 FindEntryIndex(AActor* Runner) const;

	float ComputeDynamicRadiusCm(const FStackEntry& BaseEntry) const;

	// Stacking apply
	void ApplyNewStackingState(
		const TArray<TWeakObjectPtr<AActor>>& NewBase,
		const TArray<int32>& NewOrder);

	void AttachRunnerToBase(AActor* Runner, const FStackEntry& RunnerEntry, AActor* Base, const FStackEntry& BaseEntry, int32 Order);
	void DetachRunnerToTrack(AActor* Runner, const FStackEntry& RunnerEntry);
};
