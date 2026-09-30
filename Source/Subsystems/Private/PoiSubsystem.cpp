// All Rights Reserved


#include "PoiSubsystem.h"

#include "CesiumFlyToComponent.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "PoiInterface.h"
#include "Interfaces/IHttpResponse.h"
#include "RaceSubsystem.h"
// #include "TrailHttpDebug.h"
#include "SlateNotificationsBFL.h"
#include "TrailHttpDebug.h"
#include "Kismet/GameplayStatics.h"

void UPoiSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(URaceSubsystem::StaticClass());
	if (GetGameInstance())
	{
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
		if (RaceSubsystem){
			RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &UPoiSubsystem::SetRaceSetup);
		}
	}
}

void UPoiSubsystem::Deinitialize()
{
	RacePoisMap.Empty();
	for (auto& Pair : ActivePoiRequests)
	{
		TSharedPtr<IHttpRequest> Req = Pair.Value;
		Req->OnProcessRequestComplete().Unbind();
		Req->CancelRequest();
	}
	ActivePoiRequests.Empty();
	if (RaceSubsystem){
		RaceSubsystem->OnRaceSetupDatasGathered.RemoveDynamic(this, &UPoiSubsystem::SetRaceSetup);
	}
	Super::Deinitialize();
}

bool ConvertPOIsJson(const FString& JsonString, FPOIs& OutRoot)
{
	TArray<FRacePOI> PointsOfInterest;
	const bool bOk =  FJsonObjectConverter::JsonArrayStringToUStruct(
		JsonString,
		&PointsOfInterest,
		0, 0
	);
	if (bOk)
	{
		OutRoot.POIs = MoveTemp(PointsOfInterest); 
	}
	return bOk;
}

bool ConvertPOIJson(const FString& JsonString, FRacePOI& OutRoot)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct<FRacePOI>(
		JsonString,
		&OutRoot,
		0, 0
	);
}

/**
 * @brief Request datas for all POIs
 * @param POIsEndpoint 
 */
void UPoiSubsystem::PerformHttpRequestForPOIs(int64 RaceID, const FString& POIsEndpoint)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(POIsEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	TWeakObjectPtr<UPoiSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
	[WeakThis, RaceID](FHttpRequestPtr Request, const FHttpResponsePtr Response,bool bWasSuccessful)
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
		// 	TEXT("PerformHttpRequestForPOIs"),
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
		
		const FString JsonString = Response->GetContentAsString();

		UE::Tasks::Launch(UE_SOURCE_LOCATION,
			[WeakThis, JsonString, RaceID]()
			{
				FPOIs NewSnapshot;
				ConvertPOIsJson(JsonString, NewSnapshot);
			
				AsyncTask(ENamedThreads::GameThread,
					[WeakThis, NewSnapshot = MoveTemp(NewSnapshot), RaceID]() mutable
					{
						if (!WeakThis.IsValid()) return;

						UPoiSubsystem* Self = WeakThis.Get();
						Self->POIs = MoveTemp(NewSnapshot);
						Self->RacePoisMap.Add(RaceID, Self->POIs);
						Self->OnPoisDatasGathered.Broadcast(RaceID, Self->POIs);
					});
			},
			UE::Tasks::ETaskPriority::BackgroundNormal);
	});

	Req->ProcessRequest();
}

/**
 * @brief Spawn Poi Actor
 * @param RaceID
 * @param PoiID
 * @param World 
 * @param PoiClass 
 * @param SpawnTransform 
 * @return 
 */
AActor* UPoiSubsystem::SpawnPoi(int64 RaceID, int64 PoiID, UWorld* World, TSubclassOf<AActor> PoiClass, const FTransform& SpawnTransform)
{
	if (!World || !*PoiClass)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TObjectPtr<AActor> SpawnedPoi = World->SpawnActor<AActor>(PoiClass, SpawnTransform, Params);
	PoisActorsMap.FindOrAdd(RaceID).Add(PoiID, SpawnedPoi);
	
	return SpawnedPoi;
}

bool UPoiSubsystem::DoesPoiBelongsToRace(int64 PoiID, int64 RaceID)
{
	FPOIs* TmpPois = RacePoisMap.Find(RaceID); 
	if (!TmpPois) return false;
	TArray<FRacePOI> TmpStructs = TmpPois->POIs;
	for (FRacePOI TmpStruct : TmpStructs)
	{
		if (TmpStruct.poiId == PoiID) return true;
	}
	return false;
}

/**
 * @brief Show/Hide Pois
 * @param RaceID 
 * @param bShow 
 */
void UPoiSubsystem::TogglePois(int64 RaceID, bool bShow)
{
	TMap<int64, TObjectPtr<AActor>>* TmpMap = PoisActorsMap.Find(RaceID);
	
	if (!TmpMap)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[TogglePois] Pois introuvables (RaceID=%lld)"), RaceID)), EMessageType::Error);
		}
		return;
	}
	for (TPair<int64, TObjectPtr<AActor>>& PoiPair : *TmpMap)
	{
		if (TObjectPtr<AActor> Poi = PoiPair.Value.Get())
		{
			Poi->SetActorHiddenInGame(!bShow);
		}
	}
}

/**
 * @brief Show/Hide Poi
 * @param RaceID 
 * @param PoiID 
 * @param bShow 
 */
void UPoiSubsystem::TogglePoi(int64 RaceID, int64 PoiID, bool bShow)
{
	TObjectPtr<AActor> PoiActor = GetPoiActor(PoiID, RaceID);
	
	if (!PoiActor)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[TogglePoi] Poi introuvable (RaceID=%lld PoiID=%lld)"), RaceID, PoiID)), EMessageType::Error);
		}
		return;
	}
	
	PoiActor->SetActorHiddenInGame(!bShow);
}

/**
 * @brief Show/Hide the Weather components
 * @param RaceID 
 * @param PoiID 
 * @param bShow 
 */
void UPoiSubsystem::ToggleWeather(int64 RaceID, int64 PoiID, bool bShow)
{
	TObjectPtr<AActor> PoiActor = GetPoiActor(PoiID, RaceID);
	
	if (!PoiActor)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[ToggleWeather] Poi introuvable (RaceID=%lld PoiID=%lld)"), RaceID, PoiID)), EMessageType::Error);
		}
		return;
	}
	
	if (IPoiInterface* PoiInterface = Cast<IPoiInterface>(PoiActor))
	{
		PoiInterface->TogglePoiWeather(bShow);
	} 
}

/**
 * @brief Teleport to Poi
 * @param PoiID 
 * @param RaceID 
 */
void UPoiSubsystem::Teleport(int64 PoiID, int64 RaceID)
{
	TObjectPtr<AActor> PoiActor = GetPoiActor(PoiID, RaceID);
	if (!PoiActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[Teleport] Poi introuvable (RaceID=%lld PoiID=%lld)"), RaceID, PoiID)), EMessageType::Error);
		return;
	}
	
	APawn* DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	UCesiumFlyToComponent* FlyComp = Cast<UCesiumFlyToComponent>(DynaPawn->GetComponentByClass(UCesiumFlyToComponent::StaticClass()));

	FVector Destination = PoiActor->GetActorLocation();
	Destination.Z+=5000.f;
	FlyComp->FlyToLocationUnreal(Destination, 0.f, 0.f, false );
}

/** 
 * @brief Start/Stop Poi animation 
 * @param PoiID 
 * @param RaceID 
 * @note WIP
 */
void UPoiSubsystem::Animation(int64 PoiID, int64 RaceID)
{
	TObjectPtr<AActor> PoiActor = GetPoiActor(PoiID, RaceID);
	if (!PoiActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[Animation] Poi introuvable (RaceID=%lld PoiID=%lld)"), RaceID, PoiID)), EMessageType::Error);
		return;
	}
	if (IPoiInterface* PoiInterface = Cast<IPoiInterface>(PoiActor))
	{
		PoiInterface->StartAnimation(100.f);
	}
}
void UPoiSubsystem::StopAnimation(int64 PoiID, int64 RaceID)
{
	TObjectPtr<AActor> PoiActor = GetPoiActor(PoiID, RaceID);
	if (!PoiActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[StopAnimation] Poi introuvable (RaceID=%lld PoiID=%lld)"), RaceID, PoiID)), EMessageType::Error);
		return;
	}
	if (IPoiInterface* PoiInterface = Cast<IPoiInterface>(PoiActor))
	{
		PoiInterface->StopAnimation();
	}
}

/** HELPERS */
void UPoiSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewCurrentRaceSetup)
{
	CurrentRaceSetup = NewCurrentRaceSetup;
}

TObjectPtr<AActor> UPoiSubsystem::GetPoiActor(int64 PoiID, int64 RaceID)
{
	TMap<int64, TObjectPtr<AActor>>* TmpMap = PoisActorsMap.Find(RaceID);
	if (!TmpMap)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[GetPoiActor] Pois introuvables (RaceID=%lld PoiID=%lld)"), RaceID, PoiID)), EMessageType::Error);
		return nullptr;
	}
	TObjectPtr<AActor>* PoiTmp = TmpMap->Find(PoiID);
	if (!PoiTmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[GetPoiActor] Poi introuvable (RaceID=%lld PoiID=%lld)"), RaceID, PoiID)), EMessageType::Error);
		return nullptr;
	}
	if (TObjectPtr<AActor> ActivePoi = PoiTmp->Get())
	{
		return ActivePoi;
	}
	return nullptr;
}