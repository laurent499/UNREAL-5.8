#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "UObject/ObjectKey.h"
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
	float ComputeOverlapRadiusCm(const FStackEntry& BaseEntry) const;

	// Stacking apply
	void ApplyNewStackingState(
		const TArray<TWeakObjectPtr<AActor>>& NewBase,
		const TArray<int32>& NewOrder);

	void AttachRunnerToBase(AActor* Runner, const FStackEntry& RunnerEntry, AActor* Base, const FStackEntry& BaseEntry, int32 Order);
	void DetachRunnerToTrack(AActor* Runner, const FStackEntry& RunnerEntry);

	// ------------------------------------------------------------
	// Evitement des panneaux checkpoint : chaque image, la carte d'un runner (ou de sa pile) qui
	// recouvre un panneau checkpoint a l'ecran monte au-dessus si la place est libre, sinon
	// descend dessous, avec un decalage lisse.
	// ------------------------------------------------------------
	struct FCardAvoidState
	{
		FBox LocalBox = FBox(ForceInit); // carte (+ enfants de la pile) dans le repere du composant carte
		float Target = 0.f;              // decalage vise (unites locales de l'acteur)
		int8 Side = 0;                   // +1 au-dessus, -1 en dessous, 0 a sa place
	};
	struct FCheckpointPanel
	{
		TWeakObjectPtr<USceneComponent> Panel;
		FBox LocalBox = FBox(ForceInit);
	};
	TMap<TObjectKey<AActor>, FCardAvoidState> CardStates;
	TArray<FCheckpointPanel> CheckpointPanels;
	double NextCheckpointScanTime = 0.0;
	int32 CheckpointBoxCursor = 0;
	int32 RunnerBoxCursor = 0;
	bool bAnyCardLifted = false;
	FDelegateHandle PostActorTickHandle;

	void HandlePostActorTick(UWorld* InWorld, ELevelTick TickType, float DeltaSeconds);
	void TickCardAvoidance(float DeltaSeconds);
	void InvalidateCardBox(AActor* Runner);
};
