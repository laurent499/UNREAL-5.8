// HttpGatewaySubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpRouteHandle.h"
#include "HttpServerRequest.h"
#include "HttpResultCallback.h"
#include "HttpGatewaySubsystem.generated.h"

// Race Commands
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMoveToRace, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplayRace, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplayPath, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplaySlope, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplayPulse, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplayKms, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectRaceTeams, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeselectRaceTeams, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRefreshRace, int64 RaceID);

// Teams Commands
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayTeam, int64 TeamID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplayTeams, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayFlag, int64 TeamID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayClub, int64 TeamID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayPhoto, int64 TeamID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeleportToTeam, int64 TeamID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFollowTeam, int64 TeamID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStopFollowTeam, int64 TeamID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeamAnimation, int64 TeamID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeamStopAnimation, int64 TeamID, int64 RaceID);

// Teamsgroup Commands
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAddTeamToGroup, int64 TeamID, int64 RaceID, int64 GroupID);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnRemoveTeamFromGroup, int64 RaceID, int64 TeamID, int64 GroupID);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnToggleGroup, int64 RaceID, int64 GroupID, bool bShow);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeleteRaceTeam, int64 RaceID);

// Travel
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlyForward, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlyStop, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlyPause, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlyBackward, int64 RaceID);

// Camera
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCameraAngleUpdated, float NewPitch);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnArmLengthUpdated, float NewLength);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnZAnchorUpdated, float NewZAnchor);

// Checkpoints Commands
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayCheckpoint, int64 CheckpointID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplayCheckpoints, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayCheckpointWeather, int64 CheckpointID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCheckpointTeleport, int64 CheckpointID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCheckpointAnimation, int64 CheckpointID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCheckpointStopAnimation, int64 CheckpointID, int64 RaceID);

// Pois Commands
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayPoi, int64 PoiID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisplayPois, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDisplayPoiWeather, int64 PoiID, int64 RaceID, bool bShow);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPoiTeleport, int64 PoiID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPoiAnimation, int64 PoiID, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPoiStopAnimation, int64 PoiID, int64 RaceID);

// Settings Commands
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateFetch, float FetchValue, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateOffset, float OffsetValue, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateGlow, float GlowValue, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdatePulse, float PulseSpeed, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdatePulseGlow, float Pulseglow, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnMinMaxUpdate, float MinValue, float MaxValue, int64 RaceID);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNearFarUpdate, float NearValue, float FarValue);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMainUrlUpdate, FString NewUrl);
DECLARE_MULTICAST_DELEGATE(FOnSaveMainUrl);

/**
 * @brief HTTP Server for Web commands
 * @note Manage the incoming commands from the web interface
 */
UCLASS()
class SUBSYSTEMS_API UHttpGatewaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/*** RACE COMMANDS ***/
	FOnMoveToRace OnMoveToRace;
	FOnDisplayRace OnDisplayRace;
	FOnDisplayPath OnDisplayPath;
	FOnDisplaySlope OnDisplaySlope;
	FOnDisplayPulse OnDisplayPulse;
	FOnDisplayKms OnDisplayKms;
	FOnSelectRaceTeams OnSelectRaceTeams;
	FOnDeselectRaceTeams OnDeselectRaceTeams;
	FOnRefreshRace OnRefreshRace;
	FOnDeleteRaceTeam OnDeleteRaceTeam;
	FOnFlyForward OnFlyForward;
	FOnFlyStop OnFlyStop;
	FOnFlyPause OnFlyPause;
	FOnFlyBackward OnFlyBackward;
	
	/*** TEAMS COMMANDS ***/
	FOnDisplayTeam OnDisplayTeam;
	FOnDisplayFlag OnDisplayFlag;
	FOnDisplayClub OnDisplayClub;
	FOnDisplayPhoto OnDisplayPhoto;
	FOnTeleportToTeam OnTeleportToTeam;
	FOnFollowTeam OnFollowTeam; 
	FOnStopFollowTeam OnStopFollowTeam; 
	FOnDisplayTeams OnDisplayTeams;
	FOnTeamAnimation OnTeamAnimation;
	FOnTeamStopAnimation OnTeamStopAnimation;
	
	/*** TEAMGROUPS COMMANDS ***/
	FOnAddTeamToGroup OnAddTeamToGroup;
	FOnRemoveTeamFromGroup OnRemoveTeamFromGroup;
	FOnToggleGroup OnToggleGroup;
	
	/*** CHECKPOINTS COMMANDS ***/
	FOnDisplayCheckpoint OnDisplayCheckpoint;
	FOnDisplayCheckpoints OnDisplayCheckpoints;
	FOnDisplayCheckpointWeather OnDisplayCheckpointWeather;
	FOnCheckpointTeleport OnCheckpointTeleport;
	FOnCheckpointAnimation OnCheckpointAnimation;
	FOnCheckpointStopAnimation OnCheckpointStopAnimation;
	
	/*** POIS COMMANDS ***/
	FOnDisplayPoi OnDisplayPoi;
	FOnDisplayPois OnDisplayPois;
	FOnDisplayPoiWeather OnDisplayPoiWeather;
	FOnPoiTeleport OnPoiTeleport;
	FOnPoiAnimation OnPoiAnimation;
	FOnPoiStopAnimation OnPoiStopAnimation;
	
	/*** CAMERA COMMANDS ***/
	FOnCameraAngleUpdated OnCameraAngleUpdated;
	FOnArmLengthUpdated OnArmLengthUpdated;
	FOnZAnchorUpdated OnZAnchorUpdated;
	
	/*** SETTINGS COMMANDS ***/
	FOnUpdateFetch OnUpdateFetch;
	FOnUpdateOffset OnUpdateOffset;
	FOnUpdateGlow OnUpdateGlow;
	FOnUpdatePulse OnUpdatePulse;
	FOnUpdatePulseGlow OnUpdatePulseGlow;
	FOnMinMaxUpdate OnMinMaxUpdate;
	FOnNearFarUpdate OnNearFarUpdate;
	FOnMainUrlUpdate OnMainUrlUpdate;
	FOnSaveMainUrl OnSaveMainUrl;

private:
	/** Handler pour les endpoints */
	FHttpRouteHandle CommandRouteHandle;
	bool HandleCommandRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const;
	
	// Router
	TSharedPtr<class IHttpRouter> HttpRouter;
	
	// Listen Port
	int32 ListenPort = 8200;
	
	
};
