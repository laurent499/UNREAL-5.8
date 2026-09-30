// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "WeatherSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWeatherGathered,
	FOpenWeatherResponse,
	WeatherResponse);

USTRUCT()
struct FWeatherResult
{
	GENERATED_BODY()

	FString RequestKey;          // pour debug / tracking
	bool bSuccess = false;
	int32 HttpCode = -1;
	FString Error;

	FOpenWeatherResponse Data;
};

/**
 * @brief Subsystem de gestion de la récupération/mise à jour des datas météo
 */
UCLASS()
class SUBSYSTEMS_API UWeatherSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	/**
	 * @brief Triggers the request to the API
	 * @param WeatherEndpoint 
	 */
	// void PerformHttpRequestForWeather(FString ObjectName, const FString& WeatherEndpoint);
	using FWeatherCallback = TUniqueFunction<void(FWeatherResult&&)>;

	void PerformHttpRequestForWeather(
		const FString& RequestKey,
		const FString& WeatherEndpoint,
		FWeatherCallback&& OnCompleted);
	
	FOnWeatherGathered OnWeatherGathered;
	
private:
	FOpenWeatherResponse WeatherResponse;
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup);
	
	TMap<FString, TSharedRef<IHttpRequest, ESPMode::ThreadSafe>> ActiveWeatherRequests;
};
