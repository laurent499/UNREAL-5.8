// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TrailSharedTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "RaceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRaceDatasGathered,
	int64,
	RaceID,
	FRaceSetup,
	RaceDatas);

/**
 * @brief Subsystem de récupération du setup d'une Race
 * @note FRaceSetup
 */
UCLASS()
class SUBSYSTEMS_API URaceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Deinitialize() override;
	
	UPROPERTY()
	FString MainURL = TEXT("https://simulacre.ltvprod.cc/");
	
	UFUNCTION()
	FString GetMainURL() const;
	UFUNCTION()
	bool IsRaceSetupValid(const FRaceSetup& RaceSetup) const;
	UFUNCTION()
	void SetCurrentRaceId(int64 RaceId);
	UFUNCTION()
	int64 GetCurrentRaceId() const;
	UFUNCTION()
	void SetCurrentRaceSetup(FRaceSetup NewRaceSetup);
	UFUNCTION()
	FRaceSetup GetRaceSetupByName(const FString& RaceName) const;
	UFUNCTION()
	const FRaceSetup GetRaceSetupById(int64 RaceID) const;

	/**
	 * @brief Triggers the request to the API
	 * @param RaceID
	 * @param RacesEndpoint 
	 */
	UFUNCTION()
	void PerformHttpRequestForRaceSetup(const int64 RaceID, const FString& RacesEndpoint, float Total, float Cpt);

	UPROPERTY()
	FOnRaceDatasGathered OnRaceSetupDatasGathered;
	
private:
	TMap<int64, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> RaceRequest;
	
	FRaceSetup RaceSetup;
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	UPROPERTY()
	int64 CurrentRaceId = -1;
	UPROPERTY()
	TMap<int64, FRaceSetup> RaceSetupDatasMap;
};