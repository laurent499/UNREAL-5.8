// All Rights Reserved

#include "RaceSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "LoadingStatusSubsystem.h"
#include "PathSubsystem.h"
#include "TemplateSubsystem.h"
#include "Engine/GameInstance.h"
// #include "TrailHttpDebug.h"
#include "TrailHttpDebug.h"
#include "Engine/World.h"

void URaceSubsystem::Deinitialize()
{
	Super::Deinitialize();
	for (auto& Pair : RaceRequest)
	{
		TSharedPtr<IHttpRequest> Req = Pair.Value;
		Req->OnProcessRequestComplete().Unbind();
		Req->CancelRequest();
	}
	RaceRequest.Empty();
}

bool ConvertRaceJson(const FString& JsonString, FRaceSetup& OutRoot)
{
	FRaceSetupRoot OutSetup;
	const bool bOk =  FJsonObjectConverter::JsonObjectStringToUStruct<FRaceSetupRoot>(
		JsonString,
		&OutSetup,
		0, 0
	);
	if (bOk)
	{
		OutRoot = MoveTemp(OutSetup.Setup);
	} 
	return bOk;
}

void URaceSubsystem::PerformHttpRequestForRaceSetup(const int64 RaceID, const FString& RaceSetupEndpoint, float Total, float Cpt)
{
	if (GetWorld()->GetGameInstance())
	{
		TObjectPtr<ULoadingStatusSubsystem> LoadingSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULoadingStatusSubsystem>();
		LoadingSubsystem->Update("LoadingRaces", Cpt / Total, FText::FromString(FString::Printf(TEXT("Race %lld"), RaceID)));
	}
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(RaceSetupEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	RaceRequest.Add(RaceID, Req);
	TWeakObjectPtr<URaceSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
		[WeakThis, RaceID](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bWasSuccessful)
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
			// 	TEXT("[RaceSubsystem] PerformHttpRequestForRaceSetup"),
			// 	Request,
			// 	Response,
			// 	bWasSuccessful,
			// 	Opt,
			// 	&Code
			// );
			//
			// if (!bOk)
			// {
			// 	// gestion erreur (retry, etc.)
			// 	return;
			// }
			URaceSubsystem* Self = WeakThis.Get();
			Self->RaceRequest.Remove(RaceID);
			if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
			{
				UE_LOG(LogTemp, Error, TEXT("[PerformHttpRequestForRaceSetup] Requete en echec (RaceID=%lld, Code=%d)"),
					RaceID, Response.IsValid() ? Response->GetResponseCode() : -1);
				return;
			}
			const FString JsonString = Response->GetContentAsString();
			UE::Tasks::Launch(UE_SOURCE_LOCATION,
				[WeakThis, JsonString, RaceID]()
				{
					FRaceSetup NewSnapshot;
					ConvertRaceJson(JsonString, NewSnapshot);
					AsyncTask(ENamedThreads::GameThread,
						[WeakThis, NewSnapshot = MoveTemp(NewSnapshot), RaceID]() mutable
						{
							if (!WeakThis.IsValid()) return;

							URaceSubsystem* Self = WeakThis.Get();
							Self->CurrentRaceSetup = MoveTemp(NewSnapshot);
							Self->RaceSetupDatasMap.Add(RaceID, Self->CurrentRaceSetup);

							// Le template doit être connu avant l'apparition des checkpoints, POI et coureurs
							const FRaceSetup Setup = Self->CurrentRaceSetup;
							auto Broadcast = [WeakThis, Setup]()
							{
								if (WeakThis.IsValid())
								{
									WeakThis->OnRaceSetupDatasGathered.Broadcast(Setup.raceId, Setup);
								}
							};
							if (UTemplateSubsystem* Templates = Self->GetGameInstance()->GetSubsystem<UTemplateSubsystem>())
							{
								Templates->RequestTemplate(Setup.templateName, MoveTemp(Broadcast));
							}
							else
							{
								Broadcast();
							}
						});
				},
				UE::Tasks::ETaskPriority::BackgroundNormal);
		}
	);
	if (!Req->ProcessRequest())
	{
		RaceRequest.Remove(RaceID);
		UE_LOG(LogTemp, Error, TEXT("[Race %lld] ProcessRequest() a échoué (%s)"),
			   (long long)RaceID, *RaceSetupEndpoint);
	}
}

FRaceSetup URaceSubsystem::GetRaceSetupByName(const FString& RaceName) const
{
	return CurrentRaceSetup;
}

const FRaceSetup URaceSubsystem::GetRaceSetupById(int64 RaceID) const
{
	return RaceSetupDatasMap.FindChecked(RaceID);
}

void URaceSubsystem::AddRaceSetup(const FRaceSetup& NewRaceSetup)
{
	RaceSetupDatasMap.Add(NewRaceSetup.raceId, NewRaceSetup);
}

void URaceSubsystem::SetCurrentRaceId(const int64 RaceId)
{
	CurrentRaceId = RaceId;
}

int64 URaceSubsystem::GetCurrentRaceId() const
{
	return CurrentRaceId;
}

void URaceSubsystem::SetCurrentRaceSetup(FRaceSetup NewRaceSetup)
{
	CurrentRaceSetup = NewRaceSetup;
}


FString URaceSubsystem::GetMainURL() const
{
	return MainURL;
}

bool URaceSubsystem::IsRaceSetupValid(const FRaceSetup& NewRaceSetup) const
{
	return NewRaceSetup.raceId != 0 ||
		!NewRaceSetup.raceName.IsEmpty() ||
		!NewRaceSetup.color.IsEmpty() ||
		!NewRaceSetup.templateName.IsEmpty();
}