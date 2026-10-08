// All Rights Reserved


#include "TrailsSubsystem.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "LoadingStatusSubsystem.h"
#include "Interfaces/IHttpResponse.h"
#include "RaceSubsystem.h"
#include "TrailHttpDebug.h"

void UTrailsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (GetGameInstance())
	{
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
		if (RaceSubsystem){
			RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &UTrailsSubsystem::SetRaceSetup);
		}
	}
}

void UTrailsSubsystem::Deinitialize()
{
	Super::Deinitialize();
	if (RaceSubsystem){
		RaceSubsystem->OnRaceSetupDatasGathered.RemoveDynamic(this, &UTrailsSubsystem::SetRaceSetup);
	}
}

/**
 * @brief Convert Json datas into Struct
 * @param JsonString 
 * @param OutRoot 
 * @return 
 */
bool ConvertRacesJson(const FString& JsonString, FRaceEntries& OutRoot)
{	
	TArray<FRaceEntry> RaceEntries;
	const bool bOk = FJsonObjectConverter::JsonArrayStringToUStruct(
		JsonString,
		&RaceEntries,
		0, 0
	);
	if (bOk)
	{
		OutRoot.RacesEntries = MoveTemp(RaceEntries);
	}
	return bOk;
}

void UTrailsSubsystem::PerformHttpRequestForRaces(const FString& RacesEndpoint)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(RacesEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	TWeakObjectPtr<UTrailsSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
	[WeakThis, this](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (!WeakThis.IsValid()) return;
		
		// Debug 
		// FTrailHttpDebugOptions Opt;
		// Opt.bLogHeaders = false;
		// Opt.bLogBody = true;
		// Opt.MaxBodyChars = 1500;
		//
		// int32 Code = -1;
		// const bool bOk = FTrailHttpDebug::LogAndIsSuccess(
		// 	TEXT("TrailsSubsystem"),
		// 	Request,
		// 	Response,
		// 	bWasSuccessful,
		// 	Opt,
		// 	&Code
		// );
		//
		// if (!bOk) return;

		// Serveur injoignable : Response est nulle, on signale l'echec au lieu de crasher
		if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
			UE_LOG(LogTemp, Error, TEXT("[PerformHttpRequestForRaces] Requete courses en echec (Code=%d)"),
				Response.IsValid() ? Response->GetResponseCode() : -1);
			if (LoadingSubsystem)
			{
				LoadingSubsystem->Fail("LoadingRaces", FText::FromString(TEXT("Server error")));
				LoadingSubsystem->Complete("LoadingRaces", FText::FromString(TEXT("Server error")));
			}
			return;
		}

		const FString JsonString = Response->GetContentAsString();
		UTrailsSubsystem* Self = WeakThis.Get();

		UE::Tasks::Launch(UE_SOURCE_LOCATION,
			[WeakThis, JsonString]()
			{
				FRaceEntries NewSnapshot;
				ConvertRacesJson(JsonString, NewSnapshot);
			
				AsyncTask(ENamedThreads::GameThread,
					[WeakThis, NewSnapshot = MoveTemp(NewSnapshot)]() mutable
					{
						if (!WeakThis.IsValid()) return;

						UTrailsSubsystem* Self = WeakThis.Get();
						Self->RacesArray = MoveTemp(NewSnapshot);
						Self->OnRacesDatasGathered.Broadcast(Self->RacesArray);
					});
			},
			UE::Tasks::ETaskPriority::BackgroundNormal);
	});
	if (!Req->ProcessRequest())
	{
		LoadingSubsystem->Fail("LoadingRaces", FText::FromString(FString(TEXT("Server error"))));
		LoadingSubsystem->Complete("LoadingRaces", FText::FromString(FString::Printf(TEXT("Server error"))));
		
	}
}

void UTrailsSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup)
{
	CurrentRaceSetup = NewRaceSetup;
}
