// All Rights Reserved


#include "PathSubsystem.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "RaceSubsystem.h"
// #include "TrailHttpDebug.h"


void UPathSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(URaceSubsystem::StaticClass());
	if (GetGameInstance())
	{
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
		if (RaceSubsystem){
			RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &UPathSubsystem::SetRaceSetup);
		}
	}
}

void UPathSubsystem::Deinitialize()
{
	RacePathsMap.Empty();
	for (auto& Pair : ActivePathRequests)
	{
		TSharedPtr<IHttpRequest> Req = Pair.Value;
		Req->OnProcessRequestComplete().Unbind();
		Req->CancelRequest();
	}
	ActivePathRequests.Empty();
	if (RaceSubsystem){
		RaceSubsystem->OnRaceSetupDatasGathered.RemoveDynamic(this, &UPathSubsystem::SetRaceSetup);
	}
	Super::Deinitialize();
}

bool ConvertPathJson(const FString& JsonString, FRacePath& OutRoot)
{
	TArray<FRacePathPoint> RacePath;
	const bool bOK = FJsonObjectConverter::JsonArrayStringToUStruct(
		JsonString,
		&RacePath,
		0,0);
	if (bOK)
	{
		OutRoot.Points = MoveTemp(RacePath);
	}
	return bOK;
}

/**
 * @brief Getting path datas
 * @param RaceID 
 * @param PathEndpoint 
 */
void UPathSubsystem::PerformHttpRequestForPath(int64 RaceID, const FString& PathEndpoint)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(PathEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	ActivePathRequests.Add(PathEndpoint, Req);
	TWeakObjectPtr<UPathSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
	[WeakThis, PathEndpoint, RaceID](FHttpRequestPtr Request, const FHttpResponsePtr Response,bool bWasSuccessful)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[PerformHttpRequestForPath] IsInGameThread() %d"), IsInGameThread());
		if (!WeakThis.IsValid()) return;
		UPathSubsystem* Self = WeakThis.Get();
		Self->ActivePathRequests.Remove(PathEndpoint);
		
		/*// Debug 
		FTrailHttpDebugOptions Opt;
		Opt.bLogHeaders = false;
		Opt.bLogBody = true;
		Opt.MaxBodyChars = 1500;

		int32 Code = -1;
		const bool bOk = FTrailHttpDebug::LogAndIsSuccess(
			TEXT("PathSubsystem"),
			Request,
			Response,
			bWasSuccessful,
			Opt,
			&Code
		);

		if (!bOk)
		{
			// gestion erreur (retry, etc.)
			return;
		}*/

		if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
			UE_LOG(LogTemp, Error, TEXT("[PerformHttpRequestForPath] Requete en echec (RaceID=%lld, Code=%d)"),
				RaceID, Response.IsValid() ? Response->GetResponseCode() : -1);
			return;
		}

		const FString JsonString = Response->GetContentAsString();
		UE::Tasks::Launch(UE_SOURCE_LOCATION,
			[WeakThis, JsonString, RaceID]()
			{
				// UE_LOG(LogTemp, Warning, TEXT("[PerformHttpRequestForPath Lambda] IsInGameThread() %d"), IsInGameThread());
				FRacePath NewSnapshot;
				ConvertPathJson(JsonString, NewSnapshot);
				AsyncTask(ENamedThreads::GameThread,
					[WeakThis, NewSnapshot = MoveTemp(NewSnapshot), RaceID]() mutable
					{
						// UE_LOG(LogTemp, Warning, TEXT("[PerformHttpRequestForPath Broadcast] IsInGameThread() %d"), IsInGameThread());
						if (!WeakThis.IsValid()) return;
						
						UPathSubsystem* Self = WeakThis.Get();
						Self->RacePath = MoveTemp(NewSnapshot);
						Self->RacePathsMap.Add(RaceID, Self->RacePath);
						
						// UE_LOG(LogTemp, Warning, TEXT("OnDataReady IsInGameThread=%d"), IsInGameThread());
						
						Self->OnPathDatasGathered.Broadcast(RaceID, Self->RacePath);
					});
			},
			UE::Tasks::ETaskPriority::BackgroundNormal);
	});
	
	if (!Req->ProcessRequest())
	{
		UE_LOG(LogTemp, Error, TEXT("[PathRequest] ProcessRequest() a échoué (%s)"),
			   *PathEndpoint);
		return;
	}
}

/**
 * @brief Returning Path actor by Race ID
 * @param RaceID 
 * @return 
 */
TObjectPtr<AActor> UPathSubsystem::GetPathById(int64 RaceID) const
{
	check(IsInGameThread());
	if (const TObjectPtr<AActor>* FoundPath = MapPaths.Find(RaceID))
	{
		AActor* PathActor = FoundPath->Get();
		// UE_LOG(LogTemp, Warning, TEXT("GET RaceID=%lld Ptr=%p Valid=%d Name=%s Class=%s"),
		// 	RaceID, FoundPath, IsValid(PathActor), *GetNameSafe(PathActor), *GetNameSafe(PathActor ? PathActor->GetClass() : nullptr));
		return PathActor;
	}
	return nullptr;
}

/**
 * @brief Spawning the Path Actor
 * @param World 
 * @param PathClass 
 * @param SpawnTransform 
 * @return 
 */
TObjectPtr<AActor> UPathSubsystem::SpawnPathActor(int64 RaceID, UWorld* World, TSubclassOf<AActor> PathClass, const FTransform& SpawnTransform)
{
	if (!World || !*PathClass)
	{
		return nullptr;
	}
	check(IsInGameThread());
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TObjectPtr<AActor> PathActor = World->SpawnActor<AActor>(PathClass, SpawnTransform, Params);
	
	// UE_LOG(LogTemp, Warning, TEXT("ADD RaceID=%lld Name=%s Class=%s"),
	// 	RaceID, *GetNameSafe(PathActor), *GetNameSafe(PathActor->GetClass()));
	
	if (!IsValid(PathActor)) return nullptr;
	MapPaths.Add(RaceID, PathActor);
	return PathActor;
}

/**
 * @brief Spawning the Km actor
 * @param World 
 * @param KmClass 
 * @param SpawnTransform 
 * @return 
 */
TObjectPtr<AActor> UPathSubsystem::SpawnKmActor(UWorld* World, TSubclassOf<AActor> KmClass, const FTransform& SpawnTransform)
{
	if (!World || !*KmClass)
	{
		return nullptr;
	}
	check(IsInGameThread());
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TObjectPtr<AActor> KmActor = World->SpawnActor<AActor>(KmClass, SpawnTransform, Params); 
	// MapPaths.Add(CurrentRaceSetup.raceId, KmActor);
	return KmActor;
}

/**
 * @brief Storing the current race setup
 * @param RaceID 
 * @param NewRaceSetup 
 */
void UPathSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup)
{
	CurrentRaceSetup = MoveTemp(NewRaceSetup);
}
