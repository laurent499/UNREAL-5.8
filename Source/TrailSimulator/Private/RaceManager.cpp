// All Rights Reserved

#include "RaceManager.h"
#include "CesiumFlyToComponent.h"
#include "CesiumGeoreference.h"
#include "CesiumOriginShiftComponent.h"
#include "CheckpointSubsystem.h"
#include "PathSubsystem.h"
#include "PoiSubsystem.h"
#include "HttpGatewaySubsystem.h"
#include "LoadingStatusSubsystem.h"
#include "OWLViewportCapture.h"
#include "RunnerSubsystem.h"
#include "RaceSubsystem.h"
#include "ScaleSubsystem.h"
#include "TeamSubsystem.h"
#include "SettingsSubsystem.h"
#include "SlateNotificationsBFL.h"
#include "TeamGroupSubsystem.h"
#include "WeatherSubsystem.h"
#include "Actors/Path.h"
#include "TrailsSubsystem.h"
#include "TrailSharedTypes.h"
#include "WorldUtils.h"
#include "Actors/Runner.h"
#include "Actors/Poi.h"
#include "Actors/Generic/Poi_Generic.h"
#include "Actors/Generic/Runner_Generic.h"
#include "Actors/GTWS/Poi_GTWS.h"
#include "Actors/GTWS/Runner_GTWS.h"
#include "Actors/Nike/Poi_Nike.h"
#include "Actors/Nike/Runner_Nike.h"
#include "Actors/UTMB/Poi_UTMB.h"
#include "Actors/UTMB/Runner_UTMB.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SLoadingOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Engine/EngineTypes.h"
#include "Math/UnrealMathUtility.h" // FMath::IsFinite

/**
 * @brief Ensure Subsystems in Internal methods
 * @return 
 */
bool ARaceManager::RefreshSubsystems() const
{
	UGameInstance* GI = GetGameInstance();
	UWorld* W = GetWorld();
	if (!GI) return false;

	TrailsSubsystem      = GI->GetSubsystem<UTrailsSubsystem>();
	RaceSubsystem        = GI->GetSubsystem<URaceSubsystem>();
	PathSubsystem        = GI->GetSubsystem<UPathSubsystem>();
	PoiSubsystem         = GI->GetSubsystem<UPoiSubsystem>();
	RunnerSubsystem      = GI->GetSubsystem<URunnerSubsystem>();
	TeamSubsystem        = GI->GetSubsystem<UTeamSubsystem>();
	SettingsSubsystem    = GI->GetSubsystem<USettingsSubsystem>();
	WeatherSubsystem     = GI->GetSubsystem<UWeatherSubsystem>();
	LoadingSubsystem     = GI->GetSubsystem<ULoadingStatusSubsystem>();
	CheckpointSubsystem  = GI->GetSubsystem<UCheckpointSubsystem>();
	HttpGatewaySubsystem = GI->GetSubsystem<UHttpGatewaySubsystem>();
	TeamGroupSubsystem   = GI->GetSubsystem<UTeamGroupSubsystem>();
	ScaleSubsystem		 = W->GetSubsystem<UScaleSubsystem>();

	return	IsValid(TrailsSubsystem) && 
			IsValid(RaceSubsystem) &&
			IsValid(PathSubsystem) &&
			IsValid(PoiSubsystem) &&
			IsValid(RunnerSubsystem) &&
			IsValid(TeamSubsystem) &&
			IsValid(SettingsSubsystem) &&
			IsValid(WeatherSubsystem) &&
			IsValid(LoadingSubsystem) &&
			IsValid(CheckpointSubsystem) &&
			IsValid(HttpGatewaySubsystem) &&
			IsValid(TeamGroupSubsystem) &&
			IsValid(ScaleSubsystem);
}

/** 
 * @brief Ensure Subsystems in EWorldType::PIE
 * @return 
 */
bool ARaceManager::RefreshSubsystemsFromPIE() const
{
#if WITH_EDITOR
	if (!GEngine) return false;

	for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
	{
		if (Ctx.WorldType == EWorldType::PIE && Ctx.World())
		{
			UWorld* W = Ctx.World();
			
			if (UGameInstance* GI = Ctx.World()->GetGameInstance())
			{
				TrailsSubsystem      = GI->GetSubsystem<UTrailsSubsystem>();
				RaceSubsystem        = GI->GetSubsystem<URaceSubsystem>();
				PathSubsystem        = GI->GetSubsystem<UPathSubsystem>();
				PoiSubsystem         = GI->GetSubsystem<UPoiSubsystem>();
				RunnerSubsystem      = GI->GetSubsystem<URunnerSubsystem>();
				TeamSubsystem        = GI->GetSubsystem<UTeamSubsystem>();
				SettingsSubsystem    = GI->GetSubsystem<USettingsSubsystem>();
				WeatherSubsystem     = GI->GetSubsystem<UWeatherSubsystem>();
				LoadingSubsystem     = GI->GetSubsystem<ULoadingStatusSubsystem>();
				CheckpointSubsystem  = GI->GetSubsystem<UCheckpointSubsystem>();
				HttpGatewaySubsystem = GI->GetSubsystem<UHttpGatewaySubsystem>();
				TeamGroupSubsystem   = GI->GetSubsystem<UTeamGroupSubsystem>();
				ScaleSubsystem		 = W->GetSubsystem<UScaleSubsystem>();
				return	IsValid(TrailsSubsystem) && 
						IsValid(RaceSubsystem) &&
						IsValid(PathSubsystem) &&
						IsValid(PoiSubsystem) &&
						IsValid(RunnerSubsystem) &&
						IsValid(TeamSubsystem) &&
						IsValid(SettingsSubsystem) &&
						IsValid(WeatherSubsystem) &&
						IsValid(LoadingSubsystem) &&
						IsValid(CheckpointSubsystem) &&
						IsValid(HttpGatewaySubsystem) &&
						IsValid(TeamGroupSubsystem) &&
						IsValid(ScaleSubsystem);
			}
		}
	}
#endif
	return false;
}

static FString SanitizeKey(FString S)
{
	S.ReplaceInline(TEXT(" "), TEXT(""));
	S.ReplaceInline(TEXT("_"), TEXT(""));
	S.ToLowerInline();
	return S;
}

static bool CallBP_GetFloatOut(UObject* Obj, FName FunctionName, const FString& OutParamWanted, float& OutValue)
{
	OutValue = 0.f;
	if (!IsValid(Obj)) return false;

	UFunction* Fn = Obj->FindFunction(FunctionName);
	if (!Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Function '%s' not found on %s"),
			*FunctionName.ToString(), *Obj->GetClass()->GetName());
		return false;
	}

	TArray<uint8> Params;
	Params.SetNumZeroed(Fn->ParmsSize);

	Obj->ProcessEvent(Fn, Params.GetData());

	const FString WantedKey = SanitizeKey(OutParamWanted);

	FProperty* Best = nullptr;
	FProperty* FirstOut = nullptr;

	for (TFieldIterator<FProperty> It(Fn); It; ++It)
	{
		FProperty* P = *It;
		if (!P->HasAnyPropertyFlags(CPF_Parm)) continue;

		const bool bIsOut = P->HasAnyPropertyFlags(CPF_OutParm);
		if (!bIsOut) continue;

		if (!FirstOut) FirstOut = P;

		// Match par nom interne
		if (SanitizeKey(P->GetName()) == WantedKey)
		{
			Best = P;
			break;
		}

		// Match par DisplayName (meta)
		const FString Display = P->GetMetaData(TEXT("DisplayName"));
		if (!Display.IsEmpty() && SanitizeKey(Display) == WantedKey)
		{
			Best = P;
			break;
		}
	}

	FProperty* Target = Best ? Best : FirstOut;
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Function '%s' has no out params"), *FunctionName.ToString());
		return false;
	}

	const void* Ptr = Target->ContainerPtrToValuePtr<void>(Params.GetData());

	if (const FNumericProperty* Num = CastField<FNumericProperty>(Target))
	{
		if (Num->IsFloatingPoint())
		{
			OutValue = (float)Num->GetFloatingPointPropertyValue(Ptr);
			return true;
		}
		OutValue = (float)Num->GetSignedIntPropertyValue(Ptr);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Out param '%s' is not numeric (type=%s)"),
		*Target->GetName(), *Target->GetClass()->GetName());
	return false;
}

ARaceManager::ARaceManager()
{
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FClassFinder<AActor> SkyBPClass(
		TEXT("/Game/UltraDynamicSky/Blueprints/Ultra_Dynamic_Sky") 
	);	
	if (SkyBPClass.Succeeded())
	{
		UDSClass = SkyBPClass.Class;
	} 
}

/**
 * @brief BeginPlay
 * @note Initializes Subsystems, Binds Datas gathered, Calls first requests
 */
void ARaceManager::BeginPlay()
{
	Super::BeginPlay();
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	
	// Bindings
	// Datas
	TrailsSubsystem->OnRacesDatasGathered.AddDynamic(this, &ARaceManager::HandleRacesDatasGathered);
	RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &ARaceManager::HandleRaceSetupDatasGathered);
	PathSubsystem->OnPathDatasGathered.AddDynamic(this, &ARaceManager::HandlePathDatasGathered);
	RunnerSubsystem->OnRunnersDatasGathered.AddDynamic(this, &ARaceManager::HandleRunnersDatasGathered);
	RunnerSubsystem->OnRunnersUpdateDatas.AddDynamic(this, &ARaceManager::HandleUpdateRunners);
	PoiSubsystem->OnPoisDatasGathered.AddDynamic(this, &ARaceManager::HandleRacePoisDatasGathered);
	
	/**
	 * HttpGatewaySubsystem bindings
	 */ 
	// Race
	HttpGatewaySubsystem->OnMoveToRace.AddUObject(this, &ARaceManager::RtMoveToPath);
	HttpGatewaySubsystem->OnDisplayRace.AddUObject(this, &ARaceManager::RtToggleRace);
	HttpGatewaySubsystem->OnRefreshRace.AddUObject(this, &ARaceManager::RtRefreshRace);
	HttpGatewaySubsystem->OnDisplayPath.AddUObject(this, &ARaceManager::RtTogglePath);
	HttpGatewaySubsystem->OnDisplaySlope.AddUObject(this, &ARaceManager::RtToggleSlope);
	HttpGatewaySubsystem->OnDisplayKms.AddUObject(this, &ARaceManager::RtToggleKms);
	HttpGatewaySubsystem->OnDisplayPulse.AddUObject(this, &ARaceManager::RtTogglePulse);
	HttpGatewaySubsystem->OnFlyForward.AddUObject(this, &ARaceManager::RtStartTravelFwd);
	HttpGatewaySubsystem->OnFlyStop.AddUObject(this, &ARaceManager::RtStopTravel);
	HttpGatewaySubsystem->OnFlyPause.AddUObject(this, &ARaceManager::RtPauseTravel);
	HttpGatewaySubsystem->OnFlyBackward.AddUObject(this, &ARaceManager::RtStartTravelBkd);
	
	// Teams
	HttpGatewaySubsystem->OnDisplayTeams.AddUObject(this, &ARaceManager::RtToggleTeams);
	HttpGatewaySubsystem->OnDisplayTeam.AddUObject(this, &ARaceManager::RtToggleTeam);
	HttpGatewaySubsystem->OnDisplayFlag.AddUObject(this, &ARaceManager::RtToggleFlag);
	HttpGatewaySubsystem->OnDisplayClub.AddUObject(this, &ARaceManager::RtToggleClub);
	HttpGatewaySubsystem->OnDisplayPhoto.AddUObject(this, &ARaceManager::RtTogglePhoto);
	HttpGatewaySubsystem->OnTeleportToTeam.AddUObject(this, &ARaceManager::RtTeleportToTeam);
	HttpGatewaySubsystem->OnTeamAnimation.AddUObject(this, &ARaceManager::RtAnimTeam);
	HttpGatewaySubsystem->OnTeamStopAnimation.AddUObject(this, &ARaceManager::RtStopAnimTeam);
	HttpGatewaySubsystem->OnFollowTeam.AddUObject(this, &ARaceManager::RtAnimTeam);
	HttpGatewaySubsystem->OnStopFollowTeam.AddUObject(this, &ARaceManager::RtStopAnimTeam);
	
	// Teamgroups
	HttpGatewaySubsystem->OnAddTeamToGroup.AddUObject(this, &ARaceManager::RtAddTeamToGroup);
	HttpGatewaySubsystem->OnRemoveTeamFromGroup.AddUObject(this, &ARaceManager::RtRemoveTeamFromGroup);
	HttpGatewaySubsystem->OnToggleGroup.AddUObject(this, &ARaceManager::RtToggleGroup);
	
	// Checkpoints
	HttpGatewaySubsystem->OnDisplayCheckpoint.AddUObject(this, &ARaceManager::RtToggleCheckpoint);
	HttpGatewaySubsystem->OnDisplayCheckpoints.AddUObject(this, &ARaceManager::RtToggleCheckpoints);
	HttpGatewaySubsystem->OnDisplayCheckpointWeather.AddUObject(this, &ARaceManager::RtToggleCheckpointWeather);
	HttpGatewaySubsystem->OnCheckpointTeleport.AddUObject(this, &ARaceManager::RtTeleportToCheckpoint);
	HttpGatewaySubsystem->OnCheckpointAnimation.AddUObject(this, &ARaceManager::RtAnimCheckpoint);
	HttpGatewaySubsystem->OnCheckpointStopAnimation.AddUObject(this, &ARaceManager::RtStopAnimCheckpoint);
	
	// Pois
	HttpGatewaySubsystem->OnDisplayPoi.AddUObject(this, &ARaceManager::RtTogglePoi);
	HttpGatewaySubsystem->OnDisplayPois.AddUObject(this, &ARaceManager::RtTogglePois);
	HttpGatewaySubsystem->OnDisplayPoiWeather.AddUObject(this, &ARaceManager::RtTogglePoiWeather);
	HttpGatewaySubsystem->OnPoiTeleport.AddUObject(this, &ARaceManager::RtTeleportToPoi);
	HttpGatewaySubsystem->OnPoiAnimation.AddUObject(this, &ARaceManager::RtAnimPoi);
	HttpGatewaySubsystem->OnPoiStopAnimation.AddUObject(this, &ARaceManager::RtStopAnimPoi);
	
	// Settings
	HttpGatewaySubsystem->OnUpdateFetch.AddUObject(this, &ARaceManager::RtUpdateFetchFrequency);
	HttpGatewaySubsystem->OnUpdateOffset.AddUObject(this, &ARaceManager::RtChangePathOffset);
	HttpGatewaySubsystem->OnUpdateGlow.AddUObject(this, &ARaceManager::RtUpdateGlowIntensity);
	HttpGatewaySubsystem->OnUpdatePulse.AddUObject(this, &ARaceManager::RtUpdatePulseFrequency);
	HttpGatewaySubsystem->OnUpdatePulseGlow.AddUObject(this, &ARaceManager::RtUpdatePulseGlow);
	HttpGatewaySubsystem->OnMinMaxUpdate.AddUObject(this, &ARaceManager::RtUpdateMinMax);
	HttpGatewaySubsystem->OnNearFarUpdate.AddUObject(this, &ARaceManager::RtUpdateNearFar);
	HttpGatewaySubsystem->OnMainUrlUpdate.AddUObject(this, &ARaceManager::RtUpdateMainUrl);
	
	// Camera
	HttpGatewaySubsystem->OnCameraAngleUpdated.AddUObject(this, &ARaceManager::RtUpdateCameraAngle);
	HttpGatewaySubsystem->OnArmLengthUpdated.AddUObject(this, &ARaceManager::RtUpdateUpdateArmLength);
	HttpGatewaySubsystem->OnZAnchorUpdated.AddUObject(this, &ARaceManager::RtUpdateCameraAnchor);
	
	// Pawn view
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		OriginalViewTarget = PC->GetViewTarget();
	}
	DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	FlyComp = Cast<UCesiumFlyToComponent>(DynaPawn->GetComponentByClass(UCesiumFlyToComponent::StaticClass()));
	FlyComp->OnFlightComplete.AddDynamic(this, &ARaceManager::OnFlightComplete);
	ShiftComp = Cast<UCesiumOriginShiftComponent>(DynaPawn->GetComponentByClass(UCesiumOriginShiftComponent::StaticClass()));
	
	AActor* Streamactor = UGameplayStatics::GetActorOfClass(GetWorld(), AOWLViewportCapture::StaticClass());
	if (Streamactor)
		Cast<AOWLViewportCapture>(Streamactor)->PauseRendering = false;
	
	// UDS Timer
	if (UDSClass)
	{
		UDSActor = UGameplayStatics::GetActorOfClass(GetWorld(), UDSClass);
		
	} 
	
	GetWorldTimerManager().SetTimer(
		UdsHandle,
		this,
		&ARaceManager::UpdateUdsTime,
		// 36000.f,
		1.f,
		true,
		1.f
		);
	
	// Let's Go !! Gathering all the races
	ShowLoadingOverlay();
	LoadingSubsystem->RegisterTask("LoadingRaces", FText::FromString(FString(TEXT("Loading Races"))));
	// Loading default Save
	SettingsSubsystem->Load();
		
	if (SettingsSubsystem->GetMainURL().IsEmpty())
	{
		SettingsSubsystem->SetMainURL(TEXT("https://simulacre.ltvprod.cc/"));
	}
	
	FString TrailsEndpoint = SettingsSubsystem->GetMainURL() + TEXT("races");
	TrailsSubsystem->PerformHttpRequestForRaces(TrailsEndpoint);
	DynaPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (DynaPawn)
	{
		DynaPawn->SetActorLocation(FVector((1000000.f)));
	}	
	
}
void ARaceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideLoadingOverlay();
	EmptySnapshotSince.Empty();
	Super::EndPlay(EndPlayReason);
}
void ARaceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bOverlayHidden){
		if (LoadingSubsystem->AreAllDoneSuccessfully())
		{
			HideLoadingOverlay();
			bOverlayHidden = true;
			SetActorTickEnabled(false);
		}
	}
}

void ARaceManager::UpdateUdsTime()
{
	float Time = 0.0f;
	if (UDSActor && 
		CallBP_GetFloatOut(UDSActor, TEXT("GetTimeOfDay_cpp"), TEXT("TimeOfDay"), Time))
	{
		(Time >= 600 && Time <= 1800)  ? OnChangeDayNight.Broadcast(true) : OnChangeDayNight.Broadcast(false);
	}
}

/**
 * @brief Request all Races Entries
 */
void ARaceManager::PerformHttpRequestForRaces()
{
	LoadingSubsystem = FWorldUtils::GetGISubsystemOrLog<ULoadingStatusSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!LoadingSubsystem) return;
	
	if (AllRaces.RacesEntries.Num() == 0)
	{
		LoadingSubsystem->Fail("LoadingRaces", FText::FromString(FString::Printf(TEXT("No Races found"))));
		return;
	}
	
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!RaceSubsystem || !SettingsSubsystem) return;	
	
	// For each Race, getting the Setup
	FString RaceEndpoint = SettingsSubsystem->GetMainURL() + TEXT("race/");
	float cpt = 1;
	LoadingSubsystem->SetRunning("LoadingRaces", FText::FromString(FString::Printf(TEXT("Loading Races"))));
	
	for(const FRaceEntry& RaceEntry : AllRaces.RacesEntries)
	{
		LoadingSubsystem->Update("LoadingRaces", float(cpt / AllRaces.RacesEntries.Num()), FText::FromString(FString::Printf(TEXT("Race %lld"), RaceEntry.raceId)));
		
		FString CurrentRaceEndpoint = RaceEndpoint + LexToString(RaceEntry.raceId);
		RaceSubsystem->SetCurrentRaceId(RaceEntry.raceId);
		
		const int64 RaceId = RaceEntry.raceId;
		const FName IdRunners(*FString::Printf(TEXT("SpawnRunners_%lld"), RaceId));
		const FName IdPois(*FString::Printf(TEXT("SpawnPois_%lld"), RaceId));
		const FName IdCheckpoints(*FString::Printf(TEXT("SpawnCheckpoints_%lld"), RaceId));
		const FName IdSetup(*FString::Printf(TEXT("GatherSetup_%lld"), RaceId));
		const FName IdPath(*FString::Printf(TEXT("SpawnPath_%lld"), RaceId));

		LoadingSubsystem->RegisterTask(IdPath, FText::FromString(FString::Printf(TEXT("Path for Race %lld"), RaceId)));
		LoadingSubsystem->RegisterTask(IdRunners, FText::FromString(FString::Printf(TEXT("Runners for Race %lld"), RaceId)));
		LoadingSubsystem->RegisterTask(IdPois, FText::FromString(FString::Printf(TEXT("Pois for Race %lld"), RaceId)));
		LoadingSubsystem->RegisterTask(IdCheckpoints, FText::FromString(FString::Printf(TEXT("Checkpoints for Race %lld"), RaceId)));
		LoadingSubsystem->RegisterTask(IdSetup, FText::FromString(FString::Printf(TEXT("Setup for Race %lld"), RaceId)));

		RaceSubsystem->PerformHttpRequestForRaceSetup(RaceEntry.raceId, CurrentRaceEndpoint, AllRaces.RacesEntries.Num(), cpt);
		cpt++;
		
		// Test FSettings
		FSettings SettingTmp = SettingsSubsystem->GetTrailSettingsById(RaceEntry.raceId);
	}
	LoadingSubsystem->Complete("LoadingRaces", FText::FromString(FString::Printf(TEXT("Starting loading datas"))));
}

/**
 * @brief Called when the Race setup is gathered
 * @param RaceID
 * @param RaceSetupDatas 
 */
void ARaceManager::HandleRaceSetupDatasGathered(int64 RaceID, FRaceSetup RaceSetupDatas)
{
	const FName IdSetup(*FString::Printf(TEXT("GatherSetup_%lld"), RaceID));
	const FName IdRunners(*FString::Printf(TEXT("SpawnRunners_%lld"), RaceID));
	const FName IdPois(*FString::Printf(TEXT("SpawnPois_%lld"), RaceID));
	const FName IdCheckpoints(*FString::Printf(TEXT("SpawnCheckpoints_%lld"), RaceID));
	const FName IdPath(*FString::Printf(TEXT("SpawnPath_%lld"), RaceID));
	LoadingSubsystem->SetRunning(IdSetup, FText::FromString(FString::Printf(TEXT("Loading Setup for Race %lld"), RaceID)));
	
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	PathSubsystem = FWorldUtils::GetGISubsystemOrLog<UPathSubsystem>(this, TEXT(__FUNCTION__), true);
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	
	if (RaceSetupDatas.templateName.IsEmpty() || 
		!PathSubsystem 
		|| !SettingsSubsystem 
		|| !RaceSubsystem->IsRaceSetupValid(RaceSetupDatas))
	{
		LoadingSubsystem->Fail(IdSetup, FText::FromString(FString::Printf(TEXT("Loading Setup for Race %lld failed"), RaceID)));
		LoadingSubsystem->Fail(IdRunners, FText::FromString(FString::Printf(TEXT("Loading Runners for Race %lld failed"), RaceID)));
		LoadingSubsystem->Fail(IdPois, FText::FromString(FString::Printf(TEXT("Loading Pois for Race %lld failed"), RaceID)));
		LoadingSubsystem->Fail(IdCheckpoints, FText::FromString(FString::Printf(TEXT("Loading Checkpoints for Race %lld failed"), RaceID)));
		LoadingSubsystem->Fail(IdPath, FText::FromString(FString::Printf(TEXT("Loading PathHandlePathDatas for Race %lld failed"), RaceID)));
		return;
	}
	
	RaceSetupDatasMap.Add(RaceID, RaceSetupDatas);
	FRaceLoaded RaceLoaded;
	RaceLoaded.RaceID = RaceID;
	RaceLoaded.RaceName = RaceSetupDatas.raceName;
	RaceLoaded.RaceTemplate = RaceSetupDatas.templateName;
	RaceLoadedMap.FindOrAdd(RaceID, RaceLoaded);
	
	FString CurrentPathEndpoint = SettingsSubsystem->GetMainURL() + TEXT("race/") + LexToString(RaceID) + TEXT("/path");
	PathSubsystem->PerformHttpRequestForPath(RaceID, CurrentPathEndpoint);
}

/**
 * @brief Handle the Path datas, spawns the Path and calls DrawPath()
 * @param RaceID
 * @param RacePathDatas 
 */
void ARaceManager::HandlePathDatasGathered(int64 RaceID, FRacePath RacePathDatas)
{	ensure(IsInGameThread());
	PathSubsystem = FWorldUtils::GetGISubsystemOrLog<UPathSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!PathSubsystem) return;
	
	const FName IdSetup(*FString::Printf(TEXT("GatherSetup_%lld"), RaceID));
	const FName IdPath(*FString::Printf(TEXT("DrawingPath_%lld"), RaceID));
	LoadingSubsystem->Complete(IdSetup, FText::FromString(FString::Printf(TEXT("Loading Setup for Race %lld"), RaceID)));
	LoadingSubsystem->SetRunning(IdPath, FText::FromString(FString::Printf(TEXT("Drawing Path for Race %lld"), RaceID)));
	
	
	if (RacePathDatas.Points.Num()>0)
	{
		FTransform SpawnTransform = FTransform::Identity;
		TObjectPtr<AActor> SpawnedActor = PathSubsystem->SpawnPathActor(RaceID, GetWorld(), APath::StaticClass(), SpawnTransform);
		if (const TObjectPtr<APath> Path = Cast<APath>(SpawnedActor))
		{
			PathsDatas.Add(RaceID, RacePathDatas);
			RacePaths.Add(RaceID, Path);
			Path->OnPathEndDrawing.AddDynamic(this, &ARaceManager::GetRunnersDatas);
			Path->OnPathEndDrawing.AddDynamic(this, &ARaceManager::GetCheckpointsDatas);
			Path->OnPathEndDrawing.AddDynamic(this, &ARaceManager::GetPoisDatas);
			Path->OnGeoRefLocation.AddDynamic(this, &ARaceManager::SaveGeorefLocation);
			Path->DrawPath(RaceID, RacePathDatas);
			LoadingSubsystem->Complete(IdPath, FText::FromString(FString::Printf(TEXT("Path for Race %lld drawn"), RaceID)));
			
		} else
		{
			LoadingSubsystem->Fail(IdPath, FText::FromString(FString::Printf(TEXT("Path for Race %lld failed to spawn"), RaceID)));
		}
	} else
	{
		LoadingSubsystem->Fail(IdPath, FText::FromString(FString::Printf(TEXT("No Point in Path for Race %lld failed"), RaceID)));
	}
}

/**
 * @brief Binding proxy to gather Runners datas
 * @param RaceID 
 */
void ARaceManager::GetRunnersDatas(int64 RaceID)
{
	LoadingSubsystem = FWorldUtils::GetGISubsystemOrLog<ULoadingStatusSubsystem>(this, TEXT(__FUNCTION__), true);
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	RunnerSubsystem = FWorldUtils::GetGISubsystemOrLog<URunnerSubsystem>(this, TEXT(__FUNCTION__), true);
	
	const FName IdRunners(*FString::Printf(TEXT("SpawnRunners_%lld"), RaceID));
	LoadingSubsystem->SetRunning(IdRunners, FText::FromString("Loading ..."));
	
	RunnersEndpoint = SettingsSubsystem->GetMainURL() + TEXT("race/") + LexToString(RaceID) + TEXT("/runners");
	RunnerSubsystem->PerformHttpRequestForRunners(RaceID, RunnersEndpoint, true);
}

void ARaceManager::GetCheckpointsDatas(int64 RaceID)
{
	FString CheckpointsEndpoint = SettingsSubsystem->GetMainURL() + TEXT("race/") + LexToString(RaceID) + TEXT("/checkpoints");
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	CheckpointSubsystem->PerformHttpRequestForCheckpoints(RaceID, CheckpointsEndpoint);
}

/**
 * @brief Binding proxy to gather Pois datas
 * @param RaceID 
 */
void ARaceManager::GetPoisDatas(int64 RaceID)
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem || !PoiSubsystem) return;
	FString CurrentPoisEndpoint = SettingsSubsystem->GetMainURL() + TEXT("race/") + LexToString(RaceID) + TEXT("/pois");
	PoiSubsystem->PerformHttpRequestForPOIs(RaceID, CurrentPoisEndpoint);
}

/**
 * @brief Starting the Timer to fetch datas
 */
void ARaceManager::StartFetchTimer()
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FetchHandle,
			this,
			&ARaceManager::OnUpdateFetch,
			SettingsSubsystem->GetFetchFrequency(),
			true
		);
	}
}

/**
 * @brief Called every this->UpdateIntervalSeconds to update Runner datas
 */
void ARaceManager::OnUpdateFetch()
{	
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem || !RaceSubsystem) return;	
	
	if (RaceSubsystem->GetCurrentRaceId() != -1)
	{
		RunnersEndpoint = SettingsSubsystem->GetMainURL() + TEXT("race/") + LexToString(RaceSubsystem->GetCurrentRaceId()) + TEXT("/runners");
		RunnerSubsystem->PerformHttpRequestForRunners(RaceSubsystem->GetCurrentRaceId(), RunnersEndpoint, false);
	} 
}

void ARaceManager::HandleRunnersDatasGathered(int64 RaceID, FRunners RunnersDatas)
{
	SpawnInitialRunners(RaceID, RunnersDatas);
}

void ARaceManager::HandleUpdateRunners(int64 RaceID, FRunners RunnersDatas)
{
	UpdateRunnersFromSnapshot(RaceID, RunnersDatas);
}

TSubclassOf<AActor> ARaceManager::GetPoiClassForRace(const FRaceSetup& RaceSetup) const
{
	if (RaceSetup.templateName == TEXT("UTMB"))
	{
		return APoi_UTMB::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("GTWS"))
	{
		return APoi_GTWS::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("Nike"))
	{
		return APoi_Nike::StaticClass();
	}
	return APoi_Generic::StaticClass();
}
/**
 * @brief Called when the Pois are gathered and spawn them
 * @param RaceID
 * @param RacePoisDatas 
 */
void ARaceManager::HandleRacePoisDatasGathered(int64 RaceID, FPOIs RacePoisDatas)
{
	LoadingSubsystem = FWorldUtils::GetGISubsystemOrLog<ULoadingStatusSubsystem>(this, TEXT(__FUNCTION__), true);
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	
	
	if (!LoadingSubsystem || !PoiSubsystem || !RaceSubsystem || !ScaleSubsystem) return;

	const FName IdPois(*FString::Printf(TEXT("SpawnPois_%lld"), RaceID));

	// Une course sans POI est un cas normal : la tache se termine en succes
	if (RacePoisDatas.POIs.Num() == 0)
	{
		LoadingSubsystem->Complete(IdPois, FText::FromString(FString::Printf(TEXT("No Pois for Race %lld"), RaceID)));
	} else{
		Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
		const FVector* GeorefLocation = GeorefLocations.Find(RaceID);
		UWorld* World = GetWorld();
		if (!Georeference || !GeorefLocation || !World)
		{
			LoadingSubsystem->Fail(IdPois, FText::FromString(FString::Printf(TEXT("Georeference not available for Race %lld"), RaceID)));
		}
		else
		{
			Georeference->SetOriginLongitudeLatitudeHeight(*GeorefLocation);

			// Sampling ??
			int32 cpt = 0;
			LoadingSubsystem->SetRunning(IdPois);
			const FRaceSetup& RaceSetup = RaceSubsystem->GetRaceSetupById(RaceID);

			for (const FRacePOI& RacePoi : RacePoisDatas.POIs)
			{
				float Result = float(cpt) / float(RacePoisDatas.POIs.Num());
				LoadingSubsystem->Update(IdPois, Result, FText::FromString(FString::Printf(TEXT("Poi %s spawned"), *RacePoi.name)));
				FVector PoiLocation = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(
					FVector(RacePoi.lon, RacePoi.lat, RacePoi.elevation));

				const FVector Location = PoiLocation;
				const FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
				const FVector Scale = FVector(40.f);
				const FTransform SpawnTransform = FTransform(Rotation, Location, Scale);
				
				TObjectPtr<AActor> SpawnedActor = nullptr;
				const TSubclassOf<AActor> PoiClass = GetPoiClassForRace(RaceSetup);
				SpawnedActor = PoiSubsystem->SpawnPoi(RaceID, RacePoi.poiId, World, PoiClass, SpawnTransform);
				float Total = float(cpt) / float(RacePoisDatas.POIs.Num());
				LoadingSubsystem->Update(IdPois, Total, FText::FromString(FString::Printf(TEXT("Poi %s spawned"), *RacePoi.name)));
				
				if (IPoiInterface* PoiInterface = Cast<IPoiInterface>(SpawnedActor))
				{
					PoiInterface->UpdatePoi(RacePoi, RaceSetup);
					SpawnedActor->SetActorHiddenInGame(true);
				}
				cpt++;
			}
			LoadingSubsystem->Complete(IdPois, FText::FromString(TEXT("All Pois spawned")));
		}
	} 
}

/**
 * @brief Keeps track of the Georef locations for each Race
 * @param RaceID 
 * @param GeorefLocation 
 */
void ARaceManager::SaveGeorefLocation(int64 RaceID, FVector GeorefLocation)
{
	GeorefLocations.Add(RaceID, GeorefLocation);
}

bool ARaceManager::ResolveRunnerSubsystems(bool bNeedLoading)
{
	if (bNeedLoading)
	{
		LoadingSubsystem = FWorldUtils::GetGISubsystemOrLog<ULoadingStatusSubsystem>(this, TEXT(__FUNCTION__), false);
		if (!LoadingSubsystem)
		{
			return false;
		}
	}

	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	RunnerSubsystem = FWorldUtils::GetGISubsystemOrLog<URunnerSubsystem>(this, TEXT(__FUNCTION__), true);

	return TeamSubsystem && RunnerSubsystem;
}

void ARaceManager::PrepareRunnerRuntimeData(FRunnerStruct& Runner, const FRaceSetup& RaceSetup) const
{
	if (ScaleSubsystem)
	{
		Runner.MinMax = ScaleSubsystem->GetGlobalConfig().ScaleMinMax;
	}

	if (RaceSetup.templateName == TEXT("UTMB"))
	{
		Runner.VDelta = 230.f;
	}
	else if (RaceSetup.templateName == TEXT("GTWS"))
	{
		Runner.VDelta = 200.f;
	}
	else if (RaceSetup.templateName == TEXT("Nike"))
	{
		Runner.VDelta = 250.f;
	}
	else
	{
		Runner.VDelta = 100.f;
	}
}

TSubclassOf<AActor> ARaceManager::GetRunnerClassForRace(const FRaceSetup& RaceSetup) const
{
	if (RaceSetup.templateName == TEXT("UTMB"))
	{
		return ARunner_UTMB::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("GTWS"))
	{
		return ARunner_GTWS::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("Nike"))
	{
		return ARunner_Nike::StaticClass();
	}
	return ARunner_Generic::StaticClass();
}

/**
 * @brief Trace le nombre de teams indexees pour une race.
 * Les notifications Slate n'atterrissent pas dans le log fichier : sans cette trace
 * un index vide est indetectable a posteriori.
 */
static void LogRaceTeamIndex(const UTeamSubsystem* TeamSubsystem, int64 RaceID, const TCHAR* Context)
{
	if (!TeamSubsystem)
	{
		return;
	}

	const FRaceTeamRunnerIndex* Index = TeamSubsystem->GetTeamsForRace(RaceID);
	const int32 TeamsNum = Index ? Index->TeamToRunner.Num() : 0;

	UE_LOG(LogTemp, Log, TEXT("[%s] Race %lld : %d teams indexees"), Context, RaceID, TeamsNum);
}

void ARaceManager::SpawnInitialRunners(int64 RaceID, FRunners& RunnersDatas)
{
	if (!ResolveRunnerSubsystems(true))
	{
		return;
	}

	const FName IdRunners(*FString::Printf(TEXT("SpawnRunners_%lld"), RaceID));

	if (RunnersDatas.Runners.IsEmpty())
	{
		LoadingSubsystem->Fail(IdRunners, FText::FromString(FString::Printf(TEXT("No Runners found for Race %lld"), RaceID)));
		return;
	}

	// L'index Teams est de la donnee pure : il ne depend ni du World, ni de la Georeference,
	// ni du georef de la race. On le construit AVANT les gardes visuelles, sinon une race
	// dont le spawn echoue reste definitivement sans index et tout displayTeam la visant echoue.
	if (!RaceSubsystem)
	{
		RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
		if (!RaceSubsystem)
		{
			LoadingSubsystem->Fail(IdRunners, FText::FromString(FString::Printf(TEXT("RaceSubsystem not found for Race %lld"), RaceID)));
			return;
		}
	}

	const FRaceSetup& RaceSetup = RaceSubsystem->GetRaceSetupById(RaceID);

	// PrepareRunnerRuntimeData deref ScaleSubsystem : sur ce chemin devenu plus precoce,
	// on tente une resolution tardive plutot que de crasher.
	if (!ScaleSubsystem)
	{
		ScaleSubsystem = FWorldUtils::GetWorldSubsystemOrLog<UScaleSubsystem>(this, TEXT(__FUNCTION__), true);
	}

	// Prepare le snapshot AVANT rebuild de l'index
	for (FRunnerStruct& Runner : RunnersDatas.Runners)
	{
		PrepareRunnerRuntimeData(Runner, RaceSetup);
	}

	// Source de verite reconstruite sur le snapshot courant
	TeamSubsystem->RebuildRaceTeamIndex(RaceID, RunnersDatas);
	LogRaceTeamIndex(TeamSubsystem, RaceID, TEXT("SpawnInitialRunners"));

	// A partir d'ici : partie visuelle uniquement. Un echec n'invalide plus l'index Teams.
	UWorld* World = GetWorld();
	if (!World)
	{
		LoadingSubsystem->Fail(IdRunners, FText::FromString(TEXT("World is null")));
		return;
	}

	Georeference = ACesiumGeoreference::GetDefaultGeoreference(World);
	if (!Georeference)
	{
		LoadingSubsystem->Fail(IdRunners, FText::FromString(TEXT("Georeference not found")));
		return;
	}

	const FVector* Georef = GeorefLocations.Find(RaceID);
	if (!Georef)
	{
		LoadingSubsystem->Fail(IdRunners, FText::FromString(FString::Printf(TEXT("Georef location not found for Race %lld"), RaceID)));
		return;
	}

	Georeference->SetOriginLongitudeLatitudeHeight(
		FVector(Georef->X, Georef->Y, Georef->Z));

	LoadingSubsystem->SetRunning(IdRunners, FText::FromString(TEXT("Loading runners...")));

	const TSubclassOf<AActor> RunnerClass = GetRunnerClassForRace(RaceSetup);

	int32 Count = 0;
	for (FRunnerStruct& Runner : RunnersDatas.Runners)
	{
		const FVector RunnerLocation =
			Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(
				FVector(Runner.lon, Runner.lat, Runner.elevation));

		const FTransform SpawnTransform(
			FRotator::ZeroRotator,
			RunnerLocation,
			FVector(40.f));

		TObjectPtr<AActor> SpawnedActor =
			RunnerSubsystem->SpawnRunnerActor(RaceID, Runner, World, RunnerClass, SpawnTransform);

		// SpawnRunnerActor cree toujours l'acteur cache : rien a faire ici.
		if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(SpawnedActor))
		{
			RunnerInterface->AssignRunnerToTeam(Runner.canalId);
			RunnerInterface->UpdateRunner(Runner, RaceSetup);
		}

		++Count;
		const float Progress = static_cast<float>(Count) / static_cast<float>(RunnersDatas.Runners.Num());
		LoadingSubsystem->Update(
			IdRunners,
			Progress,
			FText::FromString(TEXT("Runner ") + Runner.nom + TEXT(" spawned")));
	}

	LoadingSubsystem->Complete(
		IdRunners,
		FText::FromString(FString::Printf(TEXT("Runners from %lld fetched"), RaceID)));
}

void ARaceManager::UpdateRunnersFromSnapshot(int64 RaceID, FRunners& RunnersDatas)
{
	if (!ResolveRunnerSubsystems(false))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Un backend qui redemarre repond 200 avec un tableau vide pendant plusieurs dizaines de
	// secondes : ce signal est indiscernable d'une course reellement terminee. On n'accepte donc
	// un snapshot vide comme verite qu'apres EmptySnapshotGraceSeconds de vide consecutif.
	if (RunnersDatas.Runners.IsEmpty())
	{
		const double Now = FPlatformTime::Seconds();
		double* FirstEmptyTime = EmptySnapshotSince.Find(RaceID);

		if (!FirstEmptyTime)
		{
			// Premier snapshot vide consecutif : on horodate et on sort sans rien modifier.
			EmptySnapshotSince.Add(RaceID, Now);

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[UpdateRunnersFromSnapshot] Race %lld : snapshot vide, purge differee de %.0f s (backend indisponible ?)."),
				RaceID,
				EmptySnapshotGraceSeconds);

			return;
		}

		// Marqueur : la purge a deja eu lieu lors d'un cycle precedent, on ne relogue plus.
		if (*FirstEmptyTime != EmptySnapshotPurgedMarker)
		{
			const double Elapsed = Now - *FirstEmptyTime;

			if (Elapsed < EmptySnapshotGraceSeconds)
			{
				// Indisponibilite probable : on ne touche ni a l'index Teams ni aux acteurs.
				return;
			}

			// Seuil depasse : la course est reellement terminee ou videe, on purge.
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[UpdateRunnersFromSnapshot] Race %lld : snapshot vide depuis %.0f s (seuil %.0f s), purge de l'index et des acteurs."),
				RaceID,
				Elapsed,
				EmptySnapshotGraceSeconds);

			*FirstEmptyTime = EmptySnapshotPurgedMarker;
		}
	}
	else
	{
		// Snapshot non vide : la serie de vides est rompue, le compteur repart de zero.
		EmptySnapshotSince.Remove(RaceID);
	}

	TObjectPtr<APath>* PathPtr = RacePaths.Find(RaceID);
	if (!PathPtr || !*PathPtr)
	{
		return;
	}

	CurrentRacePath = *PathPtr;

	const FRaceSetup& RaceSetup = RaceSubsystem->GetRaceSetupById(RaceID);

	for (FRunnerStruct& Runner : RunnersDatas.Runners)
	{
		PrepareRunnerRuntimeData(Runner, RaceSetup);
	}

	// Vérité métier : TeamID -> Runner courant
	TeamSubsystem->RebuildRaceTeamIndex(RaceID, RunnersDatas);
	LogRaceTeamIndex(TeamSubsystem, RaceID, TEXT("UpdateRunnersFromSnapshot"));

	// Toute team absente du snapshot quitte cette race
	RunnerSubsystem->RemoveTeamsNotInSnapshot(RaceID, RunnersDatas);

	Georeference = ACesiumGeoreference::GetDefaultGeoreference(World);
	const TSubclassOf<AActor> RunnerClass = GetRunnerClassForRace(RaceSetup);

	for (FRunnerStruct& Runner : RunnersDatas.Runners)
	{
		TObjectPtr<AActor> CurrentRunner = RunnerSubsystem->GetRunnerActorByTeam(RaceID, Runner.canalId);

		// Nouvelle team dans la race => spawn à chaud
		if (!CurrentRunner)
		{
			if (!Georeference)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[UpdateRunnersFromSnapshot] Georeference not found for Race %lld. Cannot spawn Team %lld."),
					RaceID,
					Runner.canalId);
				continue;
			}

			const FVector RunnerLocation =
				Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(
					FVector(Runner.lon, Runner.lat, Runner.elevation));

			const FTransform SpawnTransform(
				FRotator::ZeroRotator,
				RunnerLocation,
				FVector(40.f));

			CurrentRunner = RunnerSubsystem->SpawnRunnerActor(
				RaceID,
				Runner,
				World,
				RunnerClass,
				SpawnTransform);
		}

		if (CurrentRunner)
		{
			if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(CurrentRunner))
			{
				RunnerInterface->AssignRunnerToTeam(Runner.canalId);
				RunnerInterface->UpdateRunner(Runner, RaceSetup);
				RunnerInterface->UpdateRunnerLocation(Runner, CurrentRacePath);
			}
		}
	}
}

/**
 * @brief Called when all the Races are gathered
 * @param RacesArray 
 */
void ARaceManager::HandleRacesDatasGathered(FRaceEntries RacesArray)
{
	if (RacesArray.RacesEntries.IsEmpty()) return;
	AllRaces = RacesArray;
	
	PerformHttpRequestForRaces();
}

void ARaceManager::OnFlightComplete()
{	
	if (!Georeference) return;
	const int64 CurrentRaceID = RaceSubsystem->GetCurrentRaceId();
	FRacePath* Path = PathsDatas.Find(CurrentRaceID);
	FRacePathPoint& P0 = Path->Points[0];
	Georeference->SetOriginLongitudeLatitudeHeight(FVector(P0.lon, P0.lat, P0.ele));
}

/**
 * @brief Moving the Pawn to the required path - Answering to CURL Request
 * @param RaceID 
 */
void ARaceManager::MoveToPath_Internal(int64 RaceID)
{	
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!RaceSubsystem) return;
	
	const int64 PreviousRaceID = RaceSubsystem->GetCurrentRaceId();
	
	if (RaceSetupDatasMap.Find(RaceID) && PreviousRaceID != -1){
		ToggleRace_Internal(PreviousRaceID, false);
	}
	if (!RaceSetupDatasMap.Find(RaceID))
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Teleport cancelled : Race %lld does not exists"), RaceID)), EMessageType::Error);
		return;
	}
	
	RaceSubsystem->SetCurrentRaceId(RaceID);
	if (RacePaths.Contains(RaceID))
	{
		ShiftComp->SetActive(false);
		FRacePath NewPath = PathsDatas.FindRef(RaceID);
		float NewArrivalHeight = NewPath.Points[0].datas.name.IsEmpty() ? NewPath.Points[0].ele : NewPath.Points[0].datas.altitude;
		
		FlyComp->FlyToLocationLongitudeLatitudeHeight(
			FVector(NewPath.Points[0].lon, NewPath.Points[0].lat, NewArrivalHeight + 5000.f),
			0.f,
			0.f,
			false);
				
		// Test values from settings
		TestAnchor = SettingsSubsystem->GetZAnchor();
		TestPitch = SettingsSubsystem->GetCameraPitch();
		TestMinScale = SettingsSubsystem->GetMinMaxById(RaceID).Min.X;
		TestMaxScale = SettingsSubsystem->GetMinMaxById(RaceID).Max.X;
		TestNearDist = ScaleSubsystem->GetNear();
		TestFarDist = ScaleSubsystem->GetFar();
		TestZOffset = SettingsSubsystem->GetZOffsetById(RaceID);
		TestFetch = SettingsSubsystem->GetFetchById(RaceID);
		TestGlow = SettingsSubsystem->GetGlowById(RaceID);
		TestPitch = SettingsSubsystem->GetCameraPitch();
		TestAnchor = SettingsSubsystem->GetZAnchor();
		TestArmLength = SettingsSubsystem->GetArmLength();
		
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Teleport to Race %lld"), RaceID)), EMessageType::Success);
		ToggleRace_Internal(RaceID, true);
	} else
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Path %lld does not exist"), RaceID)), EMessageType::Error);
	}
}
void ARaceManager::EdMoveToPath()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	MoveToPath_Internal(TestRaceID);
}
void ARaceManager::RtMoveToPath(int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	MoveToPath_Internal(RaceID);
}

/**
 * @brief Show/Hide Km
 * @param RaceID 
 * @param bShow 
 */
void ARaceManager::ToggleKms_Internal(int64 RaceID, bool bShow) const
{
	if (const TObjectPtr<APath>* CurrentPathActor = RacePaths.Find(RaceID))
	{
		TObjectPtr<APath> CurrentPath = CurrentPathActor->Get();
		CurrentPath->ChangeKmsVisibility(bShow);
	}
}
void ARaceManager::EdToggleKms() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	ToggleKms_Internal(TestRaceID, bAfficher);
}
void ARaceManager::RtToggleKms(int64 RaceID, bool bShow) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	ToggleKms_Internal(RaceID, bShow);
}

/**
 * @brief Pulse On/Off
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::TogglePulse_Internal(int64 RaceID, bool bDisplay)
{
	if (const TObjectPtr<APath>* CurrentPathActor = RacePaths.Find(RaceID))
	{
		TObjectPtr<APath> CurrentPath = CurrentPathActor->Get();
		if (bDisplay)
			CurrentPath->StartPulse();
		else
			CurrentPath->StopPulse();
	}
}
void ARaceManager::EdTogglePulse()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	TogglePulse_Internal(TestRaceID, bAfficher);
}
void ARaceManager::RtTogglePulse(int64 RaceID, bool bDisplay)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	TogglePulse_Internal(RaceID, bDisplay);
}

/**
 * @brief Show/Hide Path
 * @param RaceID 
 * @param bShow 
 */
void ARaceManager::TogglePath_Internal(int64 RaceID, bool bShow) const
{
	if (RacePaths.Contains(RaceID))
	{
		if (const TObjectPtr<APath>* PathRef = RacePaths.Find(RaceID))
		{
			TObjectPtr<APath> Path = PathRef->Get();
			Path->ChangePathVisibility(bShow);
		}
	} else
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Path %lld not found"), RaceID)), EMessageType::Error);
	}
}
void ARaceManager::EdTogglePath() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	TogglePath_Internal(TestRaceID, bAfficher);
}
void ARaceManager::RtTogglePath(int64 RaceID, bool bShow) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	TogglePath_Internal(RaceID, bShow);
}

/**
 * @brief Show/Hide Slope
 * @param RaceID 
 * @param bShow 
 */
void ARaceManager::ToggleSlope_Internal(int64 RaceID, bool bShow) const
{
	if (RacePaths.Contains(RaceID))
	{
		if (const TObjectPtr<APath>* PathRef = RacePaths.Find(RaceID))
		{
			TObjectPtr<APath> Path = PathRef->Get();
			Path->ChangeSlopeVisibility(bShow);
		}
	} else
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Path %lld does not exist"), RaceID)), EMessageType::Error);
	}
}
void ARaceManager::EdToggleSlope() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	ToggleSlope_Internal(TestRaceID, bAfficher);
}
void ARaceManager::RtToggleSlope(int64 RaceID, bool bShow) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	};
	ToggleSlope_Internal(RaceID, bShow);
}

/**
 * TEAMS
 */

/**
 * @brief Hiding Runners without testing Teams
 * @param RaceID 
 */
void ARaceManager::ToggleRunners_Internal(int64 RaceID, bool bShow) const
{
	RunnerSubsystem = FWorldUtils::GetGISubsystemOrLog<URunnerSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!RunnerSubsystem) return;
	
	if (RacePaths.Contains(RaceID))
	{
		RunnerSubsystem->ToggleRunners(RaceID, bShow);
	} else
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Path %lld does not exist"), RaceID)), EMessageType::Error);
	}
}

/**
 * @brief Moving the Pawn to the Team location
 * @param TeamID 
 * @param RaceID 
 */
void ARaceManager::TeleportToTeam_Internal(int64 TeamID, int64 RaceID) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->Teleport(TeamID, RaceID);
}
void ARaceManager::EdTeleportToTeam() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TeleportToTeam_Internal(TestTeamID, TestRaceID);
}
void ARaceManager::RtTeleportToTeam(int64 TeamID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TeleportToTeam_Internal(TeamID, RaceID);
}

/**
 * @brief Toggle Team flag display
 * @param TeamID 
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::ToggleFlag_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->DisplayFlag(TeamID, RaceID, bDisplay);
}
void ARaceManager::EdToggleFlag() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleFlag_Internal(TestTeamID, TestRaceID, bAfficher);
}
void ARaceManager::RtToggleFlag(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleFlag_Internal(TeamID, RaceID, bDisplay);
}

/**
 * @brief Toggle Team photo display
 * @param TeamID *
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::TogglePhoto_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->DisplayPhoto(TeamID, RaceID, bDisplay);
}
void ARaceManager::EdTogglePhoto() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePhoto_Internal(TestTeamID, TestRaceID, bAfficher);
}
void ARaceManager::RtTogglePhoto(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePhoto_Internal(TeamID, RaceID, bDisplay);
}

/**
 * @brief Toggle Team club display
 * @param TeamID 
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::ToggleClub_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->DisplayClub(TeamID, RaceID, bDisplay);
}
void ARaceManager::EdToggleClub() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleClub_Internal(TestTeamID, TestRaceID, bAfficher);
}
void ARaceManager::RtToggleClub(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleClub_Internal(TeamID, RaceID, bDisplay);
}

/**
 * @brief Show/Hide Team
 * @param TeamID 
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::ToggleTeam_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->DisplayTeam(TeamID, RaceID, bDisplay);
}
void ARaceManager::EdToggleTeam() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleTeam_Internal(TestTeamID, TestRaceID, bAfficher);
}
void ARaceManager::RtToggleTeam(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleTeam_Internal(TeamID, RaceID, bDisplay);
}

/**
 * @brief Toggle Teams by RaceID
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::ToggleTeams_Internal(int64 RaceID, bool bDisplay) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->DisplayTeams(RaceID, bDisplay);
}
void ARaceManager::EdToggleTeams() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleTeams_Internal(TestRaceID, bAfficher);
}
void ARaceManager::RtToggleTeams(int64 RaceID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleTeams_Internal(RaceID, bDisplay);
}

/**
 * @brief Playing the orbit animation around a Team
 * @param TeamID 
 * @param RaceID 
 */
void ARaceManager::AnimTeam_Internal(int64 TeamID, int64 RaceID) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->Animation(TeamID, RaceID);
}
void ARaceManager::EdAnimTeam() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AnimTeam_Internal(TestTeamID, TestRaceID);
}
void ARaceManager::RtAnimTeam(int64 TeamID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AnimTeam_Internal(TeamID, RaceID);
}

/**
 * @brief Ending the Team animation
 * @param TeamID 
 * @param RaceID 
 */
void ARaceManager::StopAnimTeam_Internal(int64 TeamID, int64 RaceID) const
{
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamSubsystem) return;
	TeamSubsystem->StopAnimation(TeamID, RaceID);
}
void ARaceManager::EdStopAnimTeam() const
{
	StopAnimTeam_Internal(TestTeamID, TestRaceID);
}
void ARaceManager::RtStopAnimTeam(int64 TeamID, int64 RaceID) const
{
	StopAnimTeam_Internal(TeamID, RaceID);
}

/**
 * GROUPS
 */

/**
 * @brief Adding a Team to a Group
 * @param RaceID
 * @param GroupID
 * @param TeamID
 */
void ARaceManager::AddTeamToGroup_Internal(int64 RaceID, int64 GroupID, int64 TeamID) const
{
	TeamGroupSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamGroupSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamGroupSubsystem) return;
	TeamGroupSubsystem->AddTeamToGroup(RaceID, GroupID, TeamID);
}
void ARaceManager::EdAddTeamToGroup() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AddTeamToGroup_Internal(TestRaceID, TestGroupID, TestTeamID);
}
void ARaceManager::RtAddTeamToGroup(int64 RaceID, int64 GroupID, int64 TeamID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AddTeamToGroup_Internal(RaceID, GroupID, TeamID);
}

/**
 * @brief Removing a Team from a Group
 * @param RaceID 
 * @param GroupID 
 * @param TeamID 
 */
void ARaceManager::RemoveTeamFromGroup_Internal(int64 RaceID, int64 GroupID, int64 TeamID) const
{
	TeamGroupSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamGroupSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamGroupSubsystem) return;
	TeamGroupSubsystem->RemoveTeamFromGroup(RaceID, GroupID, TeamID);
}
void ARaceManager::EdRemoveTeamFromGroup() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AddTeamToGroup_Internal(TestRaceID, TestGroupID, TestTeamID);
}
void ARaceManager::RtRemoveTeamFromGroup(int64 RaceID, int64 GroupID, int64 TeamID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AddTeamToGroup_Internal(RaceID, GroupID, TeamID);
}

/**
 * @brief Show/Hide	a whole Group
 * @param RaceID 
 * @param GroupID 
 * @param bDisplay 
 */
void ARaceManager::ToggleGroup_Internal(int64 RaceID, int64 GroupID, bool bDisplay) const
{
	TeamGroupSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamGroupSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!TeamGroupSubsystem) return;
	TeamGroupSubsystem->ToggleGroup(RaceID, GroupID, bDisplay);
}
void ARaceManager::EdToggleGroup() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleGroup_Internal(TestRaceID, TestGroupID, bAfficher);
}
void ARaceManager::RtToggleGroup(int64 RaceID, int64 GroupID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleGroup_Internal(RaceID, GroupID, bDisplay);
}

/**
 * CHECKPOINTS
 */

/**
 * @brief Show/Hide all Checkpoints
 * @param RaceID 
 * @param bShow 
 */
void ARaceManager::ToggleCheckpoints_Internal(int64 RaceID, bool bShow) const
{
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!CheckpointSubsystem) return;
	if (RacePaths.Contains(RaceID))
	{
		CheckpointSubsystem->ToggleCheckpoints(RaceID, bShow);
	}
}
void ARaceManager::EdToggleCheckpoints() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleCheckpoints_Internal(TestRaceID, bAfficher);
}
void ARaceManager::RtToggleCheckpoints(int64 RaceID, bool bShow) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleCheckpoints_Internal(RaceID, bShow);
}

/**
 * @brief Show/Hide Checkpoint
 * @param CheckpointID
 * @param RaceID
 * @param bDisplay
 */
void ARaceManager::ToggleCheckpoint_Internal(int64 CheckpointID, int64 RaceID, bool bDisplay) const
{
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!CheckpointSubsystem) return;
	CheckpointSubsystem->ToggleCheckpoint(CheckpointID, RaceID, bDisplay);
}
void ARaceManager::EdToggleCheckpoint() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleCheckpoint_Internal(TestCheckpointID, TestRaceID, bAfficher);
}
void ARaceManager::RtToggleCheckpoint(int64 CheckpointID, int64 RaceID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleCheckpoint_Internal(CheckpointID, RaceID, bDisplay);
}

/**
 * @brief Show/Hide Weather
 * @param CheckpointID 
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::ToggleCheckpointWeather_Internal(int64 CheckpointID, int64 RaceID, bool bDisplay) const
{
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!CheckpointSubsystem) return;
	CheckpointSubsystem->ToggleWeather(RaceID, CheckpointID, bDisplay);
}
void ARaceManager::EdToggleCheckpointWeather()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleCheckpointWeather_Internal(TestCheckpointID, TestRaceID, bAfficher);
}
void ARaceManager::RtToggleCheckpointWeather(int64 CheckpointID, int64 RaceID, bool bShow)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleCheckpointWeather_Internal(CheckpointID, RaceID, bShow);
}

/**
 * @brief Teleport to Checkpoint
 * @param CheckpointID 
 * @param RaceID 
 */
void ARaceManager::TeleportToCheckpoint_Internal(int64 CheckpointID, int64 RaceID) const
{
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!CheckpointSubsystem) return;
	CheckpointSubsystem->Teleport(CheckpointID, RaceID);	
}
void ARaceManager::EdTeleportToCheckpoint() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TeleportToCheckpoint_Internal(TestCheckpointID, TestRaceID);
}
void ARaceManager::RtTeleportToCheckpoint(int64 CheckpointID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TeleportToCheckpoint_Internal(CheckpointID, RaceID);
}

/**
 * @brief Play/Stop Checkpoint animation
 * @param CheckpointID 
 * @param RaceID 
 * @note WIP
 */
void ARaceManager::AnimCheckpoint_Internal(int64 CheckpointID, int64 RaceID) const
{
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!CheckpointSubsystem) return;
	CheckpointSubsystem->Animation(CheckpointID, RaceID);
}
void ARaceManager::EdAnimCheckpoint() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AnimCheckpoint_Internal(TestCheckpointID, TestRaceID);
}
void ARaceManager::RtAnimCheckpoint(int64 CheckpointID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AnimCheckpoint_Internal(CheckpointID, RaceID);
}
void ARaceManager::StopAnimCheckpoint_Internal(int64 CheckpointID, int64 RaceID) const
{
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!CheckpointSubsystem) return;
	CheckpointSubsystem->StopAnimation(CheckpointID, RaceID);
}
void ARaceManager::EdStopAnimCheckpoint() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StopAnimCheckpoint_Internal(TestCheckpointID, TestRaceID);
}
void ARaceManager::RtStopAnimCheckpoint(int64 CheckpointID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StopAnimCheckpoint_Internal(CheckpointID, RaceID);
}

/**
 * POI
 */

/**
 * @brief Show/Hide Poi
 * @param PoiID 
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::TogglePoi_Internal(int64 PoiID, int64 RaceID, bool bDisplay) const
{
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!PoiSubsystem) return;
	PoiSubsystem->TogglePoi(RaceID, PoiID, bDisplay);
}
void ARaceManager::EdTogglePoi() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePoi_Internal(TestPoiID, TestRaceID, bAfficher);
}
void ARaceManager::RtTogglePoi(int64 PoiID, int64 RaceID, bool bDisplay) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePoi_Internal(PoiID, RaceID, bDisplay);
}

/**
 * @brief Show/Hide all Pois
 * @param RaceID
 * @param bShow
 */
void ARaceManager::TogglePois_Internal(int64 RaceID, bool bShow) const
{
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!PoiSubsystem) return;
	if (RacePaths.Contains(RaceID))
	{
		PoiSubsystem->TogglePois(RaceID, bShow);
	}
}
void ARaceManager::EdTogglePois() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePois_Internal(TestRaceID, bAfficher);
}
void ARaceManager::RtTogglePois(int64 RaceID, bool bShow) const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePois_Internal(RaceID, bShow);
}

/**
 * @brief Teleport to Poi
 * @param PoiID 
 * @param RaceID 
 */
void ARaceManager::TeleportToPoi_Internal(int64 PoiID, int64 RaceID) const
{
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!PoiSubsystem) return;
	PoiSubsystem->Teleport(PoiID, RaceID);	
}
void ARaceManager::EDTeleportToPoi() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TeleportToPoi_Internal(TestPoiID, TestRaceID);	
}
void ARaceManager::RtTeleportToPoi(int64 PoiID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TeleportToPoi_Internal(PoiID, RaceID);	
}

/**
 * @brief Show/Hide Weather
 * @param PoiID 
 * @param RaceID 
 * @param bDisplay 
 */
void ARaceManager::TogglePoiWeather_Internal(int64 PoiID, int64 RaceID, bool bDisplay) const
{
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!PoiSubsystem) return;
	PoiSubsystem->ToggleWeather(RaceID, PoiID, bDisplay);
}
void ARaceManager::EdTogglePoiWeather()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePoiWeather_Internal(TestPoiID, TestRaceID, bAfficher);
}
void ARaceManager::RtTogglePoiWeather(int64 PoiID, int64 RaceID, bool bDisplay)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	TogglePoiWeather_Internal(PoiID, RaceID, bDisplay);
}

/**
 * @brief Start/Stop Poi Animation
 * @param PoiID 
 * @param RaceID 
 * @note WIP
 */
void ARaceManager::AnimPoi_Internal(int64 PoiID, int64 RaceID) const
{
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!PoiSubsystem) return;
	PoiSubsystem->Animation(PoiID, RaceID);
}
void ARaceManager::EdAnimPoi() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AnimPoi_Internal(TestPoiID, TestRaceID);
}
void ARaceManager::RtAnimPoi(int64 PoiID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	AnimPoi_Internal(PoiID, RaceID);
}
void ARaceManager::StopAnimPoi_Internal(int64 PoiID, int64 RaceID) const
{
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!PoiSubsystem) return;
	PoiSubsystem->StopAnimation(PoiID, RaceID);
}
void ARaceManager::EdStopAnimPoi() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StopAnimPoi_Internal(TestPoiID, TestRaceID);
}
void ARaceManager::RtStopAnimPoi(int64 PoiID, int64 RaceID) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StopAnimPoi_Internal(PoiID, RaceID);
}

/**
 * SETTINGS
 */

/**
 * @brief Resetting Saved values to default
 */
void ARaceManager::EdResetSavedDatas()
{
	if (!RefreshSubsystemsFromPIE()) return;
	for (TPair<long long, FRaceSetup> SetupDatasMap : RaceSetupDatasMap)
	{
		FSettings NewSettings = FSettings(
			SetupDatasMap.Value.raceId,
			TEXT("https://simulacre.ltvprod.cc/regie/1/"),
			1.0f,
			10.f,
			10.f,
			10.f,
			0,
			FMinMax(FVector(30.0f), FVector(200.f)),
			-25.f,
			50000.f,
			-500.f);
		SettingsSubsystem->CreateTrailSettingsById(SetupDatasMap.Value.raceId, NewSettings);
	}
	SettingsSubsystem->Save();
}

/**
 * @brief Updating the fetch frequency
 * @param FetchValue float
 * @param RaceID int64
 */
void ARaceManager::UpdateFetchFrequency_Internal(float FetchValue, int64 RaceID) const
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	SettingsSubsystem->SetFetchById(FetchValue, RaceID);
}
void ARaceManager::RtUpdateFetchFrequency(float FetchValue, int64 RaceID)
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateFetchFrequency_Internal(FetchValue, RaceID);
	
}
void ARaceManager::EdUpdateFetchFrequency()
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateFetchFrequency_Internal(TestFetch, TestRaceID);
}

/**
 * @brief Changing the ZOffset of Path
 * @param OffsetValue 
 * @param RaceID 
 */
void ARaceManager::ChangePathOffset_Internal(float OffsetValue, int64 RaceID)
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	TeamSubsystem = FWorldUtils::GetGISubsystemOrLog<UTeamSubsystem>(this, TEXT(__FUNCTION__), true);
	RunnerSubsystem = FWorldUtils::GetGISubsystemOrLog<URunnerSubsystem>(this, TEXT(__FUNCTION__), true);
	
	if (!SettingsSubsystem || !TeamSubsystem || !RunnerSubsystem) return;
	
	float OldZOffset = SettingsSubsystem->GetZOffsetById(RaceID);
	float NewZOffset = OldZOffset + OffsetValue;
	SettingsSubsystem->SetZOffsetById(NewZOffset, RaceID);
	
	TObjectPtr<APath>* PathPtr = RacePaths.Find(RaceID);
	if (!PathPtr)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Path not found in Race %lld"), RaceID)), EMessageType::Error);
	}
	
	PathPtr->Get()->AddActorWorldOffset(FVector(0.f, 0.f, OffsetValue));
	
	// Runners
	FRunners RunnersTmp = RunnerSubsystem->GetRunnersByRaceId(RaceID);
	if (RunnersTmp.Runners.IsEmpty()) return;
	for (FRunnerStruct Runner : RunnersTmp.Runners)
	{
		// TObjectPtr<AActor> RunnerActor = RunnerSubsystem->GetRunnerActor(RaceID, Runner.runnerId);
		TObjectPtr<AActor> RunnerActor = RunnerSubsystem->GetRunnerActorByTeam(RaceID, Runner.canalId);
		if (!RunnerActor) return;
		FVector RunnerLocation = RunnerActor->GetActorLocation();
		RunnerActor->SetActorLocation(FVector(
			RunnerLocation.X, RunnerLocation.Y, RunnerLocation.Z + OffsetValue));
	}
}
void ARaceManager::RtChangePathOffset(float OffsetValue, int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ChangePathOffset_Internal(OffsetValue, RaceID);
}
void ARaceManager::EdChangePathOffset()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ChangePathOffset_Internal(TestZOffset, TestRaceID);
}

/**
 * @brief Updating Glow intensity
 * @param GlowValue 
 * @param RaceID 
 */
void ARaceManager::UpdateGlowIntensity_Internal(float GlowValue, int64 RaceID) const
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	SettingsSubsystem->SetGlowById(GlowValue, RaceID);
	
	TObjectPtr<AActor> PathPtr = PathSubsystem->GetPathById(RaceID);
	if (!PathPtr) return;
	TObjectPtr<APath> PathActor = Cast<APath>(PathPtr);
	if (!PathActor) return;
	PathActor->UpdatePathGlow(GlowValue, RaceID);
}
void ARaceManager::RtUpdateGlowIntensity(float Glowvalue, int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateGlowIntensity_Internal(Glowvalue, RaceID);
}
void ARaceManager::EdUpdateGlowIntensity()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateGlowIntensity_Internal(TestGlow, TestRaceID);
}

/**
 * @brief Updating the Pulse speed
 * @param PulseSpeed 
 * @param RaceID 
 */
void ARaceManager::UpdatePulseFrequency_Internal(float PulseSpeed, int64 RaceID) const
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	SettingsSubsystem->SetPulseFrequencyById(PulseSpeed, RaceID);
	TObjectPtr<AActor> PathPtr = PathSubsystem->GetPathById(RaceID);
	if (!PathPtr) return;
	TObjectPtr<APath> PathActor = Cast<APath>(PathPtr);
	if (!PathActor) return;
	PathActor->UpdatePulseSpeed(PulseSpeed);
}
void ARaceManager::RtUpdatePulseFrequency(float PulseSpeed, int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdatePulseFrequency_Internal(PulseSpeed, RaceID);
}
void ARaceManager::EdUpdatePulseFrequency()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdatePulseFrequency_Internal(TestPulse, TestRaceID);
}

/**
 * @brief Updating the Pulse speed
 * @param PulseSpeed 
 * @param RaceID 
 */
void ARaceManager::UpdatePulseGlow_Internal(float PulseSpeed, int64 RaceID) const
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	SettingsSubsystem->SetPulseGlowById(PulseSpeed, RaceID);
	TObjectPtr<AActor> PathPtr = PathSubsystem->GetPathById(RaceID);
	if (!PathPtr) return;
	TObjectPtr<APath> PathActor = Cast<APath>(PathPtr);
	if (!PathActor) return;
	PathActor->UpdatePulseGlow(PulseSpeed);
}
void ARaceManager::RtUpdatePulseGlow(float PulseSpeed, int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdatePulseGlow_Internal(PulseSpeed, RaceID);
}
void ARaceManager::EdUpdatePulseGlow()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdatePulseGlow_Internal(TestPulse, TestRaceID);
}

/**
 * @brief Updating the Min & Max size of a Team
 * @param MinValue 
 * @param MaxValue 
 * @param RaceID 
 */
void ARaceManager::UpdateMinMax_Internal(float MinValue, float MaxValue, int64 RaceID) 
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	SettingsSubsystem->SetMinMaxById(MinValue, MaxValue, RaceID);
	ApplyMinMax(MinValue, MaxValue);
}
void ARaceManager::RtUpdateMinMax(float MinValue, float MaxValue, int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateMinMax_Internal(MinValue, MaxValue, RaceID);
}
void ARaceManager::EdUpdateMinMax()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateMinMax_Internal(TestMinScale, TestMaxScale, TestRaceID);
}
void ARaceManager::ApplyMinMax(float MinValue, float MaxValue)
{
	ScaleSubsystem->SetGlobalMinMax(MinValue, MaxValue);
}

/**
 * @brief Updating the Near & Far distance for scale
 * @param NearValue 
 * @param FarValue 
 */
void ARaceManager::UpdateNearFar_Internal(float NearValue, float FarValue) const
{
	ScaleSubsystem = FWorldUtils::GetWorldSubsystemOrLog<UScaleSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	ScaleSubsystem->SetNearFar(NearValue, FarValue);
	SettingsSubsystem->Save();
}
void ARaceManager::RtUpdateNearFar(float NearValue, float FarValue)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateNearFar_Internal(NearValue, FarValue);
}
void ARaceManager::EdUpdateNearFar()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateNearFar_Internal(TestNearDist, TestFarDist);
}

/**
 * @brief Set de l'url principale
 * @param url 
 */
void ARaceManager::UpdateMainUrl_Internal(FString url)
{
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem) return;
	SettingsSubsystem->SetMainURL(url);
	SettingsSubsystem->Save();
	PerformHttpRequestForRaces();
}
void ARaceManager::EdUpdateMainUrl()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateMainUrl_Internal(TestUrl);
}
void ARaceManager::RtUpdateMainUrl(FString url)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateMainUrl_Internal(url);
}

/**
 * @brief Show/Hide Race with all children
 * @param RaceID 
 * @param bShow 
 */
void ARaceManager::ToggleRace_Internal(int64 RaceID, bool bShow) const
{
	int64 OldRaceID = -1;
	LoadingSubsystem = FWorldUtils::GetGISubsystemOrLog<ULoadingStatusSubsystem>(this, TEXT(__FUNCTION__), true);
	PoiSubsystem = FWorldUtils::GetGISubsystemOrLog<UPoiSubsystem>(this, TEXT(__FUNCTION__), true);
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!SettingsSubsystem || !RaceSubsystem || !PoiSubsystem) return;
	if (RacePaths.Contains(RaceID))
	{
		TogglePath_Internal(RaceID, bShow);
		ToggleCheckpoints_Internal(RaceID, bShow);
		if (RaceID != OldRaceID)
		{
			ToggleRunners_Internal(RaceID, false);
			TogglePois_Internal(RaceID, false);
			ToggleSlope_Internal(RaceID, false);
			// OldRaceID = RaceID;
		}
	}
}
void ARaceManager::RtToggleRace(int64 RaceID, bool bShow) const
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleRace_Internal(RaceID, bShow);
}
void ARaceManager::EdToggleRace() const
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	ToggleRace_Internal(TestRaceID, bAfficher);
}

/**
 * @brief Refetch race datas
 * @param RaceID 
 */
void ARaceManager::RefreshRace_Internal(int64 RaceID)
{
	GetRunnersDatas(RaceID);
}
void ARaceManager::EdRefreshRace()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	RefreshRace_Internal(TestRaceID);
}
void ARaceManager::RtRefreshRace(int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	RefreshRace_Internal(RaceID);
}

/**
 * @brief Starting/Stoping the Travel
 * @param RaceID 
 */
void ARaceManager::StartTravelFwd_Internal(int64 RaceID)
{
	if (RacePaths.Contains(RaceID))
	{
		TObjectPtr<APath>* CurrentPath = RacePaths.Find(RaceID);
		if (!CurrentPath)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No path found for race %lld"), RaceID)), EMessageType::Error);
			return;
		}
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			// OriginalViewTarget = PC->GetViewTarget();
			PC->SetViewTargetWithBlend(*CurrentPath, 1.f);
			CurrentPath->Get()->StartTravelForward();
		}
	}
}
void ARaceManager::EdStartTravelFwd()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StartTravelFwd_Internal(TestRaceID);
}
void ARaceManager::RtStartTravelFwd(int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StartTravelFwd_Internal(RaceID);
}
void ARaceManager::StartTravelBkd_Internal(int64 RaceID)
{
	if (RacePaths.Contains(RaceID))
	{
		TObjectPtr<APath>* CurrentPath = RacePaths.Find(RaceID);
		if (!CurrentPath)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No path found for race %lld"), RaceID)), EMessageType::Error);
			return;
		}
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			// OriginalViewTarget = PC->GetViewTarget();
			PC->SetViewTargetWithBlend(*CurrentPath, 1.f);
			CurrentPath->Get()->StartTravelBackward();
		}
	}
}
void ARaceManager::EdStartTravelBkd()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StartTravelBkd_Internal(TestRaceID);
}
void ARaceManager::RtStartTravelBkd(int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StartTravelBkd_Internal(RaceID);
}
void ARaceManager::StopTravel_Internal(int64 RaceID)
{
	if (RacePaths.Contains(RaceID))
	{
		TObjectPtr<APath>* CurrentPath = RacePaths.Find(RaceID);
		if (!CurrentPath)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No path found for race %lld"), RaceID)), EMessageType::Error);
			return;
		}
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			CurrentPath->Get()->StopTravel();
			PC->SetViewTargetWithBlend(OriginalViewTarget, 1.f);
		}
	}
}
void ARaceManager::EdStopTravel()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StopTravel_Internal(TestRaceID);
}
void ARaceManager::RtStopTravel(int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	StopTravel_Internal(RaceID);
}
void ARaceManager::PauseTravel_Internal(int64 RaceID)
{
	if (RacePaths.Contains(RaceID))
	{
		TObjectPtr<APath>* CurrentPath = RacePaths.Find(RaceID);
		if (!CurrentPath)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No path found for race %lld"), RaceID)), EMessageType::Error);
			return;
		}
		CurrentPath->Get()->SetActorTickEnabled(false);
	}
}
void ARaceManager::EdPauseTravel()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	PauseTravel_Internal(TestRaceID);
}
void ARaceManager::RtPauseTravel(int64 RaceID)
{
	if (!RefreshSubsystems())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	PauseTravel_Internal(RaceID);
}

/**
 * LOADING SCREEN
 */
void ARaceManager::ShowLoadingOverlay()
{
	if (!GEngine || !GEngine->GameViewport) return;

	LoadingSubsystem = FWorldUtils::GetGISubsystemOrLog<ULoadingStatusSubsystem>(this, TEXT(__FUNCTION__), true);
	if (!LoadingSubsystem) return;

	LoadingOverlay = SNew(SLoadingOverlay)
		.Loading(LoadingSubsystem);

	ViewportContainer = SNew(SWeakWidget).PossiblyNullContent(LoadingOverlay.ToSharedRef());

	GEngine->GameViewport->AddViewportWidgetContent(ViewportContainer.ToSharedRef(), 9999);
}
void ARaceManager::HideLoadingOverlay()
{
	StartFetchTimer();
	if (!GEngine || !GEngine->GameViewport) return;
	
	if (ViewportContainer.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ViewportContainer.ToSharedRef());
		ViewportContainer.Reset();
	}
	LoadingOverlay.Reset();
}

/**
 * @brief Change Camera Pitch
 * @param NewPitch 
 */
void ARaceManager::UpdateCameraAngle_Internal(float NewPitch)
{
	SettingsSubsystem->SetCameraPitch(NewPitch);
}
void ARaceManager::RtUpdateCameraAngle(float NewPitch)
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateCameraAngle_Internal(NewPitch);
}
void ARaceManager::EdUpdateCameraAngle()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateCameraAngle_Internal(TestPitch);
}

/**
 * @brief Change Camera Anchor
 * @param NewAnchor 
 */
void ARaceManager::UpdateCameraAnchor_Internal(float NewAnchor)
{
	SettingsSubsystem->SetZAnchor(NewAnchor);
}
void ARaceManager::RtUpdateCameraAnchor(float NewAnchor)
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateCameraAnchor_Internal(NewAnchor);
}
void ARaceManager::EdUpdateCameraAnchor()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateCameraAnchor_Internal(TestAnchor);
}

/**
 * @brief Change SpringArm length
 * @param NewLength
 */
void ARaceManager::UpdateUpdateArmLength_Internal(float NewLength)
{
	SettingsSubsystem->SetArmLength(NewLength);
}

void ARaceManager::RtUpdateUpdateArmLength(float NewLength)
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateUpdateArmLength_Internal(NewLength);
}

void ARaceManager::EdUpdateUpdateArmLength()
{
	if (!RefreshSubsystemsFromPIE())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Subsystems not available"))), EMessageType::Error);
		return;
	}
	UpdateUpdateArmLength_Internal(TestLength);
}