// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "PathSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPathDatasGathered,
	int64,
	RaceID,
	FRacePath,
	RacePath);

/**
 * @brief Subsystem de gestion des tracés et des bornes kilométriques
 */
UCLASS()
class SUBSYSTEMS_API UPathSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/**
	 * @brief Triggers the request to the API
	 * @param PathEndpoint 
	 */
	void PerformHttpRequestForPath(int64 RaceID, const FString& PathEndpoint);

	TObjectPtr<AActor> GetPathById(int64 RaceID) const;
	
	TObjectPtr<AActor> SpawnPathActor(
		int64 RaceID, 
		UWorld* World,
		TSubclassOf<AActor> PathClass,
		const FTransform& SpawnTransform
	);
	
	TObjectPtr<AActor> SpawnKmActor(UWorld* World, TSubclassOf<AActor> PathClass, const FTransform& SpawnTransform);

	UPROPERTY(BlueprintAssignable, Category="Race|Events")
	FOnPathDatasGathered OnPathDatasGathered;
	
	FORCEINLINE FRacePath GetRacePath(){ return RacePath;}
	
private:
	TMap<FString, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ActivePathRequests;
	UPROPERTY()
	FRacePath RacePath;
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup);
	UPROPERTY()
	TMap<int64, FRacePath> RacePathsMap;
	UPROPERTY()
	TMap<int64, TObjectPtr<AActor>> MapPaths;
	
	
	
	
	
	
	
	
};
