# Graph Report - UNREAL 5.8  (2026-10-27)

## Corpus Check
- 544 files · ~1,781,952 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5286 file(s) not represented in the graph (top: .uasset 4702, .tex 369, .eps 71)

## Summary
- 4872 nodes · 7103 edges · 553 communities (172 shown, 381 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 572 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `ded490a3`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- ARaceManager
- UHttpGatewaySubsystem
- .SlateNotify
- ARunner
- RaceManager.cpp
- APath
- ACheckpoint
- APoi
- UWorldAmbienceSubsystem
- USettingsSubsystem
- WorldAmbienceSubsystem.cpp
- UCheckpointSubsystem
- modulemanager
- URunnerSubsystem
- FRunnerStruct
- TemplateSubsystem.h
- Runner.cpp
- UNameComponent
- SearchBox
- FlagComponent.h
- AWorldFauna
- ULoadingStatusSubsystem
- jquery.js
- URaceSubsystem
- UPoiSubsystem
- AWorldFlora
- Checkpoint.cpp
- APath::DrawPath
- UWaterMaskSubsystem
- UAudioSanitizerSubsystem
- Path.cpp
- Poi.cpp
- CinematicTrail.cpp
- splinemeshcomponent
- UPathSubsystem
- SettingsSubsystem.cpp
- Templates Unreal (TrailSimulator) servis par OnGraphics
- SLoadingOverlay
- URunnerStackingSubsystem
- UTeamSubsystem
- UWeatherSubsystem
- AKm
- ICameraControlInterface
- unrealbuildtool
- ACinematicGeoAnchor
- UBroadcastCaptureSubsystem
- FAutoWeatherInfo
- UClubComponent
- FEntry
- RunnerSubsystem.cpp
- UTrailTextWidgetComponent
- FOpenWeatherCurrent
- UScaleSubsystem
- FWorldAmbienceSettings
- FWeatherSample
- PoiSubsystem.cpp
- ScaleSubsystem.cpp
- ARunner_UTMB
- AudioSanitizer.cpp
- FRaceCheckpoint
- PathSubsystem.cpp
- TrailTextWidgetComponent.cpp
- WorldFlora.cpp
- SLoadingOverlay.cpp
- ACinematicTrail
- IRunnerInterface
- ARunner_GTWS
- Templates Unreal (TrailSimulator) servis par OnGraphics
- BroadcastCaptureSubsystem.cpp
- FTrailTemplate
- FLoadingTaskInfo
- WorldFauna.cpp
- FHttpResultCallback
- FBird
- ACheckpoint_Generic
- ACheckpoint_Nike
- initResizable
- PhotoComponent.cpp
- RunnerStackingSubsystem.cpp
- APoi_GTWS
- AWorldHeadlamps
- APoi_Nike
- TSubclassOf
- UPathSegmentsComponent
- Ee
- e
- NameComponent.cpp
- ARaceManager::SpawnInitialRunners
- GTWS_NameComponent.cpp
- ACinematicTrailCamera
- APoi_Generic
- SBroadcastMirror
- TrailSharedTypes.h
- CheckpointInterface.h
- UCameraControlComponent
- UMeteoPageSubsystem
- UTeamGroupSubsystem
- Km.cpp
- ACheckpoint_GTWS
- ACheckpoint_UTMB
- UTemplateSubsystem
- CinematicGeoAnchor.cpp
- WaterMaskSubsystem.cpp
- WeatherSubsystem.cpp
- .GetGameInstance
- FWorldAmbienceState
- UWorld
- ARunner_Generic
- APoi_UTMB
- CameraControlComponent.cpp
- FVector
- TeamGroupSubsystem.cpp
- showTip
- TrailHttpDebug.h
- FTrailHttpDebug::LogAndIsSuccess
- UTrailsSubsystem
- .GetWorld
- JsonResponse
- USettingsSaveGame
- FSettings
- TemplateSubsystem.cpp
- FBenchShot
- FSubmixEffectSanitizer
- FDistanceScaleConfig
- FStackEntry
- HttpGatewaySubsystem.h
- CallBP_GetFloatOut
- CheckpointSubsystem.cpp
- UTemplateGeneratorUtilityWidget
- FPointIndexSegment
- FMinMax
- FRunnerStackingConfig
- Runner_GTWS.cpp
- Runner.h
- FRaceCard
- UGTWS_NameComponent
- UFlagComponent
- FTrailTemplateRunner
- FString
- FTrailLabelStyle
- NameComponent.h
- MeteoPageSubsystem.cpp
- FRaceSetup
- PathSegmentsComponent.cpp
- WorldFlora.h
- ARunner_Nike
- AudioSanitizer.h
- BroadcastSettings.h
- FPathSegment
- RunnerStackingSubsystem.h
- DisplayController
- ClubComponent.cpp
- UGeneric_NameComponent
- BuildMesh
- splinecomponent
- FGroupStruct
- FSlateNotificationsModule
- PoiInterface.h
- UUTMB_NameComponent
- PlacementCalculator
- KmInterface.h
- FRunnerState
- ARaceManager::HideLoadingOverlay
- PathInterface.h
- FRunners
- Piecewise
- FlagComponent.cpp
- coreminimal
- UWorldAmbienceSubsystem::OnWorldBeginPlay
- URunnerStackingSubsystem::RegisterRunner
- FPendingCell
- EKind
- ACheckpoint::GetBoundsLocation
- FTrailTemplateStack
- SlateNotificationsBFL.h
- dynsections.js
- UWorldAmbienceSubsystem::Initialize
- AWorldFlora::ContinueBuild
- AWorldFlora::Tick
- EBenchPhase
- IRunnerStackableInterface
- ACinematicTrail::BeginPlay
- makeTree
- ARaceManager::BeginPlay
- MakeStep
- CLAUDE.md
- ARaceManager::ToggleRace_Internal
- FState
- FTrackSample
- cookie.js
- .mcp.json
- UBroadcastCaptureSubsystem::RequestViewTarget
- URunnerSubsystem::Deinitialize
- USettingsSubsystem::Initialize
- USettingsSubsystem::Save
- Automation_TrailSimulator.slnx
- TrailSimulator.slnx
- RawTextMaterial.h
- FOpenWeatherResponse
- int64
- GTWS_FlagComponent.h
- Generic_FlagComponent.h
- SharedTypes.h
- ARaceManager::RtTeleportToTeam
- FWaterMaskBounds
- ARaceManager::RtTogglePhoto
- ARaceManager::RtUpdateMinMax
- ARaceManager::EndPlay
- WorldAmbienceSubsystem.h
- FWeatherResult
- ACinematicTrail::SpawnRaceActors
- TrailInterfaces.h
- TrailUtils.h
- Nike_FlagComponent.h
- BroadcastCaptureSubsystem.h
- UPathSegmentsComponent::Apply
- AWorldFlora::BeginPlay
- UScaleSubsystem::Tick

## God Nodes (most connected - your core abstractions)
1. `ARaceManager` - 276 edges
2. `ARunner` - 132 edges
3. `UWorldAmbienceSubsystem` - 126 edges
4. `UHttpGatewaySubsystem` - 118 edges
5. `ACheckpoint` - 111 edges
6. `APath` - 107 edges
7. `APoi` - 107 edges
8. `FRaceSetup` - 74 edges
9. `USettingsSubsystem` - 71 edges
10. `FWorldUtils` - 61 edges

## Surprising Connections (you probably didn't know these)
- `Ce que fait le code` --references--> `ACinematicTrail`  [INFERRED]
  Docs/Cinematique/RENDU_CINEMATIQUE.md → Source/TrailSimulator/Public/Actors/Cinematic/CinematicTrail.h
- `Ce que fait le code` --references--> `ACinematicTrailCamera`  [INFERRED]
  Docs/Cinematique/RENDU_CINEMATIQUE.md → Source/TrailSimulator/Public/Actors/Cinematic/CinematicTrailCamera.h
- `UTemplateGeneratorUtilityWidget` --references--> `TSubclassOf`  [EXTRACTED]
  Plugins/TemplateGenerator/Source/TemplateGenerator/Public/TemplateGeneratorUtilityWidget.h → Source/TrailUtils/Public/WorldUtils.h
- `ACheckpoint_Nike::RefreshCheckpointValues()` --references--> `FRaceCheckpoint`  [EXTRACTED]
  Source/TrailSimulator/Private/Actors/Nike/Checkpoint_Nike.cpp → Source/SharedTypes/Public/TrailSharedTypes.h
- `UClubComponent::UpdateClub()` --references--> `FRunnerStruct`  [EXTRACTED]
  Source/TrailSimulator/Private/Components/ClubComponent.cpp → Source/SharedTypes/Public/TrailSharedTypes.h

## Import Cycles
- None detected.

## Communities (553 total, 381 thin omitted)

### Community 0 - "ARaceManager"
Cohesion: 0.01
Nodes (70): ARaceManager, AllRaces, bAfficher, bIsDay, bOverlayHidden, bSRTEncodeAudio, bStartSRTOnBeginPlay, CheckpointSubsystem (+62 more)

### Community 1 - "UHttpGatewaySubsystem"
Cohesion: 0.02
Nodes (57): UHttpGatewaySubsystem, CommandRouteHandle, HttpRouter, ListenPort, OnAddTeamToGroup, OnArmLengthUpdated, OnCameraAngleUpdated, OnCheckpointAnimation (+49 more)

### Community 2 - ".SlateNotify"
Cohesion: 0.04
Nodes (72): ARaceManager::EdAddTeamToGroup(), ARaceManager::EdAnimCheckpoint(), ARaceManager::EdAnimPoi(), ARaceManager::EdAnimTeam(), ARaceManager::EdChangePathOffset(), ARaceManager::EdMoveToPath(), ARaceManager::EdPauseTravel(), ARaceManager::EdRefreshRace() (+64 more)

### Community 3 - "ARunner"
Cohesion: 0.02
Nodes (59): ARunner, ActiveState, ArmLength, bHasDayNight, bHasStackChildren, bHasTrackDist, bLastIsDay, bLookAtEnabled (+51 more)

### Community 4 - "RaceManager.cpp"
Cohesion: 0.05
Nodes (63): ARaceManager::AddTeamToGroup_Internal(), ARaceManager::AnimCheckpoint_Internal(), ARaceManager::AnimPoi_Internal(), ARaceManager::AnimTeam_Internal(), ARaceManager::ApplyPathSegments(), ARaceManager::ChangePathOffset_Internal(), ARaceManager::EdStopAnimTeam(), ARaceManager::EdToggleRace() (+55 more)

### Community 5 - "APath"
Cohesion: 0.02
Nodes (58): APath, bForward, bPathShown, BroadCastSubsystem, CameraComponent, CheckpointSubsystem, DistanceCm, DynaPawn (+50 more)

### Community 6 - "ACheckpoint"
Cohesion: 0.02
Nodes (57): ACheckpoint, AltPicto, AltText, ArmLength, bHasDayNight, bLastIsDay, bLookAtEnabled, bOrbitEnabled (+49 more)

### Community 7 - "APoi"
Cohesion: 0.02
Nodes (55): APoi, AltPicto, AltText, ArmLength, bHasDayNight, bLastIsDay, bLookAtEnabled, bOrbitEnabled (+47 more)

### Community 8 - "UWorldAmbienceSubsystem"
Cohesion: 0.02
Nodes (57): UWorldAmbienceSubsystem::TickBench(), UWorldAmbienceSubsystem::UpdateState(), UWorldAmbienceSubsystem, AppliedWeather, AutoWeather, bBegunPlay, bBenchGrounded, BenchCsvPath (+49 more)

### Community 9 - "USettingsSubsystem"
Cohesion: 0.03
Nodes (24): USettingsSubsystem, CachedSave, LookAtInterpSpeed, LookAtIntervalSeconds, OnFetchUpdate, OnLengthChanged, OnMinMaxDirty, OnMinMaxSet (+16 more)

### Community 10 - "WorldAmbienceSubsystem.cpp"
Cohesion: 0.05
Nodes (20): Average(), ExportProperty(), ImportProperty(), JsonResponse(), Percentile(), SettingsFilePath(), UWorldAmbienceSubsystem::ApplySetting(), UWorldAmbienceSubsystem::BenchJson() (+12 more)

### Community 11 - "UCheckpointSubsystem"
Cohesion: 0.06
Nodes (9): UCheckpointSubsystem, ActiveCheckpointRequests, Checkpoint, CheckpointActorMap, Checkpoints, CheckpointsMap, CurrentRaceSetup, OnCheckpointsDatasGathered (+1 more)

### Community 13 - "URunnerSubsystem"
Cohesion: 0.04
Nodes (16): URunnerSubsystem, ActiveRunnerRequests, CurrentRaceSetup, CurrentRunnerId, MaxRunnersFetchAttempts, OnRunnerDatasGathered, OnRunnersDatasGathered, OnRunnersUpdateDatas (+8 more)

### Community 14 - "FRunnerStruct"
Cohesion: 0.10
Nodes (21): FRunnerStruct, canal, canalId, club, dossard, elevation, IndexM, kmActuel (+13 more)

### Community 15 - "TemplateSubsystem.h"
Cohesion: 0.06
Nodes (14): ACheckpoint_Generic::UpdateCheckpoint(), UseRoundedBkg_CheckpointGeneric(), APoi_Generic::UpdatePoi(), UseRoundedBkg_PoiGeneric(), ACheckpoint_GTWS::BeginPlay(), ACheckpoint_GTWS::UpdateCheckpoint(), UseRoundedBkg_CheckpointGTWS(), APoi_GTWS::BeginPlay() (+6 more)

### Community 16 - "Runner.cpp"
Cohesion: 0.05
Nodes (8): ARunner::AssignRunnerToTeam(), ARunner::GetStackAttachComponent(), ARunner::GetStackHookComponent(), ARunner::HandleTemplateChanged(), ARunner::SetMinMax(), ARunner::TriggerUpdateAfterHidden(), UCesiumFlyToComponent, USettingsSubsystem

### Community 17 - "UNameComponent"
Cohesion: 0.08
Nodes (11): UNameComponent, bShow3DText, bShowWidgetText, LabelFont, LastLocation, NameBkgComponent, NameFont, NameSplineComponent (+3 more)

### Community 18 - "SearchBox"
Cohesion: 0.13
Nodes (7): createResults(), init_search(), SearchBox(), getXPos(), getYPos(), searchResults, convertToId()

### Community 20 - "AWorldFauna"
Cohesion: 0.06
Nodes (25): AWorldFauna::Tick(), AWorldFauna, Ambience, Anchor, AnchorGroundZ, AnchorTimer, AppliedDensity, bHasAnchor (+17 more)

### Community 21 - "ULoadingStatusSubsystem"
Cohesion: 0.08
Nodes (12): Clamp01(), ULoadingStatusSubsystem::Complete(), ULoadingStatusSubsystem::Fail(), ULoadingStatusSubsystem::MarkDirty(), ULoadingStatusSubsystem::RegisterTask(), ULoadingStatusSubsystem::SetRunning(), ULoadingStatusSubsystem::Update(), ULoadingStatusSubsystem (+4 more)

### Community 22 - "jquery.js"
Cohesion: 0.08
Nodes (13): at(), getEventsNS(), getViewportDimensions(), initMouseDetection(), initTracking(), isTouchEvent(), lt(), trackMouse() (+5 more)

### Community 23 - "URaceSubsystem"
Cohesion: 0.06
Nodes (11): UTeamGroupSubsystem::Initialize(), UTeamSubsystem::Initialize(), UWeatherSubsystem::Initialize(), URaceSubsystem, CurrentRaceId, CurrentRaceSetup, MainURL, OnRaceSetupDatasGathered (+3 more)

### Community 24 - "UPoiSubsystem"
Cohesion: 0.06
Nodes (10): UPoiSubsystem, ActivePoiRequests, CurrentRaceSetup, OnPoiDatasGathered, OnPoisDatasGathered, POI, POIs, PoisActorsMap (+2 more)

### Community 25 - "AWorldFlora"
Cohesion: 0.06
Nodes (21): AWorldFlora, AllComponents, Ambience, bBuilding, bShown, BuildCenter, BuildDensity, BuildRadius (+13 more)

### Community 26 - "Checkpoint.cpp"
Cohesion: 0.07
Nodes (3): ACheckpoint::CapitalizeFirst(), ACheckpoint::HandleTemplateChanged(), ACheckpoint::SetMinMax()

### Community 27 - "APath::DrawPath"
Cohesion: 0.17
Nodes (6): APath::BeginPlay(), APath::DrawPath(), APath::GetGeoidUndulationM(), APath::MoveCheckpoint(), FGeoidGrid, ReplaceUrlParam()

### Community 28 - "UWaterMaskSubsystem"
Cohesion: 0.07
Nodes (12): UWaterMaskSubsystem::HandlePathDatasGathered(), UWaterMaskSubsystem::IsWaterAt(), FWaterPolygon, Bounds, Rings, UWaterMaskSubsystem, ActiveRequests, bWaterPolygonsDirty (+4 more)

### Community 29 - "UAudioSanitizerSubsystem"
Cohesion: 0.06
Nodes (19): UAudioSanitizerSubsystem, bCulpritRetrying, BeginPlayTime, bSoundOpened, CheckTimer, ClippedSinceLog, Culprit, CulpritRetryDelay (+11 more)

### Community 30 - "Path.cpp"
Cohesion: 0.06
Nodes (20): ActionValue(), APath::APath(), APath::BuildKms(), APath::GetCheckpointClassForRace(), APath::HandleCheckpointsDatasGathered(), APath::RebuildPathSplineMeshes(), APath::RebuildSlopeSplineMeshes(), APath::SpawnCheckpointAt() (+12 more)

### Community 31 - "Poi.cpp"
Cohesion: 0.06
Nodes (5): APoi::CapitalizeFirst(), APoi::GetBoundsLocation(), APoi::HandleTemplateChanged(), APoi::OnLookAtTimerTick(), APoi::SetMinMax()

### Community 32 - "CinematicTrail.cpp"
Cohesion: 0.07
Nodes (15): FRacePOI, elevation, lat, lon, name, poiId, type, weather (+7 more)

### Community 33 - "splinemeshcomponent"
Cohesion: 0.08
Nodes (11): ACheckpoint_Nike::RefreshCheckpointValues(), ACheckpoint_Nike::UpdateCheckpoint(), APoi_Nike::UpdatePoi(), UGeneric_NameComponent::GetMiddle(), UGeneric_NameComponent::UGeneric_NameComponent(), UGeneric_NameComponent::UpdateName(), UNike_ClubComponent::UNike_ClubComponent(), UNike_ClubComponent::UpdateClub() (+3 more)

### Community 34 - "UPathSubsystem"
Cohesion: 0.07
Nodes (9): UPathSubsystem, ActivePathRequests, CurrentRaceSetup, MapPaths, OnPathDatasGathered, RacePath, RacePathsMap, RaceSubsystem (+1 more)

### Community 35 - "SettingsSubsystem.cpp"
Cohesion: 0.14
Nodes (18): USettingsSubsystem::ApplyGeoidCorrectionById(), USettingsSubsystem::CreateTrailSettingsById(), USettingsSubsystem::DoesSettingsExist(), USettingsSubsystem::GetFetchById(), USettingsSubsystem::GetGlowById(), USettingsSubsystem::GetMinMaxById(), USettingsSubsystem::GetPulseFrequencyById(), USettingsSubsystem::GetTrailSettingsById() (+10 more)

### Community 36 - "Templates Unreal (TrailSimulator) servis par OnGraphics"
Cohesion: 0.11
Nodes (18): 1. Le contexte en deux minutes, 2. Réponse à « conventions Figma ou JSON de template ? », 3.1 Routes attendues sur OnGraphics, 3.2 Format (version 1), 3.3 Règles de valeur, 3.4 Ce qu'Unreal ne lit pas (inutile de l'envoyer), 3. Le contrat : `GET /unreal/template/{nom}`, 4.1 Trois graphics de plus (+10 more)

### Community 37 - "SLoadingOverlay"
Cohesion: 0.11
Nodes (10): SLoadingOverlay, bDirty, CardsBox, ChangedHandle, LastRebuildTime, LoadingSubsystem, PercentText, SummaryText (+2 more)

### Community 38 - "URunnerStackingSubsystem"
Cohesion: 0.09
Nodes (9): URunnerStackingSubsystem::TickSubsystem(), URunnerStackingSubsystem, bDirty, bHasLastCamLoc, CameraMoveThresholdCm, Config, Entries, LastCamLoc (+1 more)

### Community 39 - "UTeamSubsystem"
Cohesion: 0.06
Nodes (25): FRaceTeamRunnerIndex, RunnerToTeam, TeamToRunner, UCesiumFlyToComponent, UTeamSubsystem::Animation(), UTeamSubsystem::ClearRaceTeamIndex(), UTeamSubsystem::DisplayClub(), UTeamSubsystem::DisplayFlag() (+17 more)

### Community 40 - "UWeatherSubsystem"
Cohesion: 0.12
Nodes (6): UWeatherSubsystem, ActiveWeatherRequests, CurrentRaceSetup, OnWeatherGathered, RaceSubsystem, WeatherResponse

### Community 41 - "AKm"
Cohesion: 0.05
Nodes (16): APath::GetKmClassForRace(), AKm_Generic, AKm_GTWS, AKm, DynaPawn, Georeference, GlobeAnchorComponent, LabelComp (+8 more)

### Community 43 - "unrealbuildtool"
Cohesion: 0.11
Nodes (9): SlateNotifications, TemplateGenerator, SharedTypes, Subsystems, TrailInterfaces, TrailSimulatorTarget, TrailSimulator, TrailSimulatorEditorTarget (+1 more)

### Community 44 - "ACinematicGeoAnchor"
Cohesion: 0.12
Nodes (11): ACinematicGeoAnchor::Align(), ACinematicGeoAnchor::ClipCesium(), ACinematicGeoAnchor::LambertToUnreal(), ACinematicGeoAnchor, CenterLonLatHeight, ClipMarginM, ClipPolygon, HeightOffsetM (+3 more)

### Community 45 - "UBroadcastCaptureSubsystem"
Cohesion: 0.08
Nodes (10): UBroadcastCaptureSubsystem, Accum, ActiveSource, CachedPC, DefaultSource, MirrorViewport, MirrorWidget, MonitorHz (+2 more)

### Community 46 - "FAutoWeatherInfo"
Cohesion: 0.08
Nodes (24): FAutoWeatherInfo, AgeMinutes, bHasData, CameraAltitudeM, CameraTempC, CloudBaseM, Clouds, Description (+16 more)

### Community 47 - "UClubComponent"
Cohesion: 0.06
Nodes (15): UClubComponent, bShow3DText, bShowWidgetText, ClubBkgComponent, ClubFont, ClubSplineComponent, ClubTextComponent, LastLocation (+7 more)

### Community 48 - "FEntry"
Cohesion: 0.20
Nodes (7): FEntry, Actor, CurrentMinMax, OriginalScale, OverrideConfig, UScaleSubsystem::ComputeTargetScale(), UScaleSubsystem::GetCameraLocation()

### Community 49 - "RunnerSubsystem.cpp"
Cohesion: 0.20
Nodes (14): URunnerSubsystem::Animation(), URunnerSubsystem::AnimationByTeam(), URunnerSubsystem::CancelRunnersRetry(), URunnerSubsystem::DoesRunnerBelongsToRace(), URunnerSubsystem::GetRunnerActor(), URunnerSubsystem::GetRunnerActorByTeam(), URunnerSubsystem::SetRaceSetup(), URunnerSubsystem::SpawnRunnerActor() (+6 more)

### Community 50 - "UTrailTextWidgetComponent"
Cohesion: 0.09
Nodes (7): UTrailTextWidgetComponent, CurrentColor, CurrentFont, CurrentJustification, CurrentSize, CurrentText, SlateTextBlock

### Community 51 - "FOpenWeatherCurrent"
Cohesion: 0.09
Nodes (22): FOpenWeatherCondition, description, icon, id, main, FOpenWeatherCurrent, clouds, dew_point (+14 more)

### Community 52 - "UScaleSubsystem"
Cohesion: 0.09
Nodes (3): UScaleSubsystem, Entries, GlobalConfig

### Community 53 - "FWorldAmbienceSettings"
Cohesion: 0.08
Nodes (23): UWorldAmbienceSubsystem::SetSettings(), FWorldAmbienceSettings, AutoWeatherRefreshMinutes, bAutoWeather, bCityLights, bFauna, bFlora, bHeadlamps (+15 more)

### Community 54 - "FWeatherSample"
Cohesion: 0.11
Nodes (15): FBenchResult, FrameAvg, FrameP99, GpuAvg, GpuP99, Name, Samples, FWeatherSample (+7 more)

### Community 55 - "PoiSubsystem.cpp"
Cohesion: 0.18
Nodes (13): ConvertPOIJson(), ConvertPOIsJson(), UPoiSubsystem::Animation(), UPoiSubsystem::DoesPoiBelongsToRace(), UPoiSubsystem::GetPoiActor(), UPoiSubsystem::PerformHttpRequestForPOIs(), UPoiSubsystem::SetRaceSetup(), UPoiSubsystem::SpawnPoi() (+5 more)

### Community 56 - "ScaleSubsystem.cpp"
Cohesion: 0.12
Nodes (8): UScaleSubsystem::GetStatId(), UScaleSubsystem::HandleActorEndPlay(), UScaleSubsystem::Initialize(), UScaleSubsystem::IsTickable(), UScaleSubsystem::RegisterScalableActor(), UScaleSubsystem::SetActorScaleMinMax(), UScaleSubsystem::SetGlobalConfig(), UScaleSubsystem::UnregisterScalableActor()

### Community 57 - "ARunner_UTMB"
Cohesion: 0.09
Nodes (15): ARunner_UTMB, FlagComponent, FlagHook, IndexBarreComponent, IndexBkgComponent, IndexBkgHook, IndexLabelComponent, IndexLeftComponent (+7 more)

### Community 58 - "AudioSanitizer.cpp"
Cohesion: 0.11
Nodes (9): DayPhase(), FSubmixEffectSanitizer::OnProcessAudio(), SetWorldMute(), SoundName(), UAudioSanitizerSubsystem::CheckNaN(), UAudioSanitizerSubsystem::Deinitialize(), UAudioSanitizerSubsystem::Initialize(), UAudioSanitizerSubsystem::UpdateForest() (+1 more)

### Community 59 - "FRaceCheckpoint"
Cohesion: 0.13
Nodes (15): FRaceCheckpoint, altitude, checkpointId, distance, elevation, name, type, weather (+7 more)

### Community 60 - "PathSubsystem.cpp"
Cohesion: 0.16
Nodes (7): UPathSubsystem::CreatePathRequest(), UPathSubsystem::GetPathById(), UPathSubsystem::ParsePathJson(), UPathSubsystem::PerformHttpRequestForPath(), UPathSubsystem::SetRaceSetup(), UPathSubsystem::SpawnKmActor(), UPathSubsystem::SpawnPathActor()

### Community 61 - "TrailTextWidgetComponent.cpp"
Cohesion: 0.12
Nodes (9): UTrailTextWidgetComponent::GetFontInfo(), UTrailTextWidgetComponent::OnRegister(), UTrailTextWidgetComponent::SetLabelCasseToLower(), UTrailTextWidgetComponent::SetLabelCasseToUpper(), UTrailTextWidgetComponent::SetLabelColor(), UTrailTextWidgetComponent::SetLabelFont(), UTrailTextWidgetComponent::SetLabelHJustification(), UTrailTextWidgetComponent::SetLabelText() (+1 more)

### Community 62 - "WorldFlora.cpp"
Cohesion: 0.14
Nodes (5): AWorldFlora::HandlePathGathered(), AWorldFlora::IsOnTrail(), AWorldFlora::StartBuild(), AWorldFlora::TraceGround(), Hash()

### Community 63 - "SLoadingOverlay.cpp"
Cohesion: 0.10
Nodes (6): Font(), FStepDef, Label, Prefix, SLoadingOverlay::GetOverallPercent(), SLoadingOverlay::Tick()

### Community 64 - "ACinematicTrail"
Cohesion: 0.05
Nodes (25): ACinematicTrail, bCenterGeoreferenceOnStart, bShowTrail, bSpawnCheckpoints, bSpawnPois, CheckpointPointIndices, Checkpoints, Glow (+17 more)

### Community 66 - "ARunner_GTWS"
Cohesion: 0.10
Nodes (11): ARunner_GTWS, ClubComponent, ClubHook, DefaultClubLocation, DefaultNameLocation, FlagComponent, FlagHook, NameComponent (+3 more)

### Community 67 - "Templates Unreal (TrailSimulator) servis par OnGraphics"
Cohesion: 0.11
Nodes (18): 1. Le contexte en deux minutes, 2. Réponse à « conventions Figma ou JSON de template ? », 3.1 Routes attendues sur OnGraphics, 3.2 Format (version 1), 3.3 Règles de valeur, 3.4 Ce qu'Unreal ne lit pas (inutile de l'envoyer), 3. Le contrat : `GET /unreal/template/{nom}`, 4.1 Trois graphics de plus (+10 more)

### Community 68 - "BroadcastCaptureSubsystem.cpp"
Cohesion: 0.15
Nodes (5): UBroadcastCaptureSubsystem::ActivateSource(), UBroadcastCaptureSubsystem::Deinitialize(), UBroadcastCaptureSubsystem::Initialize(), UBroadcastCaptureSubsystem::ResolveCaptureSourceFromViewTarget(), UBroadcastCaptureSubsystem::UpdateViewportMirror()

### Community 69 - "FTrailTemplate"
Cohesion: 0.12
Nodes (16): FTrailTemplate, Base, Checkpoint, Globals, Name, Poi, Runner, FTrailTemplateGlobals (+8 more)

### Community 70 - "FLoadingTaskInfo"
Cohesion: 0.12
Nodes (13): ELoadingTaskState, Failed, Queued, Running, Success, FLoadingTaskInfo, Color, Detail (+5 more)

### Community 71 - "WorldFauna.cpp"
Cohesion: 0.12
Nodes (5): AWorldFauna::BeginPlay(), AWorldFauna::GroundZAt(), AWorldFauna::Respawn(), AWorldFauna::UpdateAnchor(), MaxBirdHeight()

### Community 72 - "FHttpResultCallback"
Cohesion: 0.18
Nodes (8): UWorldAmbienceSubsystem::HandleBenchStartRequest(), UWorldAmbienceSubsystem::HandleBenchStateRequest(), UWorldAmbienceSubsystem::HandleForceRequest(), UWorldAmbienceSubsystem::HandleReleaseRequest(), UWorldAmbienceSubsystem::HandleSetRequest(), UWorldAmbienceSubsystem::HandleStateRequest(), UWorldAmbienceSubsystem::SetSettingByName(), UWorldAmbienceSubsystem::TickAutoWeather()

### Community 73 - "FBird"
Cohesion: 0.11
Nodes (17): EBirdType, Flock, Raptor, FBird, FlapAmount, FlapPhase, FlapSpeed, FlapTimer (+9 more)

### Community 74 - "ACheckpoint_Generic"
Cohesion: 0.11
Nodes (12): ACheckpoint_Generic, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID, NameLeftComponent (+4 more)

### Community 75 - "ACheckpoint_Nike"
Cohesion: 0.11
Nodes (14): ACheckpoint_Nike, Infos1BkgComponent, Infos1SplineComponent, Infos1WidgetComponent, Infos2BkgComponent, Infos2SplineComponent, Infos2WidgetComponent, InfosFont (+6 more)

### Community 76 - "initResizable"
Cohesion: 0.16
Nodes (10): initNavTree(), initPageToc(), render(), initResizable(), constrainPanelWidths(), resizeHeight(), resizeWidth(), restoreWidth() (+2 more)

### Community 77 - "PhotoComponent.cpp"
Cohesion: 0.08
Nodes (10): UPhotoComponent::ClearPhoto(), UPhotoComponent::UpdateDayNight(), UPhotoComponent::UpdatePhoto(), UPhotoComponent, BaseMat, CurrentPhotoTexture, CurrentPhotoUrl, PendingPhotoUrl (+2 more)

### Community 78 - "RunnerStackingSubsystem.cpp"
Cohesion: 0.13
Nodes (11): URunnerStackingSubsystem::ApplyNewStackingState(), URunnerStackingSubsystem::AttachRunnerToBase(), URunnerStackingSubsystem::ComputeDynamicRadiusCm(), URunnerStackingSubsystem::ComputeOverlapRadiusCm(), URunnerStackingSubsystem::Deinitialize(), URunnerStackingSubsystem::DetachRunnerToTrack(), URunnerStackingSubsystem::FindEntryIndex(), URunnerStackingSubsystem::HandleActorEndPlay() (+3 more)

### Community 79 - "APoi_GTWS"
Cohesion: 0.12
Nodes (12): AWorldFauna::BuildMeshes(), UMaterialInstanceDynamic, UPhotoComponent::EnsurePhotoMID(), APoi_GTWS, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat (+4 more)

### Community 80 - "AWorldHeadlamps"
Cohesion: 0.08
Nodes (11): AWorldHeadlamps::BeginPlay(), AWorldHeadlamps::GetLamp(), AWorldHeadlamps::Tick(), AWorldHeadlamps, Ambience, Lamps, LitCount, RefreshTimer (+3 more)

### Community 81 - "APoi_Nike"
Cohesion: 0.12
Nodes (13): APoi_Nike, Infos1BkgComponent, Infos1SplineComponent, Infos1WidgetComponent, Infos2BkgComponent, Infos2SplineComponent, Infos2WidgetComponent, InfosFont (+5 more)

### Community 82 - "TSubclassOf"
Cohesion: 0.33
Nodes (3): UCheckpointSubsystem::GetCheckpointActor(), UCheckpointSubsystem::SpawnCheckpointActor(), TSubclassOf

### Community 83 - "UPathSegmentsComponent"
Cohesion: 0.06
Nodes (18): UPathSegmentsComponent, ExtraPieces, HighlightedBaseScales, HighlightedPieces, HighlightThickness, LabelHeight, LabelIllum, LabelPadding (+10 more)

### Community 84 - "Ee"
Cohesion: 0.16
Nodes (16): A(), b(), be(), Ee(), f(), he(), je(), k() (+8 more)

### Community 85 - "e"
Cohesion: 0.19
Nodes (16): ce(), e(), fe(), H(), I(), l(), le(), M() (+8 more)

### Community 86 - "NameComponent.cpp"
Cohesion: 0.15
Nodes (6): UNameComponent::GetLocalSize(), UNameComponent::GetSplineComponent(), UNameComponent::GetWorldLastPointLocation(), UNameComponent::UNameComponent(), UNameComponent::UpdateMesh(), UNameComponent::UpdateName()

### Community 87 - "ARaceManager::SpawnInitialRunners"
Cohesion: 0.31
Nodes (4): ARaceManager::ProcessPendingRunnerUpdates(), ARaceManager::SpawnInitialRunners(), ARaceManager::UpdateRunnersFromSnapshot(), LogRaceTeamIndex()

### Community 88 - "GTWS_NameComponent.cpp"
Cohesion: 0.08
Nodes (10): UGTWS_ClubComponent::SetColors(), UGTWS_ClubComponent::UGTWS_ClubComponent(), UGTWS_ClubComponent::UpdateClub(), UGTWS_NameComponent::GetSplineLastPoint(), UGTWS_NameComponent::GetStartAndEnd(), UGTWS_NameComponent::SetColors(), UGTWS_NameComponent::UGTWS_NameComponent(), UGTWS_NameComponent::UpdateName() (+2 more)

### Community 89 - "ACinematicTrailCamera"
Cohesion: 0.07
Nodes (19): Ce que fait le code, Importer et caler un nuage, Le « décalottage », Level cinéma : export vidéo d'un survol de course, Limites connues, Nuages de points et modèles géoréférencés (film Mont Blanc), Rendu, Tracé de la voie normale (+11 more)

### Community 90 - "APoi_Generic"
Cohesion: 0.12
Nodes (11): APoi_Generic, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, NameLeftComponent, NameRightComponent (+3 more)

### Community 91 - "SBroadcastMirror"
Cohesion: 0.14
Nodes (5): SBroadcastMirror, Background, bLogged, Brush, Target

### Community 92 - "TrailSharedTypes.h"
Cohesion: 0.08
Nodes (29): ERunnerTemplate, DisplayName, FCheckpoints, Checkpoints, FPOIs, POIs, FRaceEntries, RacesEntries (+21 more)

### Community 94 - "UCameraControlComponent"
Cohesion: 0.15
Nodes (4): UCameraControlComponent, CameraComponent, OwlCapture, SpringArmComponent

### Community 95 - "UMeteoPageSubsystem"
Cohesion: 0.11
Nodes (8): FHttpServerRequest, IHttpRouter, UMeteoPageSubsystem, GeoRouteHandle, GeoSetRouteHandle, HttpRouter, ListenPort, PageRouteHandle

### Community 96 - "UTeamGroupSubsystem"
Cohesion: 0.13
Nodes (5): UTeamGroupSubsystem, CurrentRaceSetup, RaceGroupTeamMap, RaceSubsystem, TeamSubsystem

### Community 97 - "Km.cpp"
Cohesion: 0.14
Nodes (4): AKm::AKm(), AKm::BeginPlay(), AKm::OnLookAtTimerTick(), AKm::UpdateKm()

### Community 98 - "ACheckpoint_GTWS"
Cohesion: 0.13
Nodes (10): ACheckpoint_GTWS, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID, RaceManager (+2 more)

### Community 99 - "ACheckpoint_UTMB"
Cohesion: 0.13
Nodes (10): ACheckpoint_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID, RaceManager (+2 more)

### Community 100 - "UTemplateSubsystem"
Cohesion: 0.11
Nodes (6): UTemplateSubsystem, ActiveRequests, OnTemplateChanged, Templates, ARunner::UpdateRunner(), ARaceManager::PrepareRunnerRuntimeData()

### Community 101 - "CinematicGeoAnchor.cpp"
Cohesion: 0.10
Nodes (5): ACinematicGeoAnchor::GetLocalBounds(), ACinematicGeoAnchor::Lambert93ToLonLat(), ACinematicGeoAnchor::LocalToLambert(), UCesiumBudgetSubsystem::DoesSupportWorldType(), UCesiumBudgetSubsystem::GetStatId()

### Community 102 - "WaterMaskSubsystem.cpp"
Cohesion: 0.14
Nodes (10): BufferLine(), IsClosed(), LineWidthMeters(), OverpassToFeatures(), PointInRing(), ReadGeometry(), RingToJson(), StitchRings() (+2 more)

### Community 103 - "WeatherSubsystem.cpp"
Cohesion: 0.12
Nodes (6): ConvertRacesJson(), UTrailsSubsystem::Initialize(), UTrailsSubsystem::PerformHttpRequestForRaces(), ConvertWeatherJson(), UWeatherSubsystem::Deinitialize(), UWeatherSubsystem::PerformHttpRequestForWeather()

### Community 104 - ".GetGameInstance"
Cohesion: 0.10
Nodes (17): UCheckpointSubsystem::Initialize(), UPathSubsystem::Initialize(), UPoiSubsystem::Initialize(), URunnerSubsystem::CancelAllRunnersRetries(), URunnerSubsystem::Initialize(), UWorldAmbienceSubsystem::DoesSupportWorldType(), Type, ACheckpoint::BeginPlay() (+9 more)

### Community 105 - "FWorldAmbienceState"
Cohesion: 0.14
Nodes (13): FWorldAmbienceState, CloudCoverage, Fog, FrameMs, GpuMs, GuardLevel, Night, Rain (+5 more)

### Community 106 - "UWorld"
Cohesion: 0.14
Nodes (16): FindPlayWorld(), FindActorByClassToken(), ReadNumber(), SetPawnInput(), UAudioSanitizerSubsystem::OnWorldBeginPlay(), UAudioSanitizerSubsystem::ShouldCreateSubsystem(), FWorldUtils::GetGameInstance(), FWorldUtils::GetGameInstanceChecked() (+8 more)

### Community 107 - "ARunner_Generic"
Cohesion: 0.12
Nodes (7): ARunner_Generic, FlagComponent, NameComponent, PhotoComponent, PhotoHook, RaceManager, SlashComponent

### Community 108 - "APoi_UTMB"
Cohesion: 0.14
Nodes (9): APoi_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, RaceManager, WeatherPictoMat (+1 more)

### Community 110 - "FVector"
Cohesion: 0.25
Nodes (10): APath::BuildSmoothCameraRail(), APath::GetClosestSplineLocation(), APath::GetDistanceAlongSpline(), APath::GetLocationAtDistance(), BuildRail(), BuildSimplifiedRailLocked(), BuildSmoothRail(), DistPointSegment3D() (+2 more)

### Community 111 - "TeamGroupSubsystem.cpp"
Cohesion: 0.25
Nodes (5): UTeamGroupSubsystem::AddTeamToGroup(), UTeamGroupSubsystem::GetTeamsInGroup(), UTeamGroupSubsystem::RemoveTeamFromGroup(), UTeamGroupSubsystem::SetRaceSetup(), UTeamGroupSubsystem::ToggleGroup()

### Community 112 - "showTip"
Cohesion: 0.28
Nodes (13): countFlags(), CSSCoordinates(), getTooltipContent(), getViewportCollisions(), isMouseOver(), TooltipController(), beginShowTip(), closeDesyncedTip() (+5 more)

### Community 113 - "TrailHttpDebug.h"
Cohesion: 0.18
Nodes (6): FTrailHttpDebug, FTrailHttpDebugOptions, bLogBody, bLogHeaders, bSkipBodyIfBinary, MaxBodyChars

### Community 114 - "FTrailHttpDebug::LogAndIsSuccess"
Cohesion: 0.27
Nodes (3): FTrailHttpDebug::LogAndIsSuccess(), FTrailHttpDebug::LogStart(), IsLikelyBinaryContent()

### Community 115 - "UTrailsSubsystem"
Cohesion: 0.15
Nodes (6): UTrailsSubsystem, CurrentRaceSetup, LoadingSubsystem, OnRacesDatasGathered, RacesArray, RaceSubsystem

### Community 116 - ".GetWorld"
Cohesion: 0.13
Nodes (12): ITrailInputModeInterface, ACheckpoint::ACheckpoint(), ACheckpoint::OnLookAtTimerTick(), ACheckpoint::StartAnimation(), ACheckpoint::StopAnimation(), APoi::StartAnimation(), APoi::StopAnimation(), ARunner::AdvanceTrackInterp() (+4 more)

### Community 117 - "JsonResponse"
Cohesion: 0.27
Nodes (6): FindGeoreference(), GeoJson(), JsonResponse(), UMeteoPageSubsystem::HandleGeoRequest(), UMeteoPageSubsystem::HandleGeoSetRequest(), UMeteoPageSubsystem::HandlePageRequest()

### Community 118 - "USettingsSaveGame"
Cohesion: 0.15
Nodes (7): USettingsSubsystem::Load(), USettingsSubsystem::SetMainURL(), USettingsSaveGame, GlobalLength, GlobalPitch, GlobalZAnchor, MainURL

### Community 119 - "FSettings"
Cohesion: 0.13
Nodes (14): FSettings, bGeoidCorrected, FetchFrequency, GlowValue, Length, MinMax, Pitch, PulseFrequency (+6 more)

### Community 120 - "TemplateSubsystem.cpp"
Cohesion: 0.20
Nodes (8): UTemplateSubsystem::Fetch(), UTemplateSubsystem::GetBase(), UTemplateSubsystem::GetBuiltin(), UTemplateSubsystem::GetForRace(), UTemplateSubsystem::GetRunnerHeight(), UTemplateSubsystem::GetTemplate(), UTemplateSubsystem::ReloadTemplate(), UTemplateSubsystem::RequestTemplate()

### Community 121 - "FBenchShot"
Cohesion: 0.15
Nodes (12): UWorldAmbienceSubsystem::ApplyBenchShot(), UWorldAmbienceSubsystem::PlaceCameraOnGround(), FBenchShot, AboveGroundM, HeightM, Lat, Lon, Name (+4 more)

### Community 122 - "FSubmixEffectSanitizer"
Cohesion: 0.17
Nodes (5): FSubmixEffectSanitizer, BadSamples, ClippedSamples, CurrentGain, TargetGain

### Community 123 - "FDistanceScaleConfig"
Cohesion: 0.17
Nodes (11): FDistanceScaleConfig, bInterp, bSmoothStep, bUseActorOriginalScaleAsMax, bUseXYOnly, FarDistanceCm, InterpSpeed, NearDistanceCm (+3 more)

### Community 124 - "FStackEntry"
Cohesion: 0.17
Nodes (11): FStackEntry, bClubVisible, bPhotoVisible, CurrentBase, CurrentOrder, HookHeightCm, HookLocalInAttach, OriginalScaleX (+3 more)

### Community 125 - "HttpGatewaySubsystem.h"
Cohesion: 0.12
Nodes (9): FCheckpointMove, Altitude, CheckpointID, Distance, Elevation, Lat, Lon, PathIndex (+1 more)

### Community 126 - "CallBP_GetFloatOut"
Cohesion: 0.18
Nodes (7): ARaceManager::EdUpdateMainUrl(), ARaceManager::HandleRacesDatasGathered(), ARaceManager::RtUpdateMainUrl(), ARaceManager::UpdateMainUrl_Internal(), ARaceManager::UpdateUdsTime(), CallBP_GetFloatOut(), SanitizeKey()

### Community 127 - "CheckpointSubsystem.cpp"
Cohesion: 0.22
Nodes (13): ConvertCheckpointJson(), ConvertCheckpointsJson(), UCheckpointSubsystem::Animation(), UCheckpointSubsystem::DoesChkBelongsToRace(), UCheckpointSubsystem::GetCheckpointByChkIdRaceId(), UCheckpointSubsystem::PerformHttpRequestForCheckpoints(), UCheckpointSubsystem::SetRaceSetup(), UCheckpointSubsystem::StopAnimation() (+5 more)

### Community 128 - "UTemplateGeneratorUtilityWidget"
Cohesion: 0.22
Nodes (4): UTemplateGeneratorUtilityWidget, CheckpointParentClass, PoiParentClass, TeamParentClass

### Community 129 - "FPointIndexSegment"
Cohesion: 0.29
Nodes (6): FPointIndexSegment, EndDistanceCm, EndIndex, Indices, StartDistanceCm, StartIndex

### Community 130 - "FMinMax"
Cohesion: 0.15
Nodes (11): FMinMax, Max, Min, FStartAndEnd, EndLocation, StartLocation, ACheckpoint::GetMinMax(), APoi::GetMinMax() (+3 more)

### Community 131 - "FRunnerStackingConfig"
Cohesion: 0.18
Nodes (8): FRunnerStackingConfig, Gamma, HysteresisCm, RadiusFarCm, RadiusNearCm, UpdateIntervalSec, URunnerStackingSubsystem::Initialize(), URunnerStackingSubsystem::SetConfig()

### Community 132 - "Runner_GTWS.cpp"
Cohesion: 0.07
Nodes (14): ARunner_Generic::BeginPlay(), ARunner_Generic::ToggleFlag(), ARunner_Generic::TogglePhoto(), ARunner_Generic::UpdateRunner(), ARunner_GTWS::BeginPlay(), ARunner_GTWS::ToggleClub(), ARunner_GTWS::ToggleFlag(), ARunner_GTWS::TogglePhoto() (+6 more)

### Community 133 - "Runner.h"
Cohesion: 0.08
Nodes (3): IBroadcastInterface, IScaleInterface, ITeamInterface

### Community 134 - "FRaceCard"
Cohesion: 0.18
Nodes (5): FRaceCard, RaceId, Steps, Title, SplitTaskId()

### Community 135 - "UGTWS_NameComponent"
Cohesion: 0.17
Nodes (3): UGTWS_NameComponent, AppliedBkgColor, TextColor

### Community 136 - "UFlagComponent"
Cohesion: 0.18
Nodes (5): UFlagComponent, bIsDayCached, CurrentCountry, FlagMID, FlagSMC

### Community 137 - "FTrailTemplateRunner"
Cohesion: 0.15
Nodes (13): FTrailTemplateRunner, BkgLeftMargin, BkgRightMargin, ClubBkg, ClubText, Foot, Height, NameBkg (+5 more)

### Community 138 - "FString"
Cohesion: 0.27
Nodes (6): ConvertRunnerJson(), ConvertRunnersJson(), URunnerSubsystem::PerformHttpRequestForRunner(), URunnerSubsystem::PerformHttpRequestForRunners(), URunnerSubsystem::RequestRunnersWithRetry(), URunnerSubsystem::ScheduleRunnersRetry()

### Community 139 - "FTrailLabelStyle"
Cohesion: 0.29
Nodes (4): FTrailLabelStyle, Color, DrawSize, Font

### Community 141 - "MeteoPageSubsystem.cpp"
Cohesion: 0.14
Nodes (3): UHttpGatewaySubsystem::HandleCommandRequest(), UHttpGatewaySubsystem::Initialize(), UMeteoPageSubsystem::Initialize()

### Community 142 - "FRaceSetup"
Cohesion: 0.07
Nodes (29): FRaceSetup, arrivalCity, arrivalCountry, arrivalCountryFlag, cat, color, departureCity, departureCountry (+21 more)

### Community 143 - "PathSegmentsComponent.cpp"
Cohesion: 0.10
Nodes (9): ACheckpoint_UTMB::BeginPlay(), ACheckpoint_UTMB::UpdateCheckpoint(), APoi_UTMB::BeginPlay(), APoi_UTMB::UpdatePoi(), UPathSegmentsComponent::BeginPlay(), UPathSegmentsComponent::ConfigurePiece(), UPathSegmentsComponent::CreatePiece(), UPathSegmentsComponent::OnLookAtTimerTick() (+1 more)

### Community 144 - "WorldFlora.h"
Cohesion: 0.22
Nodes (5): FKindSet, Components, UHierarchicalInstancedStaticMeshComponent, UStaticMesh, UWorldAmbienceSubsystem

### Community 145 - "ARunner_Nike"
Cohesion: 0.18
Nodes (7): ARunner_Nike, ClubComponent, ClubFont, ClubHook, FlagComponent, LabelFont, NameComponent

### Community 146 - "AudioSanitizer.h"
Cohesion: 0.22
Nodes (5): ENaNHuntPhase, Done, Idle, Stop, USubmixEffectSanitizerPreset

### Community 147 - "BroadcastSettings.h"
Cohesion: 0.25
Nodes (3): UBroadcastSettings, bMirrorCaptureToViewport, UTextureRenderTarget2D

### Community 148 - "FPathSegment"
Cohesion: 0.18
Nodes (10): FPathSegment, bVisible, CheckpointIn, CheckpointOut, Color, KmIn, KmOut, Label (+2 more)

### Community 150 - "DisplayController"
Cohesion: 0.54
Nodes (7): DisplayController(), cancelClose(), cancelTimer(), checkForIntent(), closeAnyDelayed(), closeTooltip(), openTooltip()

### Community 151 - "ClubComponent.cpp"
Cohesion: 0.18
Nodes (4): UClubComponent::GetLocalSize(), UClubComponent::UClubComponent(), UClubComponent::UpdateClub(), UClubComponent::UpdateMesh()

### Community 152 - "UGeneric_NameComponent"
Cohesion: 0.22
Nodes (3): UGeneric_NameComponent, NameLeftComponent, NameRightComponent

### Community 155 - "splinecomponent"
Cohesion: 0.11
Nodes (6): ACinematicTrailCamera::ACinematicTrailCamera(), ACinematicTrailCamera::OnConstruction(), ACinematicTrailCamera::Tick(), UUTMB_NameComponent::UpdateMesh(), UUTMB_NameComponent::UpdateName(), UUTMB_NameComponent::UUTMB_NameComponent()

### Community 156 - "FGroupStruct"
Cohesion: 0.22
Nodes (6): FGroups, Groups, RaceId, FGroupStruct, groupId, Teams

### Community 159 - "UUTMB_NameComponent"
Cohesion: 0.25
Nodes (3): UUTMB_NameComponent, RightSlashComponent, SplineComponent

### Community 160 - "PlacementCalculator"
Cohesion: 0.47
Nodes (5): isSvgElement(), PlacementCalculator(), computePlacementCoords(), getHtmlPlacement(), getSvgPlacement()

### Community 162 - "FRunnerState"
Cohesion: 0.33
Nodes (4): FRunnerState, CurrentOffset, CurrentScale, Runner

### Community 165 - "FRunners"
Cohesion: 0.33
Nodes (5): FRunners, Runners, URunnerSubsystem::GetRunnersByRaceId(), URunnerSubsystem::RemoveTeamsNotInSnapshot(), UTeamSubsystem::RebuildRaceTeamIndex()

### Community 167 - "FlagComponent.cpp"
Cohesion: 0.20
Nodes (3): UFlagComponent::UFlagComponent(), UFlagComponent::UpdateDayNight(), UFlagComponent::UpdateFlag()

### Community 171 - "FPendingCell"
Cohesion: 0.40
Nodes (3): FPendingCell, X, Y

### Community 172 - "EKind"
Cohesion: 0.40
Nodes (5): EKind, Count, Grass, Rock, Shrub

### Community 174 - "FTrailTemplateStack"
Cohesion: 0.15
Nodes (13): FTrailTemplateStack, Foot, InfosBkg, InfosPicto, InfosText, Line1, Line2, MainPicto (+5 more)

### Community 176 - "dynsections.js"
Cohesion: 0.50
Nodes (3): codefold, dynsection, toggleVisibility()

### Community 180 - "EBenchPhase"
Cohesion: 0.50
Nodes (4): EBenchPhase, Idle, Measuring, Settling

### Community 186 - "MakeStep"
Cohesion: 0.27
Nodes (8): SWidget, MakePill(), MakeStep(), Rounded(), SLoadingOverlay::Construct(), SLoadingOverlay::Rebuild(), StateColor(), SVerticalBox

### Community 189 - "ARaceManager::ToggleRace_Internal"
Cohesion: 0.14
Nodes (9): ARaceManager::EdToggleCheckpoints(), ARaceManager::EdTogglePath(), ARaceManager::EdTogglePois(), ARaceManager::EdToggleSlope(), ARaceManager::RtToggleCheckpoints(), ARaceManager::RtTogglePath(), ARaceManager::RtTogglePois(), ARaceManager::RtToggleSlope() (+1 more)

### Community 190 - "FState"
Cohesion: 0.25
Nodes (8): FState, bSpeedDownHeld, bSpeedUpHeld, HeightM, SpeedMult, Tight, ZoomMult, ResetPad()

### Community 191 - "FTrackSample"
Cohesion: 0.67
Nodes (3): FTrackSample, Dist, Time

### Community 194 - "UBroadcastCaptureSubsystem::RequestViewTarget"
Cohesion: 0.40
Nodes (3): UBroadcastCaptureSubsystem::RequestViewTarget(), UBroadcastCaptureSubsystem::Tick(), APlayerController

### Community 201 - "Automation_TrailSimulator.slnx"
Cohesion: 0.06
Nodes (35): Apple.Automation, AutomationTool, AutomationUtils.Automation, CookedEditor.Automation, CrowdinLocalization.Automation, EpicGames.Analytics.Telemetry, EpicGames.Build, EpicGames.Build.Tests (+27 more)

### Community 530 - "TrailSimulator.slnx"
Cohesion: 0.06
Nodes (34): Android.Automation, AutomationScripts.Automation, BuildGraph.Automation, EpicGames.Analytics.Generators, EpicGames.Analytics.PerformanceTrends, EpicGames.AspNet, EpicGames.Benchmarks, EpicGames.BuildGraph (+26 more)

### Community 533 - "RawTextMaterial.h"
Cohesion: 0.24
Nodes (4): Apply(), ApplyToAllTexts(), EnableOcclusion(), GetMaterial()

### Community 535 - "FOpenWeatherResponse"
Cohesion: 0.22
Nodes (8): FOpenWeatherResponse, current, lat, lon, timezone, timezone_offset, ACheckpoint::UpdateCheckpointWeather(), APoi::UpdatePoiWeather()

### Community 536 - "int64"
Cohesion: 0.13
Nodes (12): FRaceEntry, name, raceId, templateName, FTeams, RaceId, Teams, FTeamStruct (+4 more)

### Community 538 - "Generic_FlagComponent.h"
Cohesion: 0.25
Nodes (3): UGeneric_FlagComponent::UGeneric_FlagComponent(), UGeneric_FlagComponent, FlagHook

### Community 541 - "FWaterMaskBounds"
Cohesion: 0.15
Nodes (10): UWaterMaskSubsystem::ComputeBounds(), UWaterMaskSubsystem::GetCacheFilePath(), UWaterMaskSubsystem::OnRaceWaterReady(), UWaterMaskSubsystem::RequestOverpass(), UWaterMaskSubsystem::SendOverpassRequest(), FWaterMaskBounds, East, North (+2 more)

### Community 545 - "WorldAmbienceSubsystem.h"
Cohesion: 0.25
Nodes (6): UWorldAmbienceSubsystem::FindSunLight(), FHttpServerRequest, IHttpRouter, UDirectionalLightComponent, UMaterialParameterCollection, UVolumetricCloudComponent

### Community 546 - "FWeatherResult"
Cohesion: 0.25
Nodes (6): FWeatherResult, bSuccess, Data, Error, HttpCode, RequestKey

### Community 547 - "ACinematicTrail::SpawnRaceActors"
Cohesion: 0.25
Nodes (3): ACinematicTrail::BuildMarkers(), ACinematicTrail::Rebuild(), ACinematicTrail::SpawnRaceActors()

## Knowledge Gaps
- **1213 isolated node(s):** `TeamParentClass`, `PoiParentClass`, `CheckpointParentClass`, `Queued`, `Running` (+1208 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 3238 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **381 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `FRaceSetup` connect `FRaceSetup` to `ARaceManager`, `ARunner`, `Runner_GTWS.cpp`, `RaceManager.cpp`, `ACheckpoint`, `APoi`, `UCheckpointSubsystem`, `URunnerSubsystem`, `TemplateSubsystem.h`, `PathSegmentsComponent.cpp`, `URaceSubsystem`, `int64`, `UPoiSubsystem`, `Path.cpp`, `CinematicTrail.cpp`, `splinemeshcomponent`, `UPathSubsystem`, `UTeamSubsystem`, `UWeatherSubsystem`, `AKm`, `RunnerSubsystem.cpp`, `PoiSubsystem.cpp`, `FRaceCheckpoint`, `PathSubsystem.cpp`, `ACinematicTrail`, `TrailSharedTypes.h`, `UTeamGroupSubsystem`, `UTemplateSubsystem`, `TeamGroupSubsystem.cpp`, `UTrailsSubsystem`, `TemplateSubsystem.cpp`, `CheckpointSubsystem.cpp`?**
  _High betweenness centrality (0.102) - this node is a cross-community bridge._
- **What connects `TeamParentClass`, `PoiParentClass`, `CheckpointParentClass` to the rest of the system?**
  _1213 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ARaceManager` be split into smaller, more focused modules?**
  _Cohesion score 0.01 - nodes in this community are weakly interconnected._
- **Why does `ARaceManager` connect `ARaceManager` to `.SlateNotify`, `Runner_GTWS.cpp`, `APath`, `RaceManager.cpp`, `FRaceSetup`, `PathSegmentsComponent.cpp`, `TemplateSubsystem.h`, `FRunnerStruct`, `FPathSegment`, `ARaceManager::RtTeleportToTeam`, `ARaceManager::RtTogglePhoto`, `ARaceManager::RtUpdateMinMax`, `ARaceManager::EndPlay`, `ARaceManager::HideLoadingOverlay`, `ARaceManager::BeginPlay`, `ARaceManager::ToggleRace_Internal`, `TSubclassOf`, `ARaceManager::SpawnInitialRunners`, `TrailSharedTypes.h`, `Km.cpp`, `CallBP_GetFloatOut`?**
  _High betweenness centrality (0.095) - this node is a cross-community bridge._
- **Should `UHttpGatewaySubsystem` be split into smaller, more focused modules?**
  _Cohesion score 0.017241379310344827 - nodes in this community are weakly interconnected._
- **Why does `UWorldAmbienceSubsystem` connect `UWorldAmbienceSubsystem` to `WorldAmbienceSubsystem.h`, `Piecewise`, `FHttpResultCallback`, `FWorldAmbienceState`, `WorldAmbienceSubsystem.cpp`, `UWorldAmbienceSubsystem::OnWorldBeginPlay`, `FAutoWeatherInfo`, `UWorldAmbienceSubsystem::Initialize`, `EBenchPhase`, `FWorldAmbienceSettings`, `FWeatherSample`, `FBenchShot`, `TrailSharedTypes.h`?**
  _High betweenness centrality (0.093) - this node is a cross-community bridge._
- **Should `.SlateNotify` be split into smaller, more focused modules?**
  _Cohesion score 0.036017669045191984 - nodes in this community are weakly interconnected._