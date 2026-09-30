// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Engine/TimerHandle.h"
#include "RunnerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRunnersDatasGathered,
	int64,
	RaceID,
	FRunners,
	RunnersDatas);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRunnersUpdateDatas,
	int64,
	RaceID,
	FRunners,
	RunnersDatas);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRunnerDatasGathered,
	int64,
	RaceID,
	FRunnerStruct,
	Runner);

/**
 * @brief Subsystem de gestion/récupération/update des datas des Runners
 */
UCLASS()
class SUBSYSTEMS_API URunnerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION()
	FRunners GetRunnersByRaceId(int64 RaceID);
	TObjectPtr<AActor> GetRunnerActor(int64 RaceID, int64 RunnerID);
	
	TObjectPtr<AActor> GetRunnerActorByTeam(int64 RaceID, int64 TeamID);

	UFUNCTION()
	void ToggleRunnerByTeam(int64 RaceID, int64 TeamID, bool bShow);

	UFUNCTION()
	void AnimationByTeam(int64 TeamID, int64 RaceID);

	UFUNCTION()
	void StopAnimationByTeam(int64 TeamID, int64 RaceID);

	UFUNCTION()
	void RemoveTeamsNotInSnapshot(int64 RaceID, const FRunners& RunnersDatas);
	
		
	/**
	 * @brief Triggers the request to the API for all Runners
	 * @param RaceID
	 * @param RunnersEndPoint 
	 */
	void PerformHttpRequestForRunners(int64 RaceID, const FString& RunnersEndPoint, bool bStartup);
	
	/**
	 * @brief Triggers the request to the API for one Runner
	 * @param RaceID
	 * @param RunnerEndPoint
	 * @param RunnerId 
	 */
	
	void PerformHttpRequestForRunner(int64 RaceID, const FString& RunnerEndPoint, int64 RunnerId);
	/**
	 * @brief Spawn one runner
	 * @param RaceID
	 * @param World 
	 * @param RunnerClass 
	 * @param SpawnTransform 
	 * @return 
	 */
	UFUNCTION()
	AActor* SpawnRunnerActor(
		int64 RaceID,
		FRunnerStruct Runner,
		UWorld* World,
		TSubclassOf<AActor> RunnerClass,
		const FTransform& SpawnTransform);
	
	UFUNCTION()
	bool DoesRunnerBelongsToRace(int64 RunnerID, int64 RaceID);

	UFUNCTION()
	void ToggleRunners(int64 RaceID, bool bShow);
	UFUNCTION()
	void ToggleRunner(int64 RaceID, int64 RunnerID, bool bShow);

	UPROPERTY(BlueprintAssignable, Category="Race|Events")
	FOnRunnersDatasGathered OnRunnersDatasGathered;
	UPROPERTY(BlueprintAssignable, Category="Race|Events")
	FOnRunnersUpdateDatas OnRunnersUpdateDatas;
	
	UPROPERTY(BlueprintAssignable, Category="Race|Events")
	FOnRunnerDatasGathered OnRunnerDatasGathered;
	
	UFUNCTION()
	void Animation(int64 RunnerID, int64 RaceID);
	UFUNCTION()
	void StopAnimation(int64 RunnerID, int64 RaceID);
	UFUNCTION()
	void UpdateDayNightGlow(int64 RaceID, bool bIsDay, float NewGlow);

private:
	/**
	 * @brief Fetch des runners avec suivi du numero de tentative
	 * @param Attempt Numero de la tentative courante, 1 pour la premiere
	 */
	void RequestRunnersWithRetry(int64 RaceID, const FString& RunnersEndPoint, bool bStartup, int32 Attempt);

	/**
	 * @brief Planifie une nouvelle tentative de fetch (backoff 2s, 4s, 8s...)
	 * @return true si une tentative a ete planifiee, false si la serie est epuisee
	 */
	bool ScheduleRunnersRetry(
		int64 RaceID,
		const FString& RunnersEndPoint,
		bool bStartup,
		int32 Attempt,
		const FString& Reason);

	/** @brief Annule la serie de retries en cours pour une race */
	void CancelRunnersRetry(int64 RaceID);

	/** @brief Annule toutes les series de retries en cours */
	void CancelAllRunnersRetries();

	/** Nombre maximum de tentatives pour un fetch runners */
	static constexpr int32 MaxRunnersFetchAttempts = 5;
	/** Delai de la premiere relance, double a chaque tentative */
	static constexpr float RunnersRetryBaseDelay = 2.f;
	/** Plafond du backoff */
	static constexpr float RunnersRetryMaxDelay = 8.f;

	/** Timer de retry en cours, par RaceID */
	TMap<int64, FTimerHandle> RunnersRetryTimers;

	TMap<int64, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ActiveRunnerRequests;
	UPROPERTY()
	FRunners Runners;
	
	TMap<int64, FRunners> RunnersMap;
	
	// clé interne = TeamID (canalId), pas runnerId
	TMap<int64, TMap<int64, TObjectPtr<AActor>>> RunnersActorsMap;
	
	UPROPERTY()
	FRunnerStruct RunnerDatas;
	UPROPERTY()
	int64 CurrentRunnerId;
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup);
};