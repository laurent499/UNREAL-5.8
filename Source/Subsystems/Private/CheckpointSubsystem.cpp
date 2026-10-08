// All Rights Reserved


#include "CheckpointSubsystem.h"
#include "CesiumFlyToComponent.h"
#include "CheckpointInterface.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "RaceSubsystem.h"
// #include "TrailHttpDebug.h"
#include "SlateNotificationsBFL.h"
#include "Kismet/GameplayStatics.h"

void UCheckpointSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(URaceSubsystem::StaticClass());
	if (GetGameInstance())
	{
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
		if (RaceSubsystem){
			RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &UCheckpointSubsystem::SetRaceSetup);
		}
	}
}

void UCheckpointSubsystem::Deinitialize()
{
	CheckpointsMap.Empty();
	for (auto& Pair : ActiveCheckpointRequests)
	{
		TSharedPtr<IHttpRequest> Req = Pair.Value;
		Req->OnProcessRequestComplete().Unbind();
		Req->CancelRequest();
	}
	ActiveCheckpointRequests.Empty();
	if (RaceSubsystem){
		RaceSubsystem->OnRaceSetupDatasGathered.RemoveDynamic(this, &UCheckpointSubsystem::SetRaceSetup);
	}
	Super::Deinitialize();
	
}

FCheckpoints* UCheckpointSubsystem::GetCheckpointsByRaceId(int64 RaceID)
{
	return CheckpointsMap.Find(RaceID);
}

FRaceCheckpoint UCheckpointSubsystem::GetCheckpointByChkIdRaceId(int64 RaceID, int64 ChkID)
{
	FCheckpoints* Chks = CheckpointsMap.Find(RaceID);
	return Chks->Checkpoints[ChkID];
	
}

bool ConvertCheckpointsJson(const FString& JsonString, FCheckpoints& OutRoot)
{
	TArray<FRaceCheckpoint> CheckpointsArray;
	const bool bOk =  FJsonObjectConverter::JsonArrayStringToUStruct(
		JsonString,
		&CheckpointsArray,
		0, 0
	);
	if (bOk)
	{
		OutRoot.Checkpoints = MoveTemp(CheckpointsArray);
	}
	return bOk;
}

bool ConvertCheckpointJson(const FString& JsonString, FRaceCheckpoint& OutRoot)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct<FRaceCheckpoint>(
		JsonString,
		&OutRoot,
		0, 0
	);
}

/**
 * @brief Request datas for all Checkpoints
 * @param RaceID
 * @param CheckpointsEndpoint 
 */
void UCheckpointSubsystem::PerformHttpRequestForCheckpoints(int64 RaceID, const FString& CheckpointsEndpoint)
{
TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(CheckpointsEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	TWeakObjectPtr<UCheckpointSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
		[this, RaceID, WeakThis]
		(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (!WeakThis.IsValid()) return;

			if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
			{
				UE_LOG(LogTemp, Error, TEXT("[PerformHttpRequestForCheckpoints] Requete en echec (RaceID=%lld, Code=%d)"),
					RaceID, Response.IsValid() ? Response->GetResponseCode() : -1);
				return;
			}

			const FString JsonString = Response->GetContentAsString();
			UE::Tasks::Launch(UE_SOURCE_LOCATION,
		[WeakThis, JsonString, RaceID]()
				{
					FCheckpoints NewSnapshot;
					ConvertCheckpointsJson(JsonString, NewSnapshot);
					AsyncTask(ENamedThreads::GameThread,
						[WeakThis, RaceID, NewSnapshot = MoveTemp(NewSnapshot)]() mutable
						{
							if (!WeakThis.IsValid()) return;

							UCheckpointSubsystem* Self = WeakThis.Get();
							Self->Checkpoints = NewSnapshot;
							Self->CheckpointsMap.Add(RaceID, MoveTemp(NewSnapshot));
							
							const FCheckpoints* Added = Self->CheckpointsMap.Find(RaceID);
							Self->OnCheckpointsDatasGathered.Broadcast(RaceID, Self->Checkpoints);
						});
				},
		UE::Tasks::ETaskPriority::BackgroundNormal);
		}
	);
	Req->ProcessRequest();
}

/**
 * @brief Spawn Chk Actor
 * @param RaceID
 * @param CheckpointID
 * @param World 
 * @param CheckpointClass 
 * @param SpawnTransform 
 * @return 
 */
AActor* UCheckpointSubsystem::SpawnCheckpointActor(
	int64 RaceID,
	int64 CheckpointID,
	UWorld* World, 
	TSubclassOf<AActor> CheckpointClass, 
	const FTransform& SpawnTransform)
{
	if (!World || !*CheckpointClass)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TObjectPtr<AActor> SpawnedActor = World->SpawnActor<AActor>(CheckpointClass, SpawnTransform, Params);
	CheckpointActorMap.FindOrAdd(RaceID).Add(CheckpointID, SpawnedActor);
	return SpawnedActor;
}

bool UCheckpointSubsystem::DoesChkBelongsToRace(int64 ChkID, int64 RaceID)
{
	FCheckpoints* TmpChks = CheckpointsMap.Find(RaceID); 
	if (!TmpChks) return false;
	
	TArray<FRaceCheckpoint> TmpStructs = TmpChks->Checkpoints;
	for (const FRaceCheckpoint& TmpStruct : TmpStructs)
	{
		if (TmpStruct.checkpointId == ChkID) return true;
	}
	return false;
}

/**
 * @brief Show/Hide Checkpoints
 * @param RaceID 
 * @param bShow 
 */
void UCheckpointSubsystem::ToggleCheckpoints(int64 RaceID, bool bShow)
{
	TMap<int64, TObjectPtr<AActor>>* TmpMap = CheckpointActorMap.Find(RaceID);
	if (!TmpMap)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[ToggleCheckpoints] Checkpoints introuvable (RaceID=%lld)"), RaceID)), EMessageType::Error);
		}
		return;
	}
	for (TPair<int64, TObjectPtr<AActor>>& CheckpointPair : *TmpMap)
	{
		if (TObjectPtr<AActor> ActiveCheckpoint = CheckpointPair.Value.Get())
		{
			ActiveCheckpoint->SetActorHiddenInGame(!bShow);
		}
	}
}

/**
 * @brief Show/Hide Poi
 * @param RaceID 
 * @param CheckpointID 
 * @param bShow 
 */
void UCheckpointSubsystem::ToggleCheckpoint(int64 CheckpointID, int64 RaceID, bool bShow)
{
	TObjectPtr<AActor> ActiveCheckpoint = GetCheckpointActor(RaceID, CheckpointID);
	if (!ActiveCheckpoint)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[ToggleCheckpoints] Checkpoints introuvable (RaceID=%lld)"), RaceID)), EMessageType::Error);
		}
		return;
	}
	
	ActiveCheckpoint->SetActorHiddenInGame(!bShow);
}

/**
 * @brief Show/Hide the Weather components
 * @param RaceID 
 * @param CheckpointID 
 * @param bShow 
 */
void UCheckpointSubsystem::ToggleWeather(int64 RaceID, int64 CheckpointID, bool bShow)
{
	TObjectPtr<AActor> CheckpointActor = GetCheckpointActor(RaceID, CheckpointID);
	if (!CheckpointActor)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[ToggleCheckpoints] Checkpoints introuvable (RaceID=%lld)"), RaceID)), EMessageType::Error);
		}
		return;
	}
	if (ICheckpointInterface* CheckpointInterface = Cast<ICheckpointInterface>(CheckpointActor))
	{
		CheckpointInterface->ToggleCheckpointWeather(bShow);
	} 
}

/**
 * @brief Teleport to Poi
 * @param CheckpointID 
 * @param RaceID 
 */
void UCheckpointSubsystem::Teleport(int64 CheckpointID, int64 RaceID)
{
	TObjectPtr<AActor> CheckpointActor = GetCheckpointActor(RaceID, CheckpointID);
	if (!CheckpointActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[Teleport] Checkpoint introuvable (RaceID=%lld CheckpointID=%lld)"), RaceID, CheckpointID)), EMessageType::Error);
		return;
	}
	APawn* DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	UCesiumFlyToComponent* FlyComp = Cast<UCesiumFlyToComponent>(DynaPawn->GetComponentByClass(UCesiumFlyToComponent::StaticClass()));

	FVector Destination = CheckpointActor->GetActorLocation();
	// Destination.Z += 5000.f;
	FlyComp->FlyToLocationUnreal(Destination, 0.f, 0.f, false );
	
	
}

/** 
 * @brief Start/Stop Checkpoint animation 
 * @param CheckpointID 
 * @param RaceID 
 * @note WIP
 */
void UCheckpointSubsystem::Animation(int64 CheckpointID, int64 RaceID)
{
	TObjectPtr<AActor> CheckpointActor = GetCheckpointActor(RaceID, CheckpointID);
	if (!CheckpointActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[Animation] Checkpoint introuvable (RaceID=%lld CheckpointID=%lld)"), RaceID, CheckpointID)), EMessageType::Error);
		return;
	}
	if (ICheckpointInterface* CheckpointInterface = Cast<ICheckpointInterface>(CheckpointActor))
	{
		CheckpointInterface->StartAnimation(100.f);
	}
}
void UCheckpointSubsystem::StopAnimation(int64 CheckpointID, int64 RaceID)
{
	TObjectPtr<AActor> CheckpointActor = GetCheckpointActor(RaceID, CheckpointID);
	if (!CheckpointActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[StopAnimation] Checkpoint introuvable (RaceID=%lld CheckpointID=%lld)"), RaceID, CheckpointID)), EMessageType::Error);
		return;
	}
	if (ICheckpointInterface* CheckpointInterface = Cast<ICheckpointInterface>(CheckpointActor))
	{
		CheckpointInterface->StopAnimation();
	}
}

/** HELPERS */
void UCheckpointSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewCurrentRaceSetup)
{
	CurrentRaceSetup = NewCurrentRaceSetup;
}
TObjectPtr<AActor> UCheckpointSubsystem::GetCheckpointActor(int64 RaceID, int64 CheckpointID)
{
	TMap<int64, TObjectPtr<AActor>>* TmpMap = CheckpointActorMap.Find(RaceID);
	if (!TmpMap)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[GetCheckpointActor] Checkpoints introuvables (RaceID=%lld CheckpointID=%lld)"), RaceID, CheckpointID)), EMessageType::Error);
		return nullptr;
	}
	TObjectPtr<AActor>* CheckpointActor = TmpMap->Find(CheckpointID);
	
	if (!CheckpointActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf( TEXT("[GetCheckpointActor] Checkpoint introuvable (RaceID=%lld CheckpointID=%lld)"), RaceID, CheckpointID)), EMessageType::Error);
		return nullptr;
	}
	return CheckpointActor->Get();
}