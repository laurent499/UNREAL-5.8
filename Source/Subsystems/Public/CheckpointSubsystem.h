// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TrailSharedTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "CheckpointSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCheckpointsDatasGathered,
	int64,
	RaceID,
	FCheckpoints,
	AllCheckpoints);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnCheckpointDatasGathered,
	FRaceCheckpoint,
	OneCheckpoint,
	int64,
	RaceID,
	int64,
	CheckpointID);

UCLASS()
class SUBSYSTEMS_API UCheckpointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/** GETTERS */
	FCheckpoints* GetCheckpointsByRaceId(int64 RaceID);
	FRaceCheckpoint GetCheckpointByChkIdRaceId(int64 RaceID, int64 ChkID);
	
	/**
	 * @brief Triggers the request to the API for all checkpoints
	 * @param RaceID
	 * @param CheckpointsEndpoint 
	 */
	UFUNCTION()
	void PerformHttpRequestForCheckpoints(int64 RaceID, const FString& CheckpointsEndpoint);
	UPROPERTY(BlueprintAssignable, Category="Race|Events")
	FOnCheckpointsDatasGathered OnCheckpointsDatasGathered;
	
	UFUNCTION()
	AActor* SpawnCheckpointActor(
		int64 RaceID,
		int64 CheckpointID,
		UWorld* World,
		TSubclassOf<AActor> PathClass,
		const FTransform& SpawnTransform
	);
	
	UFUNCTION()
	bool DoesChkBelongsToRace(int64 ChkID, int64 RaceID);
	
	
	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewCurrentRaceSetup);

	TObjectPtr<AActor> GetCheckpointActor(int64 RaceID, int64 CheckpointID);
	UFUNCTION()
	void ToggleCheckpoints(int64 RaceID, bool bShow);
	UFUNCTION()
	void ToggleCheckpoint(int64 RaceID, int64 CheckpointID, bool bShow);
	UFUNCTION()
	void ToggleWeather(int64 RaceID, int64 CheckpointID, bool bShow);
	UFUNCTION()
	void Teleport(int64 CheckpointID, int64 RaceID);
	UFUNCTION()
	void Animation(int64 CheckpointID, int64 IdRace);
	UFUNCTION()
	void StopAnimation(int64 CheckpointID, int64 RaceID);

private:
	UPROPERTY()
	FCheckpoints Checkpoints;
	UPROPERTY()
	FRaceCheckpoint Checkpoint;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	
	UPROPERTY()
	TMap<int64, FCheckpoints> CheckpointsMap;
	TMap<int64, TMap<int64, TObjectPtr<AActor>>> CheckpointActorMap;
	TMap<int64, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ActiveCheckpointRequests;
};
