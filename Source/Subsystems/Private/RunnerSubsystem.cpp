#include "RunnerSubsystem.h"
#include "JsonObjectConverter.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "RaceSubsystem.h"
#include "RunnerInterface.h"
#include "SlateNotificationsBFL.h"
#include "TrailHttpDebug.h"

void URunnerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(URaceSubsystem::StaticClass());
	if (GetGameInstance())
	{
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
		if (RaceSubsystem){
			RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &URunnerSubsystem::SetRaceSetup);
		}
	}
}

void URunnerSubsystem::Deinitialize()
{
	CancelAllRunnersRetries();
	RunnersMap.Empty();
	for (auto& Pair : ActiveRunnerRequests)
	{
		TSharedPtr<IHttpRequest> Req = Pair.Value;
		Req->OnProcessRequestComplete().Unbind();
		Req->CancelRequest();
	}
	ActiveRunnerRequests.Empty();
	if (RaceSubsystem){
		RaceSubsystem->OnRaceSetupDatasGathered.RemoveDynamic(this, &URunnerSubsystem::SetRaceSetup);
	}
	Super::Deinitialize();
}

static bool ConvertRunnersJson(const FString& JsonString, FRunners& OutRoot)
{
	TArray<FRunnerStruct> Runners;
	const bool bOk = FJsonObjectConverter::JsonArrayStringToUStruct(
		JsonString,
		&Runners,
		0, 0
	);
	if (bOk)
	{
		OutRoot.Runners = MoveTemp(Runners);
	}
	return bOk;
}

static bool ConvertRunnerJson(const FString& JsonString, FRunnerStruct& OutRoot)
{
	return FJsonObjectConverter::JsonObjectStringToUStruct<FRunnerStruct>(
		JsonString,
		&OutRoot,
		0, 0
	);
}

/**
 * @brief Request datas for all Runners at startup
 * @param RunnersEndpoint 
 */
void URunnerSubsystem::PerformHttpRequestForRunners(int64 RaceID, const FString& RunnersEndpoint, bool bStartup)
{
	// Nouvelle demande explicite : on repart d'une serie de tentatives vierge
	CancelRunnersRetry(RaceID);
	RequestRunnersWithRetry(RaceID, RunnersEndpoint, bStartup, 1);
}

/**
 * @brief Fetch des runners avec suivi du numero de tentative (retry interne)
 * @param RaceID
 * @param RunnersEndpoint
 * @param bStartup
 * @param Attempt Numero de la tentative courante, 1 pour la premiere
 */
void URunnerSubsystem::RequestRunnersWithRetry(int64 RaceID, const FString& RunnersEndpoint, bool bStartup, int32 Attempt)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(RunnersEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	TWeakObjectPtr<URunnerSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
		[WeakThis, RaceID, RunnersEndpoint, bStartup, Attempt](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			const FString Url = Request.IsValid() ? Request->GetURL() : RunnersEndpoint;
			
			if (!bWasSuccessful || !Response.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to process request: %s"), *Url);
				if (WeakThis.IsValid())
				{
					WeakThis->ScheduleRunnersRetry(RaceID, RunnersEndpoint, bStartup, Attempt, TEXT("requete echouee"));
				}
				return;
			}

			// Backend redemarre ou en erreur : on ne desserialise pas une reponse non 200
			const int32 ResponseCode = Response->GetResponseCode();
			if (ResponseCode != 200)
			{
				UE_LOG(LogTemp, Error, TEXT("[RunnerSubsystem] Race %lld : HTTP %d sur %s"), RaceID, ResponseCode, *Url);
				if (WeakThis.IsValid())
				{
					WeakThis->ScheduleRunnersRetry(
						RaceID,
						RunnersEndpoint,
						bStartup,
						Attempt,
						FString::Printf(TEXT("HTTP %d"), ResponseCode));
				}
				return;
			}
			
			// Debug 
			// FTrailHttpDebugOptions Opt;
			// Opt.bLogHeaders = false;
			// Opt.bLogBody = true;
			// Opt.MaxBodyChars = 1500;
			//
			// int32 Code = -1;
			// const bool bOk = FTrailHttpDebug::LogAndIsSuccess(
			// 	TEXT("RunnerSubsystem"),
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
			// Le backend pose cet en-tete tant que son premier cycle de fetch n'est pas termine :
			// seule une liste vide « en attente » merite une nouvelle tentative.
			const bool bBackendPending = !Response->GetHeader(TEXT("X-Runners-Pending")).IsEmpty();

			UE::Tasks::Launch(UE_SOURCE_LOCATION,
		[WeakThis, JsonString, RaceID, RunnersEndpoint, bStartup, Attempt, bBackendPending]()
			{
				FRunners NewSnapshot;
				ConvertRunnersJson(JsonString, NewSnapshot);
				AsyncTask(ENamedThreads::GameThread,
					[WeakThis, 
						NewSnapshot = MoveTemp(NewSnapshot), 
						RaceID,
						RunnersEndpoint,
						bStartup,
						Attempt,
						bBackendPending]() mutable
					{
						if (!WeakThis.IsValid()) return;

						URunnerSubsystem* Self = WeakThis.Get();

						// Au chargement, on ne reessaye un snapshot vide que si le backend signale
						// qu'il n'a pas encore produit son premier cycle. Une course sans coureur
						// (pas encore partie, terminee) est legitime : on l'accepte tout de suite,
						// les teams arriveront par le fetch periodique (spawn a chaud).
						// Avant, chaque course vide bloquait le chargement ~25 s (5 tentatives).
						if (bStartup && bBackendPending && NewSnapshot.Runners.IsEmpty())
						{
							if (Self->ScheduleRunnersRetry(RaceID, RunnersEndpoint, bStartup, Attempt, TEXT("snapshot vide au demarrage")))
							{
								return;
							}
							// Plus de tentative disponible : on laisse le flux normal signaler l'echec
						}
						else
						{
							// Tentative satisfaisante : la serie de retries n'a plus lieu d'etre
							Self->CancelRunnersRetry(RaceID);
						}

						Self->Runners = MoveTemp(NewSnapshot);
						Self->RunnersMap.Add(RaceID,  Self->Runners);
						if (bStartup)
						{
							Self->OnRunnersDatasGathered.Broadcast(RaceID, Self->Runners);
						} else
						{
							Self->OnRunnersUpdateDatas.Broadcast(RaceID, Self->Runners);
							
						}
					});
			},
		UE::Tasks::ETaskPriority::BackgroundNormal);
		}
	);
	Req->ProcessRequest();
}

/**
 * @brief Planifie une nouvelle tentative de fetch des runners (backoff 2s, 4s, 8s...)
 * @return true si une tentative a ete planifiee, false si la serie est epuisee
 */
bool URunnerSubsystem::ScheduleRunnersRetry(
	int64 RaceID,
	const FString& RunnersEndpoint,
	bool bStartup,
	int32 Attempt,
	const FString& Reason)
{
	if (Attempt >= MaxRunnersFetchAttempts)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[RunnerSubsystem] Race %lld : abandon du fetch runners apres %d tentatives (%s) - %s"),
			RaceID,
			Attempt,
			*Reason,
			*RunnersEndpoint);
		RunnersRetryTimers.Remove(RaceID);
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[RunnerSubsystem] Race %lld : impossible de planifier un retry (GameInstance nulle) - %s"),
			RaceID,
			*RunnersEndpoint);
		return false;
	}

	const int32 NextAttempt = Attempt + 1;
	const float Delay = FMath::Min(
		RunnersRetryBaseDelay * static_cast<float>(1 << (Attempt - 1)),
		RunnersRetryMaxDelay);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RunnerSubsystem] Race %lld : %s. Tentative %d/%d dans %.0fs - %s"),
		RaceID,
		*Reason,
		NextAttempt,
		MaxRunnersFetchAttempts,
		Delay,
		*RunnersEndpoint);

	TWeakObjectPtr<URunnerSubsystem> WeakThis(this);
	FTimerHandle& Handle = RunnersRetryTimers.FindOrAdd(RaceID);
	GameInstance->GetTimerManager().ClearTimer(Handle);
	GameInstance->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda(
			[WeakThis, RaceID, RunnersEndpoint, bStartup, NextAttempt]()
			{
				if (!WeakThis.IsValid()) return;

				URunnerSubsystem* Self = WeakThis.Get();
				Self->RunnersRetryTimers.Remove(RaceID);
				Self->RequestRunnersWithRetry(RaceID, RunnersEndpoint, bStartup, NextAttempt);
			}),
		Delay,
		false);

	return true;
}

/**
 * @brief Annule la serie de retries en cours pour une race
 */
void URunnerSubsystem::CancelRunnersRetry(int64 RaceID)
{
	FTimerHandle Handle;
	if (!RunnersRetryTimers.RemoveAndCopyValue(RaceID, Handle))
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->GetTimerManager().ClearTimer(Handle);
	}
}

/**
 * @brief Annule toutes les series de retries en cours
 */
void URunnerSubsystem::CancelAllRunnersRetries()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		for (auto& Pair : RunnersRetryTimers)
		{
			GameInstance->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	RunnersRetryTimers.Empty();
}

/**
 * @brief Request datas for one Runner
 * @param RunnerEndpoint 
 * @param RunnerId
 */
void URunnerSubsystem::PerformHttpRequestForRunner(int64 RaceID, const FString& RunnerEndpoint, int64 RunnerId)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(RunnerEndpoint);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	// Garder une ref pour suivre/éviter que le code réutilise une requête unique
	ActiveRunnerRequests.Add(RunnerId, Req);
	TWeakObjectPtr<URunnerSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
		[WeakThis, RunnerId, RaceID](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (!WeakThis.IsValid()) return;
			URunnerSubsystem* Self = WeakThis.Get();
			Self->ActiveRunnerRequests.Remove(RunnerId);
			
			// Debug 
			/*FTrailHttpDebugOptions Opt;
			Opt.bLogHeaders = false;
			Opt.bLogBody = true;
			Opt.MaxBodyChars = 1500;

			int32 Code = -1;
			const bool bOk = FTrailHttpDebug::LogAndIsSuccess(
				TEXT("RunnerSubsystem"),
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
				UE_LOG(LogTemp, Error, TEXT("[RunnerSubsystem] Requete coureur en echec (RaceID=%lld, Code=%d)"),
					RaceID, Response.IsValid() ? Response->GetResponseCode() : -1);
				return;
			}

			const FString JsonString = Response->GetContentAsString();
			UE::Tasks::Launch(UE_SOURCE_LOCATION,
				[WeakThis, JsonString, RaceID]()
				{
					FRunnerStruct NewSnapshot;
					ConvertRunnerJson(JsonString, NewSnapshot);
					AsyncTask(ENamedThreads::GameThread,
						[WeakThis, NewSnapshot = MoveTemp(NewSnapshot), RaceID]() mutable
						{
							if (!WeakThis.IsValid()) return;

							URunnerSubsystem* Self = WeakThis.Get();
							Self->RunnerDatas = MoveTemp(NewSnapshot);
							Self->OnRunnerDatasGathered.Broadcast(RaceID, Self->RunnerDatas);
						});
				},
				UE::Tasks::ETaskPriority::BackgroundNormal);
		}
	);

	if (!Req->ProcessRequest())
	{
		ActiveRunnerRequests.Remove(RunnerId);
		UE_LOG(LogTemp, Error, TEXT("[Runner %lld] ProcessRequest() a échoué (%s)"),
			   (long long)RunnerId, *RunnerEndpoint);
	}
}

/**
 * @brief Spawn un runner et le place
 * @param RaceID
 * @param RunnerStruct
 * @param World 
 * @param RunnerClass 
 * @param SpawnTransform 
 * @return 
 */
// AActor* URunnerSubsystem::SpawnRunnerActor(int64 RaceID, FRunnerStruct RunnerStruct, UWorld* World, TSubclassOf<AActor> RunnerClass,
//                                            const FTransform& SpawnTransform)
// {
// 	if (!World || !*RunnerClass) return nullptr;
//
// 	FActorSpawnParameters Params;
// 	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
// 	TObjectPtr<AActor> SpawnedRunner = World->SpawnActor<AActor>(RunnerClass, SpawnTransform, Params);
// 	if (!SpawnedRunner) return nullptr;
// 	RunnersActorsMap.FindOrAdd(RaceID).Add(RunnerStruct.runnerId, SpawnedRunner);
// 	return SpawnedRunner;
// }
AActor* URunnerSubsystem::SpawnRunnerActor(int64 RaceID, FRunnerStruct RunnerStruct, UWorld* World, TSubclassOf<AActor> RunnerClass,
										   const FTransform& SpawnTransform)
{
	if (!World || !*RunnerClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TObjectPtr<AActor> SpawnedRunner = World->SpawnActor<AActor>(RunnerClass, SpawnTransform, Params);
	if (!SpawnedRunner) return nullptr;
	RunnersActorsMap.FindOrAdd(RaceID).Add(RunnerStruct.canalId, SpawnedRunner);

	// Regle operateur : une team qui arrive dans une race (nouvelle, ou de retour apres un
	// passage sur une autre course) est toujours cachee. Seul un displayteam l'affiche ;
	// displayallteams ne vise que les acteurs deja presents (voir ToggleRunners).
	SpawnedRunner->SetActorHiddenInGame(true);

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(SpawnedRunner))
	{
		RunnerInterface->AssignRunnerToTeam(RunnerStruct.canalId);
	}

	return SpawnedRunner;
}

bool URunnerSubsystem::DoesRunnerBelongsToRace(int64 RunnerID, int64 RaceID)
{
	FRunners* TmpRunners = RunnersMap.Find(RaceID); 
	if (!TmpRunners) return false;
	TArray<FRunnerStruct> TmpStructs = TmpRunners->Runners;
	for (FRunnerStruct TmpStruct : TmpStructs)
	{
		if (TmpStruct.runnerId == RunnerID) return true;
	}
	return false;
}

/**
 * @brief Show/Hide Runners
 * @param RaceID 
 * @param bShow 
 */
void URunnerSubsystem::ToggleRunners(int64 RaceID, bool bShow)
{
	// N'agit que sur les acteurs presents : une team qui arrive ensuite reste cachee
	// (SpawnRunnerActor), rien n'est memorise pour elle.
	TMap<int64, TObjectPtr<AActor>>* TmpMap = RunnersActorsMap.Find(RaceID);
	if (!TmpMap)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[ToggleRunners] Runners introuvable (RaceID=%lld)"), RaceID)), EMessageType::Error);
		}
		return;
	}
	for (TPair<int64, TObjectPtr<AActor>>& RunnerPair : *TmpMap)
	{
		if (TObjectPtr<AActor> ActiveRunner = RunnerPair.Value.Get())
		{
			ActiveRunner->SetActorHiddenInGame(!bShow);
		}
	}
}

void URunnerSubsystem::ToggleRunner(int64 RaceID, int64 RunnerID, bool bShow)
{
	TObjectPtr<AActor> ActiveRunner = GetRunnerActor(RaceID, RunnerID);
	if (!ActiveRunner)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(
				FText::FromString(FString::Printf(
					TEXT("[ToggleRunner] Runner not found (RaceID=%lld RunnerID=%lld)"),
					RaceID, RunnerID)),
				EMessageType::Error);
		}
		return;
	}

	ActiveRunner->SetActorHiddenInGame(!bShow);

	if (bShow)
	{
		if (IRunnerInterface* IRunner = Cast<IRunnerInterface>(ActiveRunner))
		{
			IRunner->TriggerUpdateAfterHidden(RunnerID);
		}
	}
}

void URunnerSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup)
{
	CurrentRaceSetup = NewRaceSetup;
}

FRunners URunnerSubsystem::GetRunnersByRaceId(int64 RaceID)
{
	FRunners* TmpRunners = RunnersMap.Find(RaceID); 
	if (!TmpRunners) return FRunners();
	FRunners StructRunners = *TmpRunners;
	return StructRunners;
}

TObjectPtr<AActor> URunnerSubsystem::GetRunnerActor(int64 RaceID, int64 RunnerID)
{
	FRunners* TmpRunners = RunnersMap.Find(RaceID);
	if (!TmpRunners)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(TEXT("[GetRunnerActor] No runners snapshot (RaceID=%lld)"), RaceID)),
			EMessageType::Error);
		return nullptr;
	}

	for (const FRunnerStruct& TmpStruct : TmpRunners->Runners)
	{
		if (TmpStruct.runnerId == RunnerID)
		{
			if (TObjectPtr<AActor> RunnerActor = GetRunnerActorByTeam(RaceID, TmpStruct.canalId))
			{
				return RunnerActor;
			}

			USlateNotificationsBFL::SlateNotify(
				FText::FromString(FString::Printf(
					TEXT("[GetRunnerActor] Team actor not found (RaceID=%lld TeamID=%lld RunnerID=%lld)"),
					RaceID, TmpStruct.canalId, RunnerID)),
				EMessageType::Error);
			return nullptr;
		}
	}

	USlateNotificationsBFL::SlateNotify(
		FText::FromString(FString::Printf(
			TEXT("[GetRunnerActor] Runner not found in snapshot (RaceID=%lld RunnerID=%lld)"),
			RaceID, RunnerID)),
		EMessageType::Error);
	return nullptr;
}

TObjectPtr<AActor> URunnerSubsystem::GetRunnerActorByTeam(int64 RaceID, int64 TeamID)
{
	TMap<int64, TObjectPtr<AActor>>* TmpMap = RunnersActorsMap.Find(RaceID);
	if (!TmpMap)
	{
		return nullptr;
	}

	TObjectPtr<AActor>* RunnerTmp = TmpMap->Find(TeamID);
	if (!RunnerTmp)
	{
		return nullptr;
	}

	return RunnerTmp->Get();
}

void URunnerSubsystem::ToggleRunnerByTeam(int64 RaceID, int64 TeamID, bool bShow)
{
	TObjectPtr<AActor> ActiveRunner = GetRunnerActorByTeam(RaceID, TeamID);
	if (!ActiveRunner)
	{
		if (bShow)
		{
			USlateNotificationsBFL::SlateNotify(
				FText::FromString(FString::Printf(
					TEXT("[ToggleRunnerByTeam] Runner not found (RaceID=%lld TeamID=%lld)"),
					RaceID, TeamID)),
				EMessageType::Error);
		}
		return;
	}

	ActiveRunner->SetActorHiddenInGame(!bShow);

	if (bShow)
	{
		if (IRunnerInterface* IRunner = Cast<IRunnerInterface>(ActiveRunner))
		{
			// Le param s'appelle RunnerId dans l'interface,
			// mais ARunner n'en fait actuellement rien d'utile.
			IRunner->TriggerUpdateAfterHidden(TeamID);
		}
	}
}

/**
 * @brief Detruit les acteurs des teams absentes du snapshot.
 * @note Aucun etat n'est conserve : une team qui revient est respawnee cachee.
 * Un snapshot entierement vide n'arrive ici qu'apres EmptySnapshotGraceSeconds
 * (voir ARaceManager::UpdateRunnersFromSnapshot).
 */
void URunnerSubsystem::RemoveTeamsNotInSnapshot(int64 RaceID, const FRunners& RunnersDatas)
{
	TMap<int64, TObjectPtr<AActor>>* TeamMap = RunnersActorsMap.Find(RaceID);
	if (!TeamMap)
	{
		return;
	}

	TSet<int64> IncomingTeamIDs;
	for (const FRunnerStruct& Runner : RunnersDatas.Runners)
	{
		if (Runner.canalId > 0)
		{
			IncomingTeamIDs.Add(Runner.canalId);
		}
	}

	TArray<int64> ExistingTeamIDs;
	TeamMap->GetKeys(ExistingTeamIDs);

	for (const int64 ExistingTeamID : ExistingTeamIDs)
	{
		if (!IncomingTeamIDs.Contains(ExistingTeamID))
		{
			if (TObjectPtr<AActor>* ActorPtr = TeamMap->Find(ExistingTeamID))
			{
				if (TObjectPtr<AActor> Actor = ActorPtr->Get())
				{
					Actor->Destroy();
				}
			}

			TeamMap->Remove(ExistingTeamID);
		}
	}

	if (TeamMap->IsEmpty())
	{
		RunnersActorsMap.Remove(RaceID);
	}
}

/** 
 * @brief Start/Stop Runner animation 
 * @param RunnerID 
 * @param RaceID 
 * @note WIP
 */

void URunnerSubsystem::AnimationByTeam(int64 TeamID, int64 RaceID)
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActorByTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(
				TEXT("[AnimationByTeam] Runner introuvable (RaceID=%lld TeamID=%lld)"),
				RaceID, TeamID)),
			EMessageType::Error);
		return;
	}

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->StartAnimation(100.f);
	}
}

void URunnerSubsystem::StopAnimationByTeam(int64 TeamID, int64 RaceID)
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActorByTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(
				TEXT("[StopAnimationByTeam] Runner introuvable (RaceID=%lld TeamID=%lld)"),
				RaceID, TeamID)),
			EMessageType::Error);
		return;
	}

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->StopAnimation();
	}
}

void URunnerSubsystem::Animation(int64 RunnerID, int64 RaceID)
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActor(RaceID, RunnerID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[Animation] Runner introuvable (RaceID=%lld RunnerID=%lld)"), RaceID, RunnerID)), EMessageType::Error);
		return;
	}
	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->StartAnimation(100.f);
	}
}

void URunnerSubsystem::StopAnimation(int64 RunnerID, int64 RaceID)
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActor(RaceID, RunnerID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[StopAnimation] Runner introuvable (RaceID=%lld RunnerID=%lld)"), RaceID, RunnerID)), EMessageType::Error);
		return;
	}
	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->StopAnimation();
	}
}

void URunnerSubsystem::UpdateDayNightGlow(int64 RaceID, bool bIsDay, float NewGlow)
{
	TMap<int64, TObjectPtr<AActor>>* TmpMap = RunnersActorsMap.Find(RaceID);
	if (!TmpMap)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[UpdateDayNightGlow] Runners introuvable (RaceID=%lld)"), RaceID)), EMessageType::Error);
		return;
	}
	for (TPair<int64, TObjectPtr<AActor>>& RunnerPair : *TmpMap)
	{
		if (TObjectPtr<AActor> ActiveRunner = RunnerPair.Value.Get())
		{
			if (bIsDay)
			{
				
			} else
			{
				
			}
		}
	}
}