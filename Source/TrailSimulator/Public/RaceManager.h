// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Cesium3DTileset.h"
#include "GameFramework/Actor.h"
#include "TrailSharedTypes.h"
#include "RaceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeDayNight, bool, bIsDay);

UCLASS()
class TRAILSIMULATOR_API ARaceManager : public AActor
{
	GENERATED_BODY()

public:
	ARaceManager();
	virtual void Tick(float DeltaTime) override;
	
	DECLARE_MULTICAST_DELEGATE(FOnChangeDestination);
	FOnChangeDestination OnChangeDestination;
	
	FOnChangeDayNight OnChangeDayNight;
	
	UFUNCTION()
	bool RefreshSubsystems() const;
	UFUNCTION()
	bool RefreshSubsystemsFromPIE() const;
		
	UPROPERTY()
	TObjectPtr<class APawn> DynaPawn;
	UPROPERTY()
	TObjectPtr<class UCesiumFlyToComponent> FlyComp;
	UPROPERTY()
	TObjectPtr<class UCesiumOriginShiftComponent> ShiftComp;
	UFUNCTION()
	void OnFlightComplete();
	
	/** Subsystems */
	UPROPERTY(Transient)
	mutable TObjectPtr<class UPathSubsystem> PathSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class URunnerSubsystem> RunnerSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UTeamSubsystem> TeamSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UScaleSubsystem> ScaleSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UPoiSubsystem> PoiSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UCheckpointSubsystem> CheckpointSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UTrailsSubsystem> TrailsSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class USettingsSubsystem> SettingsSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UWeatherSubsystem> WeatherSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class ULoadingStatusSubsystem> LoadingSubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UHttpGatewaySubsystem> HttpGatewaySubsystem;
	UPROPERTY(Transient)
	mutable TObjectPtr<class UTeamGroupSubsystem> TeamGroupSubsystem;
	
	// Helpers
	bool ResolveRunnerSubsystems(bool bNeedLoading);
	void PrepareRunnerRuntimeData(FRunnerStruct& Runner, const FRaceSetup& RaceSetup) const;
	TSubclassOf<AActor> GetRunnerClassForRace(const FRaceSetup& RaceSetup) const;
	void SpawnInitialRunners(int64 RaceID, FRunners& RunnersDatas);
	void UpdateRunnersFromSnapshot(int64 RaceID, FRunners& RunnersDatas);
	
	/** Request Callbacks */
	// All Races
	UFUNCTION()
	void HandleRacesDatasGathered(FRaceEntries Races);
	// Path
	UFUNCTION()
	void HandlePathDatasGathered(int64 RaceID, FRacePath RacePathDatas);
	// Race setup
	UFUNCTION()
	void HandleRaceSetupDatasGathered(int64 RaceID, FRaceSetup RaceSetupDatas);
	// Pois
	UFUNCTION()
	void HandleRacePoisDatasGathered(int64 RaceID, FPOIs RacePoisDatas);
	UFUNCTION()
	void GetPoisDatas(int64 RaceID);
	// Runners
	UFUNCTION()
	void HandleRunnersDatasGathered(int64 RaceID, FRunners RaceRunnersDatas);
	UFUNCTION()
	void HandleUpdateRunners(int64 RaceID, FRunners RunnersDatas);
	UFUNCTION()
	TSubclassOf<AActor> GetPoiClassForRace(const FRaceSetup& RaceSetup) const;
	UFUNCTION()
	void GetRunnersDatas(int64 RaceID);
	UFUNCTION()
	void GetCheckpointsDatas(int64 RaceID);
	// Georef
	UFUNCTION()
	void SaveGeorefLocation(int64 RaceID, FVector GeorefLocation);	
	// Move to Path
	UFUNCTION()
	void MoveToPath_Internal(int64 RaceID);
	UFUNCTION()
	void RtMoveToPath(int64 RaceID);
	UFUNCTION(CallInEditor, Category="Race")
	void EdMoveToPath();
	
	/*** INTERNAL FUNCTIONS */
	// Race
	UFUNCTION()
	void TogglePath_Internal(int64 RaceID, bool bShow) const;
	UFUNCTION()
	void RtTogglePath(int64 RaceID, bool bShow) const;
	UFUNCTION(CallInEditor, Category="Race")
	void EdTogglePath() const;
	
	UFUNCTION()
	void RefreshRace_Internal(int64 RaceID);
	UFUNCTION(CallInEditor, Category="Race")
	void EdRefreshRace();
	UFUNCTION()
	void RtRefreshRace(int64 RaceID);
	
	UFUNCTION()
	void ToggleSlope_Internal(int64 RaceID, bool bShow) const;
	UFUNCTION()
	void RtToggleSlope(int64 RaceID, bool bShow) const;
	UFUNCTION(CallInEditor, Category="Race")
	void EdToggleSlope() const;
	
	UFUNCTION()
	void ToggleKms_Internal(int64 RaceID, bool bShow) const;
	UFUNCTION()
	void RtToggleKms(int64 RaceID, bool bShow) const;
	UFUNCTION(CallInEditor, Category="Race")
	void EdToggleKms() const;
	
	UFUNCTION()
	void TogglePulse_Internal(int64 RaceID, bool bDisplay);
	UFUNCTION(CallInEditor, Category="Race")
	void EdTogglePulse();
	UFUNCTION()
	void RtTogglePulse(int64 RaceID, bool bDisplay);
	
	// Teams
	UFUNCTION()
	void ToggleRace_Internal(int64 RaceID, bool bShow) const;
	UFUNCTION()
	void RtToggleRace(int64 RaceID, bool bShow) const;
	UFUNCTION(CallInEditor, Category="Race")
	void EdToggleRace() const;
	

	UFUNCTION()
	void ToggleRunners_Internal(int64 RaceID, bool bShow) const;
	
	UFUNCTION()
	void TeleportToTeam_Internal(int64 TeamID, int64 RaceID) const;
	UFUNCTION()
	void RtTeleportToTeam(int64 TeamID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdTeleportToTeam() const;
	
	UFUNCTION()
	void ToggleFlag_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtToggleFlag(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdToggleFlag() const;
	
	UFUNCTION()
	void TogglePhoto_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtTogglePhoto(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdTogglePhoto() const;
	
	UFUNCTION()
	void ToggleClub_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtToggleClub(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdToggleClub() const;
	
	UFUNCTION()
	void ToggleTeam_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtToggleTeam(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdToggleTeam() const;
	
	UFUNCTION()
	void ToggleTeams_Internal(int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtToggleTeams(int64 RaceID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdToggleTeams() const;
	
	UFUNCTION() 
	void AnimTeam_Internal(int64 TeamID, int64 RaceID) const;
	UFUNCTION()
	void RtAnimTeam(int64 TeamID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdAnimTeam() const;
	
	UFUNCTION() 
	void StopAnimTeam_Internal(int64 TeamID, int64 RaceID) const;
	UFUNCTION()
	void RtStopAnimTeam(int64 TeamID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Teams")
	void EdStopAnimTeam() const;
	
	// Checkpoints
	UFUNCTION()
	void ToggleCheckpoints_Internal(int64 RaceID, bool bShow) const;
	UFUNCTION()
	void RtToggleCheckpoints(int64 RaceID, bool bShow) const;
	UFUNCTION(CallInEditor, Category="Checkpoints")
	void EdToggleCheckpoints() const;
	
	UFUNCTION() 
	void ToggleCheckpoint_Internal(int64 CheckpointID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtToggleCheckpoint(int64 CheckpointID, int64 RaceID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Checkpoints")
	void EdToggleCheckpoint() const;
	
	UFUNCTION() 
	void ToggleCheckpointWeather_Internal(int64 TeamID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtToggleCheckpointWeather(int64 CheckpointID, int64 RaceID, bool bShow);
	UFUNCTION(CallInEditor, Category="Checkpoints")
	void EdToggleCheckpointWeather();
	
	UFUNCTION() 
	void TeleportToCheckpoint_Internal(int64 CheckpointID, int64 RaceID) const;
	UFUNCTION()
	void RtTeleportToCheckpoint(int64 CheckpointID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Checkpoints")
	void EdTeleportToCheckpoint() const;
	
	UFUNCTION() 
	void AnimCheckpoint_Internal(int64 CheckpointID, int64 RaceID) const;
	UFUNCTION()
	void RtAnimCheckpoint(int64 CheckpointID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Checkpoints")
	void EdAnimCheckpoint() const;
	
	UFUNCTION() 
	void StopAnimCheckpoint_Internal(int64 CheckpointID, int64 RaceID) const;
	UFUNCTION()
	void RtStopAnimCheckpoint(int64 CheckpointID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Checkpoints")
	void EdStopAnimCheckpoint() const;
	
	// Groups
	UFUNCTION() 
	void AddTeamToGroup_Internal(int64 RaceID, int64 GroupID, int64 TeamID) const;
	UFUNCTION()
	void RtAddTeamToGroup(int64 RaceID, int64 GroupID, int64 TeamID) const;
	UFUNCTION(CallInEditor, Category="Groups")
	void EdAddTeamToGroup() const;
	
	UFUNCTION()
	void RemoveTeamFromGroup_Internal(int64 RaceID, int64 GroupID, int64 TeamID) const;
	UFUNCTION()
	void RtRemoveTeamFromGroup(int64 RaceID, int64 GroupID, int64 TeamID) const;
	UFUNCTION(CallInEditor, Category="Groups")
	void EdRemoveTeamFromGroup() const;
	
	UFUNCTION()
	void ToggleGroup_Internal(int64 RaceID, int64 GroupID, bool bDisplay) const;
	UFUNCTION()
	void RtToggleGroup(int64 RaceID, int64 GroupID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Groups")
	void EdToggleGroup() const;
	
	// Pois
	UFUNCTION()
	void TogglePoi_Internal(int64 PoiID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtTogglePoi(int64 PoiID, int64 RaceID, bool bDisplay) const;
	UFUNCTION(CallInEditor, Category="Pois")
	void EdTogglePoi() const;
	
	UFUNCTION()
	void TogglePois_Internal(int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtTogglePois(int64 RaceID, bool bShow) const;
	UFUNCTION(CallInEditor, Category="Pois")
	void EdTogglePois() const;
	
	UFUNCTION()
	void TogglePoiWeather_Internal(int64 PoiID, int64 RaceID, bool bDisplay) const;
	UFUNCTION()
	void RtTogglePoiWeather(int64 PoiID, int64 RaceID, bool bShow);
	UFUNCTION(CallInEditor, Category="Pois")
	void EdTogglePoiWeather();
	
	UFUNCTION()
	void TeleportToPoi_Internal(int64 PoiID, int64 RaceID) const;
	UFUNCTION()
	void RtTeleportToPoi(int64 PoiID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Pois")
	void EDTeleportToPoi() const;
	
	UFUNCTION()
	void AnimPoi_Internal(int64 PoiID, int64 RaceID) const;
	UFUNCTION()
	void RtAnimPoi(int64 PoiID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Pois")
	void EdAnimPoi() const;
	
	UFUNCTION()
	void StopAnimPoi_Internal(int64 PoiID, int64 RaceID) const;
	UFUNCTION()
	void RtStopAnimPoi(int64 PoiID, int64 RaceID) const;
	UFUNCTION(CallInEditor, Category="Pois")
	void EdStopAnimPoi() const;
	
	// Settings
	UFUNCTION(CallInEditor, Category="Reset")
	void EdResetSavedDatas();
	UFUNCTION()
	void UpdateFetchFrequency_Internal(float FetchValue, int64 RaceID) const;
	UFUNCTION()
	void RtUpdateFetchFrequency(float FetchValue, int64 RaceID);
	UFUNCTION(CallInEditor, Category="Settings")
	void EdUpdateFetchFrequency();
	
	UFUNCTION()
	void ChangePathOffset_Internal(float OffsetValue, int64 RaceID);
	UFUNCTION()
	void RtChangePathOffset(float OffsetValue, int64 RaceID);
	UFUNCTION(CallInEditor, Category="Settings")
	void EdChangePathOffset();
	
	UFUNCTION()
	void UpdateGlowIntensity_Internal(float GlowValue, int64 RaceID) const;
	UFUNCTION()
	void RtUpdateGlowIntensity(float Glowvalue, int64 RaceID);
	UFUNCTION(CallInEditor, Category="Settings")
	void EdUpdateGlowIntensity();
	
	UFUNCTION()
	void UpdatePulseFrequency_Internal(float PulseSpeed, int64 RaceID) const;
	UFUNCTION()
	void RtUpdatePulseFrequency(float Glowvalue, int64 RaceID);
	UFUNCTION(CallInEditor, Category="Settings")
	void EdUpdatePulseFrequency();
	void UpdatePulseGlow_Internal(float PulseSpeed, int64 RaceID) const;
	void RtUpdatePulseGlow(float PulseSpeed, int64 RaceID);
	void EdUpdatePulseGlow();

	UFUNCTION()
	void UpdateMinMax_Internal(float MinValue, float MaxValue, int64 RaceID);
	UFUNCTION()
	void RtUpdateMinMax(float MinValue, float MaxValue, int64 RaceID);
	UFUNCTION(CallInEditor, Category="Settings")
	void EdUpdateMinMax();
	UFUNCTION()
	void ApplyMinMax(float MinValue, float MaxValue);
	UFUNCTION()
	void UpdateNearFar_Internal(float NearValue, float FarValue) const;
	UFUNCTION()
	void RtUpdateNearFar(float NearValue, float FarValue);
	UFUNCTION(CallInEditor, Category="Settings")
	void EdUpdateNearFar();

	UFUNCTION()
	void UpdateMainUrl_Internal(FString url);
	UFUNCTION(CallInEditor, Category="Settings")
	void EdUpdateMainUrl();
	UFUNCTION()
	void RtUpdateMainUrl(FString NewUrl);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Settings")
	float TestPulse = 1.f;
	
	// Travel
	UFUNCTION()
	void StartTravelFwd_Internal(int64 RaceID);
	UFUNCTION(CallInEditor, Category="Travel")
	void EdStartTravelFwd();
	UFUNCTION()
	void RtStartTravelFwd(int64 RaceID);
	UFUNCTION()
	void StartTravelBkd_Internal(int64 RaceID);
	UFUNCTION(CallInEditor, Category="Travel")
	void EdStartTravelBkd();
	UFUNCTION()
	void RtStartTravelBkd(int64 RaceID);
	UFUNCTION()
	void StopTravel_Internal(int64 RaceID);
	UFUNCTION(CallInEditor, Category="Travel")
	void EdStopTravel();
	UFUNCTION()
	void RtStopTravel(int64 RaceID);
	UFUNCTION()
	void PauseTravel_Internal(int64 RaceID);
	UFUNCTION(CallInEditor, Category="Travel")
	void EdPauseTravel();
	UFUNCTION()
	void RtPauseTravel(int64 RaceID);
	
	// Camera pitch
	UPROPERTY()
	float TestLength = 0.f;
	UFUNCTION()
	void UpdateCameraAngle_Internal(float NewPitch);
	UFUNCTION()
	void RtUpdateCameraAngle(float NewPitch);
	UFUNCTION(CallInEditor, Category="Camera")
	void EdUpdateCameraAngle();
	// Anchor
	UFUNCTION()
	void UpdateCameraAnchor_Internal(float NewAnchor);
	UFUNCTION()
	void RtUpdateCameraAnchor(float NewAnchor);
	UFUNCTION(CallInEditor, Category="Camera")
	void EdUpdateCameraAnchor();
	// Camera Arm length
	UFUNCTION()
	void UpdateUpdateArmLength_Internal(float NewLength);
	UFUNCTION()
	void RtUpdateUpdateArmLength(float NewLength);
	UFUNCTION(CallInEditor, Category="Camera")
	void EdUpdateUpdateArmLength();

	// PROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="AGeneral")
	bool bAfficher = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AGeneral|Resume")
	TMap<int64, FRaceLoaded> RaceLoadedMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Race")
	int64 TestRaceID = 16;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Race")
	float TestMinScale = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Race")
	float TestMaxScale = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Race")
	float TestNearDist = 50000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Race")
	float TestFarDist = 1000000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Settings")
	float TestZOffset = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Settings")
	float TestFetch = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Settings")
	float TestGlow = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Settings")
	FString TestUrl = FString("https://simulacre.ltvprod.cc/regie/2/");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float TestPitch = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float TestAnchor = -500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float TestArmLength = 50000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Teams")
	int64 TestTeamID = 1506535286;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Checkpoints")
	int64 TestCheckpointID = 4086211504;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Pois")
	int64 TestPoiID = 792472354;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Groups")
	int64 TestGroupID;
	
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	UPROPERTY()
	AActor* OriginalViewTarget = nullptr;
	
	UPROPERTY(meta=(allowPrivateAccess=true))
	TSoftObjectPtr<class ACesiumGeoreference> Georeference;
	UPROPERTY()
	TMap<int64, FVector> GeorefLocations;
	UPROPERTY()
	FString MainURL;
	UPROPERTY()
	FRaceStruct CurrentRaceStruct;	
	UPROPERTY()
	FRaceEntries AllRaces;
	UFUNCTION()
	void PerformHttpRequestForRaces();
	UPROPERTY()
	FString RunnersEndpoint;	
	UPROPERTY()
	TMap<int64, FRacePath> PathsDatas;
	UPROPERTY()
	TMap<int64, TObjectPtr<class APath>> RacePaths;
	UPROPERTY()
	TObjectPtr<APath> CurrentRacePath;
	UPROPERTY()
	TMap<int64, FRaceSetup> RaceSetupDatasMap;
		
	// Snapshots vides : purge differee
	// Horodatage (FPlatformTime::Seconds) du premier snapshot vide consecutif, par RaceID.
	// Un backend qui redemarre renvoie des listes vides pendant quelques dizaines de secondes :
	// on ne purge l'index Teams et les acteurs que si le vide persiste au-dela du seuil.
	TMap<int64, double> EmptySnapshotSince;

	// Duree de tolerance avant purge (en secondes). Volontairement exprime en temps et non en
	// nombre de cycles : la frequence de fetch est reglable a l'execution.
	static constexpr double EmptySnapshotGraceSeconds = 60.0;

	// Valeur sentinelle stockee dans EmptySnapshotSince une fois la purge effectuee,
	// pour ne loguer qu'au changement d'etat.
	static constexpr double EmptySnapshotPurgedMarker = -1.0;

	// Timer fetch
	UPROPERTY()
	FTimerHandle FetchHandle;
	UFUNCTION()
	void OnUpdateFetch();
	UPROPERTY()
	float UpdateIntervalSeconds;
	
	// Sortie OWL : destination SRT configuree et demarree au BeginPlay
	UPROPERTY(EditAnywhere, Category="Broadcast")
	bool bStartSRTOnBeginPlay = true;
	UPROPERTY(EditAnywhere, Category="Broadcast", meta=(EditCondition="bStartSRTOnBeginPlay"))
	FString SRTStreamURL = TEXT("srt://192.168.88.129:7029");
	// Audio coupe par defaut : chaque crash Cesium (corruption du tas) suivait de moins d'une
	// seconde l'init du resampler audio OWL 7.1 -> stereo. -SRTAudio le reactive.
	UPROPERTY(EditAnywhere, Category="Broadcast")
	bool bSRTEncodeAudio = false;
	void StartSRTOutput();

	// UDS
	UPROPERTY()
	TSubclassOf<AActor> UDSClass;
	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> UDSActor;
	UPROPERTY()
	FTimerHandle UdsHandle;
	UFUNCTION()
	void UpdateUdsTime();
	void RecenterSkyAtmosphere();
	UPROPERTY()
	bool bIsDay = false;
		
	// Loading Screen
	TSharedPtr<class SWidget> ViewportContainer;
	TSharedPtr<class SLoadingOverlay> LoadingOverlay;
	UFUNCTION()
	void ShowLoadingOverlay();
	UFUNCTION()
	void HideLoadingOverlay();
	
	UFUNCTION()
	void StartFetchTimer();
	
	UPROPERTY()
	bool bOverlayHidden = false;
};
