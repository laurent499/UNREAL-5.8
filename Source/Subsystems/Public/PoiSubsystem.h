// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "PoiSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPoisDatasGathered,
	int64,
	RaceID,
	FPOIs,
	RacePois);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnPoiDatasGathered,
	int64,
	RaceID,
	int64,
	PoiID,
	FRacePOI,
	RacePoi);

/**
 * @brief Manage POIs
 */
UCLASS()
class SUBSYSTEMS_API UPoiSubsystem : public UGameInstanceSubsystem 
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION()
	void TogglePois(int64 RaceID, bool bShow);
	UFUNCTION()
	void TogglePoi(int64 RaceID, int64 PoiID, bool bShow);
	UFUNCTION()
	void ToggleWeather(int64 RaceID, int64 PoiID, bool bShow);
	UFUNCTION()
	void Teleport(int64 PoiID, int64 RaceID);
	UFUNCTION()
	void Animation(int64 PoiID, int64 IdRace);
	UFUNCTION()
	void StopAnimation(int64 PoiID, int64 RaceID);

	
	/**
	 * @brief Triggers the request to the API for all POIs
	 * @param POIsEndpoint 
	 */
	void PerformHttpRequestForPOIs(int64 RaceID, const FString& POIsEndpoint);
	
	UPROPERTY(BlueprintAssignable, Category="Race|Events")
	FOnPoisDatasGathered OnPoisDatasGathered;
	
	UPROPERTY(BlueprintAssignable, Category="Race|Events")
	FOnPoiDatasGathered OnPoiDatasGathered;
	
	UFUNCTION()
	AActor* SpawnPoi(
		int64 RaceID,
		int64 PoiID,
		UWorld* World,
		TSubclassOf<AActor> PoiClass,
		const FTransform& SpawnTransform);
	
	UFUNCTION()
	bool DoesPoiBelongsToRace(int64 PoiID, int64 RaceID);
	
private:
	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup);
	TObjectPtr<AActor> GetPoiActor(int64 PoiID, int64 RaceID);
	
	UPROPERTY()
	FRacePOI POI;
	UPROPERTY()
	FPOIs POIs;
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UPROPERTY()
	TMap<int64, FPOIs> RacePoisMap;
	
	TMap<int64, TMap<int64, TObjectPtr<AActor>>> PoisActorsMap;
	
	TMap<int64, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ActivePoiRequests;
};
