// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "TrailsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnRacesDatasGathered,
	FRaceEntries,
	RacesDatas);

/**
 * @brief Subsystem de gestion des données de toutes les courses
 */
UCLASS()
class SUBSYSTEMS_API UTrailsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/**
	 * @brief Triggers the request to the API
	 * @param RacesEndpoint 
	 */
	void PerformHttpRequestForRaces(const FString& RacesEndpoint);

	UPROPERTY()
	FOnRacesDatasGathered OnRacesDatasGathered;
	
private:
	UPROPERTY()
	FRaceEntries RacesArray;
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup);
	
	UPROPERTY()
	TObjectPtr<class ULoadingStatusSubsystem> LoadingSubsystem;
};
