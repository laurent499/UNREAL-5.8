// All Rights Reserved


#include "WeatherSubsystem.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "RaceSubsystem.h"
#include "WorldUtils.h"

void UWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(URaceSubsystem::StaticClass());
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!RaceSubsystem) return;
	RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &UWeatherSubsystem::SetRaceSetup);
}

void UWeatherSubsystem::Deinitialize()
{
	Super::Deinitialize();
	// for (auto& Pair : WeatherRequests)
	// {
	// 	TSharedPtr<IHttpRequest> Req = Pair.Value;
	// 	Req->OnProcessRequestComplete().Unbind();
	// 	Req->CancelRequest();	
	// }
	// WeatherRequests.Empty();
	
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!RaceSubsystem) return;
	RaceSubsystem->OnRaceSetupDatasGathered.RemoveDynamic(this, &UWeatherSubsystem::SetRaceSetup);
	
}

/**
 * @brief Convert OpenWeather
 * @param JsonString 
 * @param OutRoot 
 * @return 
 */
bool ConvertWeatherJson(const FString& JsonString, FOpenWeatherResponse& OutRoot)
{
	TSharedPtr<FJsonObject> Root;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), Root) || !Root.IsValid())
		return false;
	if (!FJsonObjectConverter::JsonObjectToUStruct(Root.ToSharedRef(), &OutRoot, 0, 0))
		return false;

	// "current.rain.1h" / "current.snow.1h" : absents quand il ne tombe rien
	const TSharedPtr<FJsonObject>* Current = nullptr;
	if (Root->TryGetObjectField(TEXT("current"), Current))
	{
		const TSharedPtr<FJsonObject>* Precip = nullptr;
		double Value = 0.0;
		if ((*Current)->TryGetObjectField(TEXT("rain"), Precip) && (*Precip)->TryGetNumberField(TEXT("1h"), Value))
			OutRoot.current.rain_1h = Value;
		if ((*Current)->TryGetObjectField(TEXT("snow"), Precip) && (*Precip)->TryGetNumberField(TEXT("1h"), Value))
			OutRoot.current.snow_1h = Value;
	}
	return true;
}

void UWeatherSubsystem::PerformHttpRequestForWeather(
	const FString& RequestKey,
	const FString& WeatherEndpoint,
	FWeatherCallback&& OnCompleted)
{
	// Wrap dans un objet copiable (le delegate UE va copier la lambda)
	TSharedRef<FWeatherCallback, ESPMode::ThreadSafe> Callback =
		MakeShared<FWeatherCallback, ESPMode::ThreadSafe>(MoveTemp(OnCompleted));

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(WeatherEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	ActiveWeatherRequests.Add(RequestKey, Req);

	TWeakObjectPtr<UWeatherSubsystem> WeakThis(this);

	Req->OnProcessRequestComplete().BindLambda(
		[WeakThis, RequestKey, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			FWeatherResult Result;
			Result.RequestKey = RequestKey;

			if (!WeakThis.IsValid())
			{
				Result.Error = TEXT("Subsystem destroyed.");
				(*Callback)(MoveTemp(Result));
				return;
			}

			// cleanup request tracking (souvent callback déjà sur GameThread)
			WeakThis->ActiveWeatherRequests.Remove(RequestKey);

			if (!bWasSuccessful || !Response.IsValid())
			{
				Result.Error = TEXT("HTTP failed or invalid response.");
				(*Callback)(MoveTemp(Result));
				return;
			}

			Result.HttpCode = Response->GetResponseCode();
			const FString JsonString = Response->GetContentAsString();

			UE::Tasks::Launch(UE_SOURCE_LOCATION,
				[WeakThis, Callback, Result = MoveTemp(Result), JsonString]() mutable
				{
					FOpenWeatherResponse Parsed;
					const bool bParsedOk = ConvertWeatherJson(JsonString, Parsed);

					Result.Data = MoveTemp(Parsed);
					Result.bSuccess = bParsedOk && (Result.HttpCode >= 200 && Result.HttpCode < 300);

					if (!bParsedOk)
						Result.Error = TEXT("JSON parse failed.");
					else if (!Result.bSuccess)
						Result.Error = FString::Printf(TEXT("HTTP error code: %d"), Result.HttpCode);

					AsyncTask(ENamedThreads::GameThread,
						[Callback, Result = MoveTemp(Result)]() mutable
						{
							(*Callback)(MoveTemp(Result));
						});
				},
				UE::Tasks::ETaskPriority::BackgroundNormal);
		});

	Req->ProcessRequest();
}

void UWeatherSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup)
{
	CurrentRaceSetup = NewRaceSetup;
}
