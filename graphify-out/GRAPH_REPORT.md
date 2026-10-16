# Graph Report - UNREAL 5.8  (2026-10-16)

## Corpus Check
- 537 files · ~1,773,489 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5284 file(s) not represented in the graph (top: .uasset 4702, .tex 369, .eps 71)

## Summary
- 4664 nodes · 6772 edges · 527 communities (159 shown, 368 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 542 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `5c2d9a41`
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
- Path.cpp
- Runner.cpp
- UNameComponent
- SearchBox
- Checkpoint_GTWS.cpp
- AWorldFauna
- ULoadingStatusSubsystem
- jquery.js
- URaceSubsystem
- UPoiSubsystem
- AWorldFlora
- Checkpoint.cpp
- UFlagComponent
- UWaterMaskSubsystem
- UAudioSanitizerSubsystem
- .GetWorld
- Poi.cpp
- gameplaystatics
- LoadingStatusSubsystem.cpp
- UPathSubsystem
- SettingsSubsystem.cpp
- Templates Unreal (TrailSimulator) servis par OnGraphics
- SLoadingOverlay
- URunnerStackingSubsystem
- UTeamSubsystem
- UWeatherSubsystem
- AKm
- FRaceSetup
- unrealbuildtool
- UPROPERTY
- UBroadcastCaptureSubsystem
- FAutoWeatherInfo
- UClubComponent
- FSettings
- RunnerSubsystem.cpp
- UTrailTextWidgetComponent
- FOpenWeatherCurrent
- UScaleSubsystem
- FWorldAmbienceSettings
- TrailHttpDebug.h
- PoiSubsystem.cpp
- ScaleSubsystem.cpp
- ARunner_UTMB
- AudioSanitizer.cpp
- FRaceCheckpoint
- TrailTextWidgetComponent.cpp
- WorldFlora.cpp
- SLoadingOverlay.cpp
- CheckpointSubsystem.cpp
- IRunnerInterface
- ARunner_GTWS
- Templates Unreal (TrailSimulator) servis par OnGraphics
- BroadcastCaptureSubsystem.cpp
- TemplateSubsystem.h
- FLoadingTaskInfo
- WorldFauna.cpp
- FHttpResultCallback
- FBird
- ACheckpoint_Generic
- ACheckpoint_Nike
- initResizable
- UPhotoComponent
- FMinMax
- APoi_GTWS
- AWorldHeadlamps
- APoi_Nike
- JsonResponse
- UPathSegmentsComponent
- Ee
- e
- PathSubsystem.cpp
- ARaceManager::SpawnInitialRunners
- GTWS_NameComponent.cpp
- RunnerStackingSubsystem.cpp
- APoi_Generic
- SBroadcastMirror
- TeamSubsystem.cpp
- ClubComponent.h
- UCameraControlComponent
- UMeteoPageSubsystem
- UTeamGroupSubsystem
- NameComponent.cpp
- ACheckpoint_GTWS
- ACheckpoint_UTMB
- UTemplateSubsystem
- TemplateSubsystem.cpp
- OverpassToFeatures
- TrailsSubsystem.cpp
- FPathSegment
- WorldAmbienceSubsystem.h
- UObject
- ARunner_Generic
- APoi_UTMB
- GTWS_ClubComponent.cpp
- PathSegmentsComponent.cpp
- UGeneric_NameComponent
- showTip
- UTemplateGeneratorUtilityWidget
- PhotoComponent.cpp
- UTrailsSubsystem
- WeatherSubsystem.cpp
- FTrailHttpDebug::LogAndIsSuccess
- USettingsSaveGame
- FRacePOI
- FBenchShot
- FSubmixEffectSanitizer
- FDistanceScaleConfig
- FStackEntry
- splinemeshcomponent
- CallBP_GetFloatOut
- SlateNotificationsBFL.h
- UTMB_NameComponent.cpp
- RawTextMaterial.h
- CesiumBudgetSubsystem.cpp
- FRunnerStackingConfig
- UGTWS_ClubComponent
- ICameraControlInterface
- FRaceCard
- UGTWS_NameComponent
- NameComponent.h
- UWaterMaskSubsystem::HandlePathDatasGathered
- FString
- int64
- FWaterMaskBounds
- ICheckpointInterface
- UUTMB_NameComponent
- WorldFlora.h
- ARunner_Nike
- AudioSanitizer.h
- BroadcastSettings.h
- WaterMaskSubsystem.cpp
- UCesiumBudgetSubsystem
- DisplayController
- FTrailLabelStyle
- BuildMesh
- ConvertPOIsJson
- TrailSharedTypes.h
- FStepDef
- IPoiInterface
- TrailUtilsLog.h
- PlacementCalculator
- AWorldFlora::BeginPlay
- FRunnerState
- ARaceManager::HideLoadingOverlay
- IPathInterface
- MeteoPageSubsystem.cpp
- TeamGroupSubsystem.cpp
- Runner_GTWS.cpp
- coreminimal
- UWorldAmbienceSubsystem::OnWorldBeginPlay
- FPendingCell
- EKind
- IRunnerStackableInterface
- dynsections.js
- UWorldAmbienceSubsystem::Initialize
- AWorldFlora::ContinueBuild
- AWorldFlora::Tick
- EBenchPhase
- UPoiSubsystem::SpawnPoi
- FPointIndexSegment
- makeTree
- ARaceManager::BeginPlay
- CLAUDE.md
- Piecewise
- ARaceManager::ToggleRace_Internal
- FTrackSample
- cookie.js
- .mcp.json
- UBroadcastCaptureSubsystem::RequestViewTarget
- URunnerSubsystem::Deinitialize
- USettingsSubsystem::Initialize
- USettingsSubsystem::Save
- Automation_TrailSimulator.slnx
- TrailSimulator.slnx
- UWorldAmbienceSubsystem::TickBench
- ARaceManager::RtToggleCheckpoint

## God Nodes (most connected - your core abstractions)
1. `ARaceManager` - 275 edges
2. `ARunner` - 132 edges
3. `UWorldAmbienceSubsystem` - 126 edges
4. `UHttpGatewaySubsystem` - 116 edges
5. `ACheckpoint` - 110 edges
6. `APoi` - 107 edges
7. `APath` - 103 edges
8. `FRaceSetup` - 71 edges
9. `USettingsSubsystem` - 71 edges
10. `FWorldUtils` - 61 edges

## Surprising Connections (you probably didn't know these)
- `UTemplateGeneratorUtilityWidget` --references--> `TSubclassOf`  [EXTRACTED]
  Plugins/TemplateGenerator/Source/TemplateGenerator/Public/TemplateGeneratorUtilityWidget.h → Source/TrailUtils/Public/WorldUtils.h
- `UClubComponent::UpdateClub()` --references--> `FRunnerStruct`  [EXTRACTED]
  Source/TrailSimulator/Private/Components/ClubComponent.cpp → Source/SharedTypes/Public/TrailSharedTypes.h
- `UGTWS_ClubComponent::UpdateClub()` --references--> `FRunnerStruct`  [EXTRACTED]
  Source/TrailSimulator/Private/Components/GTWS/GTWS_ClubComponent.cpp → Source/SharedTypes/Public/TrailSharedTypes.h
- `UNike_ClubComponent::UpdateClub()` --references--> `FRunnerStruct`  [EXTRACTED]
  Source/TrailSimulator/Private/Components/Nike/Nike_ClubComponent.cpp → Source/SharedTypes/Public/TrailSharedTypes.h
- `UScaleSubsystem::SetGlobalConfig()` --references--> `FDistanceScaleConfig`  [EXTRACTED]
  Source/Subsystems/Private/ScaleSubsystem.cpp → Source/SharedTypes/Public/TrailSharedTypes.h

## Import Cycles
- None detected.

## Communities (527 total, 368 thin omitted)

### Community 0 - "ARaceManager"
Cohesion: 0.01
Nodes (70): ARaceManager, AllRaces, bAfficher, bIsDay, bOverlayHidden, bSRTEncodeAudio, bStartSRTOnBeginPlay, CheckpointSubsystem (+62 more)

### Community 1 - "UHttpGatewaySubsystem"
Cohesion: 0.02
Nodes (56): UHttpGatewaySubsystem, CommandRouteHandle, HttpRouter, ListenPort, OnAddTeamToGroup, OnArmLengthUpdated, OnCameraAngleUpdated, OnCheckpointAnimation (+48 more)

### Community 2 - ".SlateNotify"
Cohesion: 0.04
Nodes (74): ARaceManager::EdAddTeamToGroup(), ARaceManager::EdAnimCheckpoint(), ARaceManager::EdAnimPoi(), ARaceManager::EdAnimTeam(), ARaceManager::EdChangePathOffset(), ARaceManager::EdMoveToPath(), ARaceManager::EdPauseTravel(), ARaceManager::EdRefreshRace() (+66 more)

### Community 3 - "ARunner"
Cohesion: 0.02
Nodes (59): ARunner, ActiveState, ArmLength, bHasDayNight, bHasStackChildren, bHasTrackDist, bLastIsDay, bLookAtEnabled (+51 more)

### Community 4 - "RaceManager.cpp"
Cohesion: 0.05
Nodes (64): ARaceManager::AddTeamToGroup_Internal(), ARaceManager::AnimCheckpoint_Internal(), ARaceManager::AnimPoi_Internal(), ARaceManager::AnimTeam_Internal(), ARaceManager::ApplyPathSegments(), ARaceManager::ChangePathOffset_Internal(), ARaceManager::EdStopAnimTeam(), ARaceManager::EdToggleRace() (+56 more)

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
Nodes (56): UWorldAmbienceSubsystem::UpdateState(), UWorldAmbienceSubsystem, AppliedWeather, AutoWeather, bBegunPlay, bBenchGrounded, BenchCsvPath, BenchFrameSamples (+48 more)

### Community 9 - "USettingsSubsystem"
Cohesion: 0.03
Nodes (24): USettingsSubsystem, CachedSave, LookAtInterpSpeed, LookAtIntervalSeconds, OnFetchUpdate, OnLengthChanged, OnMinMaxDirty, OnMinMaxSet (+16 more)

### Community 10 - "WorldAmbienceSubsystem.cpp"
Cohesion: 0.05
Nodes (20): Average(), ExportProperty(), ImportProperty(), JsonResponse(), Percentile(), SettingsFilePath(), UWorldAmbienceSubsystem::ApplySetting(), UWorldAmbienceSubsystem::BenchJson() (+12 more)

### Community 11 - "UCheckpointSubsystem"
Cohesion: 0.06
Nodes (9): UCheckpointSubsystem, ActiveCheckpointRequests, Checkpoint, CheckpointActorMap, Checkpoints, CheckpointsMap, CurrentRaceSetup, OnCheckpointsDatasGathered (+1 more)

### Community 12 - "modulemanager"
Cohesion: 0.04
Nodes (6): FSlateNotificationsModule, FTemplateGeneratorModule, FSharedTypesModule, FSubsystemsModule, FTrailInterfacesModule, FTrailUtilsModule

### Community 13 - "URunnerSubsystem"
Cohesion: 0.04
Nodes (16): URunnerSubsystem, ActiveRunnerRequests, CurrentRaceSetup, CurrentRunnerId, MaxRunnersFetchAttempts, OnRunnerDatasGathered, OnRunnersDatasGathered, OnRunnersUpdateDatas (+8 more)

### Community 14 - "FRunnerStruct"
Cohesion: 0.07
Nodes (28): FGroupStruct, groupId, Teams, FRaceTeamRunnerIndex, RunnerToTeam, TeamToRunner, FRunnerStruct, canal (+20 more)

### Community 15 - "Path.cpp"
Cohesion: 0.08
Nodes (20): APath::BeginPlay(), APath::BuildKms(), APath::DrawPath(), APath::GetClosestSplineLocation(), APath::GetDistanceAlongSpline(), APath::GetLocationAtDistance(), APath::HandleCheckpointsDatasGathered(), APath::RebuildPathSplineMeshes() (+12 more)

### Community 16 - "Runner.cpp"
Cohesion: 0.05
Nodes (9): ARunner::AssignRunnerToTeam(), ARunner::GetStackAttachComponent(), ARunner::GetStackHookComponent(), ARunner::HandleTemplateChanged(), ARunner::SetMinMax(), ARunner::TriggerUpdateAfterHidden(), ARunner::UpdateRunnerLocation(), UCesiumFlyToComponent (+1 more)

### Community 17 - "UNameComponent"
Cohesion: 0.08
Nodes (11): UNameComponent, bShow3DText, bShowWidgetText, LabelFont, LastLocation, NameBkgComponent, NameFont, NameSplineComponent (+3 more)

### Community 18 - "SearchBox"
Cohesion: 0.13
Nodes (7): createResults(), init_search(), SearchBox(), getXPos(), getYPos(), searchResults, convertToId()

### Community 19 - "Checkpoint_GTWS.cpp"
Cohesion: 0.07
Nodes (16): ACheckpoint_Generic::UpdateCheckpoint(), UseRoundedBkg_CheckpointGeneric(), APoi_Generic::UpdatePoi(), UseRoundedBkg_PoiGeneric(), ACheckpoint_GTWS::BeginPlay(), ACheckpoint_GTWS::UpdateCheckpoint(), UseRoundedBkg_CheckpointGTWS(), APoi_GTWS::BeginPlay() (+8 more)

### Community 20 - "AWorldFauna"
Cohesion: 0.06
Nodes (25): AWorldFauna::Tick(), AWorldFauna, Ambience, Anchor, AnchorGroundZ, AnchorTimer, AppliedDensity, bHasAnchor (+17 more)

### Community 21 - "ULoadingStatusSubsystem"
Cohesion: 0.10
Nodes (6): ULoadingStatusSubsystem::MarkDirty(), ULoadingStatusSubsystem, bDirty, FlushHandle, OnChanged, Tasks

### Community 22 - "jquery.js"
Cohesion: 0.08
Nodes (13): at(), getEventsNS(), getViewportDimensions(), initMouseDetection(), initTracking(), isTouchEvent(), lt(), trackMouse() (+5 more)

### Community 23 - "URaceSubsystem"
Cohesion: 0.06
Nodes (12): UCheckpointSubsystem::Initialize(), UPoiSubsystem::Initialize(), URunnerSubsystem::Initialize(), UTeamSubsystem::Initialize(), URaceSubsystem, CurrentRaceId, CurrentRaceSetup, MainURL (+4 more)

### Community 24 - "UPoiSubsystem"
Cohesion: 0.06
Nodes (10): UPoiSubsystem, ActivePoiRequests, CurrentRaceSetup, OnPoiDatasGathered, OnPoisDatasGathered, POI, POIs, PoisActorsMap (+2 more)

### Community 25 - "AWorldFlora"
Cohesion: 0.06
Nodes (21): AWorldFlora, AllComponents, Ambience, bBuilding, bShown, BuildCenter, BuildDensity, BuildRadius (+13 more)

### Community 26 - "Checkpoint.cpp"
Cohesion: 0.04
Nodes (5): ACheckpoint::CapitalizeFirst(), ACheckpoint::GetBoundsLocation(), ACheckpoint::HandleTemplateChanged(), ACheckpoint::SetMinMax(), UCameraControlComponent::TickComponent()

### Community 27 - "UFlagComponent"
Cohesion: 0.05
Nodes (15): UFlagComponent::UFlagComponent(), UFlagComponent::UpdateDayNight(), UFlagComponent::UpdateFlag(), UGeneric_FlagComponent::UGeneric_FlagComponent(), UGTWS_FlagComponent::UGTWS_FlagComponent(), UNike_FlagComponent::UNike_FlagComponent(), UFlagComponent, bIsDayCached (+7 more)

### Community 28 - "UWaterMaskSubsystem"
Cohesion: 0.08
Nodes (10): FWaterPolygon, Bounds, Rings, UWaterMaskSubsystem, ActiveRequests, bWaterPolygonsDirty, MaterialLayerKey, PathSubsystem (+2 more)

### Community 29 - "UAudioSanitizerSubsystem"
Cohesion: 0.06
Nodes (19): UAudioSanitizerSubsystem, bCulpritRetrying, BeginPlayTime, bSoundOpened, CheckTimer, ClippedSinceLog, Culprit, CulpritRetryDelay (+11 more)

### Community 30 - ".GetWorld"
Cohesion: 0.08
Nodes (32): URunnerSubsystem::CancelAllRunnersRetries(), UWaterMaskSubsystem::ApplyToTilesets(), UWorldAmbienceSubsystem::DoesSupportWorldType(), Type, ITrailInputModeInterface, ACheckpoint::ACheckpoint(), ACheckpoint::BeginPlay(), ACheckpoint::EndPlay() (+24 more)

### Community 31 - "Poi.cpp"
Cohesion: 0.05
Nodes (6): UGameViewportClient, UTextureRenderTarget2D, APoi::CapitalizeFirst(), APoi::GetBoundsLocation(), APoi::HandleTemplateChanged(), APoi::SetMinMax()

### Community 32 - "gameplaystatics"
Cohesion: 0.09
Nodes (7): ACheckpoint_UTMB::BeginPlay(), ACheckpoint_UTMB::UpdateCheckpoint(), APoi_UTMB::BeginPlay(), APoi_UTMB::UpdatePoi(), ARunner_UTMB::BeginPlay(), UUTMB_FlagComponent::UUTMB_FlagComponent(), UUTMB_FlagComponent

### Community 33 - "LoadingStatusSubsystem.cpp"
Cohesion: 0.30
Nodes (6): Clamp01(), ULoadingStatusSubsystem::Complete(), ULoadingStatusSubsystem::Fail(), ULoadingStatusSubsystem::RegisterTask(), ULoadingStatusSubsystem::SetRunning(), ULoadingStatusSubsystem::Update()

### Community 34 - "UPathSubsystem"
Cohesion: 0.07
Nodes (8): UPathSubsystem, ActivePathRequests, CurrentRaceSetup, MapPaths, OnPathDatasGathered, RacePath, RacePathsMap, RaceSubsystem

### Community 35 - "SettingsSubsystem.cpp"
Cohesion: 0.14
Nodes (18): USettingsSubsystem::ApplyGeoidCorrectionById(), USettingsSubsystem::CreateTrailSettingsById(), USettingsSubsystem::DoesSettingsExist(), USettingsSubsystem::GetFetchById(), USettingsSubsystem::GetGlowById(), USettingsSubsystem::GetMinMaxById(), USettingsSubsystem::GetPulseFrequencyById(), USettingsSubsystem::GetTrailSettingsById() (+10 more)

### Community 36 - "Templates Unreal (TrailSimulator) servis par OnGraphics"
Cohesion: 0.11
Nodes (18): 1. Le contexte en deux minutes, 2. Réponse à « conventions Figma ou JSON de template ? », 3.1 Routes attendues sur OnGraphics, 3.2 Format (version 1), 3.3 Règles de valeur, 3.4 Ce qu'Unreal ne lit pas (inutile de l'envoyer), 3. Le contrat : `GET /unreal/template/{nom}`, 4.1 Trois graphics de plus (+10 more)

### Community 37 - "SLoadingOverlay"
Cohesion: 0.09
Nodes (13): SLoadingOverlay::Construct(), SLoadingOverlay::Tick(), SLoadingOverlay, bDirty, CardsBox, ChangedHandle, LastRebuildTime, LoadingSubsystem (+5 more)

### Community 38 - "URunnerStackingSubsystem"
Cohesion: 0.09
Nodes (9): URunnerStackingSubsystem::TickSubsystem(), URunnerStackingSubsystem, bDirty, bHasLastCamLoc, CameraMoveThresholdCm, Config, Entries, LastCamLoc (+1 more)

### Community 39 - "UTeamSubsystem"
Cohesion: 0.08
Nodes (6): UTeamSubsystem, CurrentRaceSetup, HttpGatewaySubsystem, RaceSubsystem, RaceTeamsRunnersMap, RunnerSubsystem

### Community 40 - "UWeatherSubsystem"
Cohesion: 0.08
Nodes (12): FWeatherResult, bSuccess, Data, Error, HttpCode, RequestKey, UWeatherSubsystem, ActiveWeatherRequests (+4 more)

### Community 41 - "AKm"
Cohesion: 0.05
Nodes (17): IKmInterface, APath::GetKmClassForRace(), AKm_Generic, AKm_GTWS, AKm, DynaPawn, Georeference, GlobeAnchorComponent (+9 more)

### Community 42 - "FRaceSetup"
Cohesion: 0.09
Nodes (26): FRaceSetup, arrivalCity, arrivalCountry, arrivalCountryFlag, cat, color, departureCity, departureCountry (+18 more)

### Community 43 - "unrealbuildtool"
Cohesion: 0.11
Nodes (9): SlateNotifications, TemplateGenerator, SharedTypes, Subsystems, TrailInterfaces, TrailSimulatorTarget, TrailSimulator, TrailSimulatorEditorTarget (+1 more)

### Community 44 - "UPROPERTY"
Cohesion: 0.09
Nodes (22): FCheckpoints, Checkpoints, FGroups, Groups, RaceId, FPOIs, POIs, FRaceEntries (+14 more)

### Community 45 - "UBroadcastCaptureSubsystem"
Cohesion: 0.08
Nodes (10): UBroadcastCaptureSubsystem, Accum, ActiveSource, CachedPC, DefaultSource, MirrorViewport, MirrorWidget, MonitorHz (+2 more)

### Community 46 - "FAutoWeatherInfo"
Cohesion: 0.08
Nodes (24): FAutoWeatherInfo, AgeMinutes, bHasData, CameraAltitudeM, CameraTempC, CloudBaseM, Clouds, Description (+16 more)

### Community 47 - "UClubComponent"
Cohesion: 0.08
Nodes (11): UClubComponent, bShow3DText, bShowWidgetText, ClubBkgComponent, ClubFont, ClubSplineComponent, ClubTextComponent, LastLocation (+3 more)

### Community 48 - "FSettings"
Cohesion: 0.10
Nodes (19): FOpenWeatherCondition, description, icon, id, main, FSettings, bGeoidCorrected, FetchFrequency (+11 more)

### Community 49 - "RunnerSubsystem.cpp"
Cohesion: 0.18
Nodes (16): URunnerSubsystem::Animation(), URunnerSubsystem::AnimationByTeam(), URunnerSubsystem::CancelRunnersRetry(), URunnerSubsystem::DoesRunnerBelongsToRace(), URunnerSubsystem::GetRunnerActor(), URunnerSubsystem::GetRunnerActorByTeam(), URunnerSubsystem::GetRunnersByRaceId(), URunnerSubsystem::RemoveTeamsNotInSnapshot() (+8 more)

### Community 50 - "UTrailTextWidgetComponent"
Cohesion: 0.09
Nodes (7): UTrailTextWidgetComponent, CurrentColor, CurrentFont, CurrentJustification, CurrentSize, CurrentText, SlateTextBlock

### Community 51 - "FOpenWeatherCurrent"
Cohesion: 0.08
Nodes (25): FOpenWeatherCurrent, clouds, dew_point, dt, feels_like, humidity, pressure, rain_1h (+17 more)

### Community 52 - "UScaleSubsystem"
Cohesion: 0.08
Nodes (4): UScaleSubsystem::Tick(), UScaleSubsystem, Entries, GlobalConfig

### Community 53 - "FWorldAmbienceSettings"
Cohesion: 0.08
Nodes (23): UWorldAmbienceSubsystem::SetSettings(), FWorldAmbienceSettings, AutoWeatherRefreshMinutes, bAutoWeather, bCityLights, bFauna, bFlora, bHeadlamps (+15 more)

### Community 54 - "TrailHttpDebug.h"
Cohesion: 0.18
Nodes (6): FTrailHttpDebug, FTrailHttpDebugOptions, bLogBody, bLogHeaders, bSkipBodyIfBinary, MaxBodyChars

### Community 55 - "PoiSubsystem.cpp"
Cohesion: 0.33
Nodes (8): UPoiSubsystem::Animation(), UPoiSubsystem::DoesPoiBelongsToRace(), UPoiSubsystem::SetRaceSetup(), UPoiSubsystem::StopAnimation(), UPoiSubsystem::Teleport(), UPoiSubsystem::TogglePoi(), UPoiSubsystem::TogglePois(), UPoiSubsystem::ToggleWeather()

### Community 56 - "ScaleSubsystem.cpp"
Cohesion: 0.11
Nodes (10): UScaleSubsystem::ComputeTargetScale(), UScaleSubsystem::GetCameraLocation(), UScaleSubsystem::GetStatId(), UScaleSubsystem::HandleActorEndPlay(), UScaleSubsystem::Initialize(), UScaleSubsystem::IsTickable(), UScaleSubsystem::RegisterScalableActor(), UScaleSubsystem::SetActorScaleMinMax() (+2 more)

### Community 57 - "ARunner_UTMB"
Cohesion: 0.10
Nodes (15): ARunner_UTMB, FlagComponent, FlagHook, IndexBarreComponent, IndexBkgComponent, IndexBkgHook, IndexLabelComponent, IndexLeftComponent (+7 more)

### Community 58 - "AudioSanitizer.cpp"
Cohesion: 0.13
Nodes (8): DayPhase(), FSubmixEffectSanitizer::OnProcessAudio(), SetWorldMute(), UAudioSanitizerSubsystem::CheckNaN(), UAudioSanitizerSubsystem::Deinitialize(), UAudioSanitizerSubsystem::Initialize(), UAudioSanitizerSubsystem::OnWorldBeginPlay(), UAudioSanitizerSubsystem::UpdateForest()

### Community 59 - "FRaceCheckpoint"
Cohesion: 0.13
Nodes (14): FRaceCheckpoint, altitude, checkpointId, distance, elevation, name, type, weather (+6 more)

### Community 61 - "TrailTextWidgetComponent.cpp"
Cohesion: 0.12
Nodes (9): UTrailTextWidgetComponent::GetFontInfo(), UTrailTextWidgetComponent::OnRegister(), UTrailTextWidgetComponent::SetLabelCasseToLower(), UTrailTextWidgetComponent::SetLabelCasseToUpper(), UTrailTextWidgetComponent::SetLabelColor(), UTrailTextWidgetComponent::SetLabelFont(), UTrailTextWidgetComponent::SetLabelHJustification(), UTrailTextWidgetComponent::SetLabelText() (+1 more)

### Community 62 - "WorldFlora.cpp"
Cohesion: 0.13
Nodes (5): AWorldFlora::HandlePathGathered(), AWorldFlora::IsOnTrail(), AWorldFlora::StartBuild(), AWorldFlora::TraceGround(), Hash()

### Community 63 - "SLoadingOverlay.cpp"
Cohesion: 0.12
Nodes (8): SWidget, Font(), MakePill(), MakeStep(), Rounded(), SLoadingOverlay::GetOverallPercent(), SLoadingOverlay::Rebuild(), StateColor()

### Community 64 - "CheckpointSubsystem.cpp"
Cohesion: 0.17
Nodes (15): ConvertCheckpointJson(), ConvertCheckpointsJson(), UCheckpointSubsystem::Animation(), UCheckpointSubsystem::DoesChkBelongsToRace(), UCheckpointSubsystem::GetCheckpointActor(), UCheckpointSubsystem::GetCheckpointByChkIdRaceId(), UCheckpointSubsystem::GetCheckpointsByRaceId(), UCheckpointSubsystem::PerformHttpRequestForCheckpoints() (+7 more)

### Community 66 - "ARunner_GTWS"
Cohesion: 0.11
Nodes (11): ARunner_GTWS, ClubComponent, ClubHook, DefaultClubLocation, DefaultNameLocation, FlagComponent, FlagHook, NameComponent (+3 more)

### Community 67 - "Templates Unreal (TrailSimulator) servis par OnGraphics"
Cohesion: 0.11
Nodes (18): 1. Le contexte en deux minutes, 2. Réponse à « conventions Figma ou JSON de template ? », 3.1 Routes attendues sur OnGraphics, 3.2 Format (version 1), 3.3 Règles de valeur, 3.4 Ce qu'Unreal ne lit pas (inutile de l'envoyer), 3. Le contrat : `GET /unreal/template/{nom}`, 4.1 Trois graphics de plus (+10 more)

### Community 68 - "BroadcastCaptureSubsystem.cpp"
Cohesion: 0.13
Nodes (6): UBroadcastCaptureSubsystem::ActivateSource(), UBroadcastCaptureSubsystem::Deinitialize(), UBroadcastCaptureSubsystem::Initialize(), UBroadcastCaptureSubsystem::ResolveCaptureSourceFromViewTarget(), UBroadcastCaptureSubsystem::UpdateViewportMirror(), IBroadcastInterface

### Community 69 - "TemplateSubsystem.h"
Cohesion: 0.05
Nodes (42): FTrailTemplate, Base, Checkpoint, Globals, Name, Poi, Runner, FTrailTemplateGlobals (+34 more)

### Community 70 - "FLoadingTaskInfo"
Cohesion: 0.11
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

### Community 77 - "UPhotoComponent"
Cohesion: 0.13
Nodes (7): UPhotoComponent, BaseMat, CurrentPhotoTexture, CurrentPhotoUrl, PendingPhotoUrl, PhotoMID, PhotoSMC

### Community 78 - "FMinMax"
Cohesion: 0.14
Nodes (12): FMinMax, Max, Min, FStartAndEnd, EndLocation, StartLocation, ACheckpoint::GetMinMax(), APoi::GetMinMax() (+4 more)

### Community 79 - "APoi_GTWS"
Cohesion: 0.14
Nodes (9): APoi_GTWS, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, RaceManager, WeatherPictoMat (+1 more)

### Community 80 - "AWorldHeadlamps"
Cohesion: 0.08
Nodes (11): AWorldHeadlamps::BeginPlay(), AWorldHeadlamps::GetLamp(), AWorldHeadlamps::Tick(), AWorldHeadlamps, Ambience, Lamps, LitCount, RefreshTimer (+3 more)

### Community 81 - "APoi_Nike"
Cohesion: 0.12
Nodes (13): APoi_Nike, Infos1BkgComponent, Infos1SplineComponent, Infos1WidgetComponent, Infos2BkgComponent, Infos2SplineComponent, Infos2WidgetComponent, InfosFont (+5 more)

### Community 82 - "JsonResponse"
Cohesion: 0.27
Nodes (6): FindGeoreference(), GeoJson(), JsonResponse(), UMeteoPageSubsystem::HandleGeoRequest(), UMeteoPageSubsystem::HandleGeoSetRequest(), UMeteoPageSubsystem::HandlePageRequest()

### Community 83 - "UPathSegmentsComponent"
Cohesion: 0.07
Nodes (16): UPathSegmentsComponent::Apply(), UPathSegmentsComponent::RestoreTrace(), UPathSegmentsComponent, ExtraPieces, HighlightedPieces, LabelHeight, LabelIllum, LabelPadding (+8 more)

### Community 84 - "Ee"
Cohesion: 0.16
Nodes (16): A(), b(), be(), Ee(), f(), he(), je(), k() (+8 more)

### Community 85 - "e"
Cohesion: 0.19
Nodes (16): ce(), e(), fe(), H(), I(), l(), le(), M() (+8 more)

### Community 86 - "PathSubsystem.cpp"
Cohesion: 0.22
Nodes (8): ConvertPathJson(), UPathSubsystem::GetPathById(), UPathSubsystem::Initialize(), UPathSubsystem::PerformHttpRequestForPath(), UPathSubsystem::SetRaceSetup(), UPathSubsystem::SpawnKmActor(), UPathSubsystem::SpawnPathActor(), TSubclassOf

### Community 87 - "ARaceManager::SpawnInitialRunners"
Cohesion: 0.31
Nodes (4): ARaceManager::ProcessPendingRunnerUpdates(), ARaceManager::SpawnInitialRunners(), ARaceManager::UpdateRunnersFromSnapshot(), LogRaceTeamIndex()

### Community 88 - "GTWS_NameComponent.cpp"
Cohesion: 0.15
Nodes (4): UGTWS_NameComponent::GetSplineLastPoint(), UGTWS_NameComponent::SetColors(), UGTWS_NameComponent::UGTWS_NameComponent(), UGTWS_NameComponent::UpdateName()

### Community 89 - "RunnerStackingSubsystem.cpp"
Cohesion: 0.11
Nodes (13): URunnerStackingSubsystem::ApplyNewStackingState(), URunnerStackingSubsystem::AttachRunnerToBase(), URunnerStackingSubsystem::ComputeDynamicRadiusCm(), URunnerStackingSubsystem::ComputeOverlapRadiusCm(), URunnerStackingSubsystem::Deinitialize(), URunnerStackingSubsystem::DetachRunnerToTrack(), URunnerStackingSubsystem::FindEntryIndex(), URunnerStackingSubsystem::HandleActorEndPlay() (+5 more)

### Community 90 - "APoi_Generic"
Cohesion: 0.12
Nodes (11): APoi_Generic, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, NameLeftComponent, NameRightComponent (+3 more)

### Community 91 - "SBroadcastMirror"
Cohesion: 0.14
Nodes (5): SBroadcastMirror, Background, bLogged, Brush, Target

### Community 92 - "TeamSubsystem.cpp"
Cohesion: 0.18
Nodes (15): UCesiumFlyToComponent, UTeamSubsystem::Animation(), UTeamSubsystem::ClearRaceTeamIndex(), UTeamSubsystem::DisplayClub(), UTeamSubsystem::DisplayFlag(), UTeamSubsystem::DisplayPhoto(), UTeamSubsystem::DisplayTeam(), UTeamSubsystem::DisplayTeams() (+7 more)

### Community 94 - "UCameraControlComponent"
Cohesion: 0.15
Nodes (4): UCameraControlComponent, CameraComponent, OwlCapture, SpringArmComponent

### Community 95 - "UMeteoPageSubsystem"
Cohesion: 0.11
Nodes (8): FHttpServerRequest, IHttpRouter, UMeteoPageSubsystem, GeoRouteHandle, GeoSetRouteHandle, HttpRouter, ListenPort, PageRouteHandle

### Community 96 - "UTeamGroupSubsystem"
Cohesion: 0.13
Nodes (5): UTeamGroupSubsystem, CurrentRaceSetup, RaceGroupTeamMap, RaceSubsystem, TeamSubsystem

### Community 97 - "NameComponent.cpp"
Cohesion: 0.15
Nodes (6): UNameComponent::GetLocalSize(), UNameComponent::GetSplineComponent(), UNameComponent::GetWorldLastPointLocation(), UNameComponent::UNameComponent(), UNameComponent::UpdateMesh(), UNameComponent::UpdateName()

### Community 98 - "ACheckpoint_GTWS"
Cohesion: 0.13
Nodes (10): ACheckpoint_GTWS, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID, RaceManager (+2 more)

### Community 99 - "ACheckpoint_UTMB"
Cohesion: 0.12
Nodes (11): APath::GetCheckpointClassForRace(), ACheckpoint_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID (+3 more)

### Community 100 - "UTemplateSubsystem"
Cohesion: 0.11
Nodes (6): UTemplateSubsystem, ActiveRequests, OnTemplateChanged, Templates, ARunner::UpdateRunner(), ARaceManager::PrepareRunnerRuntimeData()

### Community 101 - "TemplateSubsystem.cpp"
Cohesion: 0.18
Nodes (8): UTemplateSubsystem::Fetch(), UTemplateSubsystem::GetBase(), UTemplateSubsystem::GetBuiltin(), UTemplateSubsystem::GetForRace(), UTemplateSubsystem::GetRunnerHeight(), UTemplateSubsystem::GetTemplate(), UTemplateSubsystem::ReloadTemplate(), UTemplateSubsystem::RequestTemplate()

### Community 102 - "OverpassToFeatures"
Cohesion: 0.26
Nodes (8): BufferLine(), IsClosed(), LineWidthMeters(), OverpassToFeatures(), PointInRing(), ReadGeometry(), RingToJson(), StitchRings()

### Community 103 - "TrailsSubsystem.cpp"
Cohesion: 0.20
Nodes (4): ConvertRacesJson(), UTrailsSubsystem::Initialize(), UTrailsSubsystem::PerformHttpRequestForRaces(), UTrailsSubsystem::SetRaceSetup()

### Community 104 - "FPathSegment"
Cohesion: 0.25
Nodes (7): FPathSegment, bVisible, Color, KmIn, KmOut, Label, UPathSegmentsComponent::SetSegments()

### Community 105 - "WorldAmbienceSubsystem.h"
Cohesion: 0.07
Nodes (26): UWorldAmbienceSubsystem::FindSunLight(), FBenchResult, FrameAvg, FrameP99, GpuAvg, GpuP99, Name, Samples (+18 more)

### Community 106 - "UObject"
Cohesion: 0.16
Nodes (14): FindPlayWorld(), FindActorByClassToken(), ReadNumber(), UAudioSanitizerSubsystem::ShouldCreateSubsystem(), FWorldUtils::GetGameInstance(), FWorldUtils::GetGameInstanceChecked(), FWorldUtils::GetRuntimeWorldFallback(), FWorldUtils::GetWorld() (+6 more)

### Community 107 - "ARunner_Generic"
Cohesion: 0.14
Nodes (7): ARunner_Generic, FlagComponent, NameComponent, PhotoComponent, PhotoHook, RaceManager, SlashComponent

### Community 108 - "APoi_UTMB"
Cohesion: 0.12
Nodes (12): AWorldFauna::BuildMeshes(), UMaterialInstanceDynamic, UPhotoComponent::EnsurePhotoMID(), APoi_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat (+4 more)

### Community 109 - "GTWS_ClubComponent.cpp"
Cohesion: 0.22
Nodes (3): UGTWS_ClubComponent::SetColors(), UGTWS_ClubComponent::UGTWS_ClubComponent(), UGTWS_ClubComponent::UpdateClub()

### Community 110 - "PathSegmentsComponent.cpp"
Cohesion: 0.07
Nodes (8): AKm::AKm(), AKm::BeginPlay(), AKm::OnLookAtTimerTick(), AKm::UpdateKm(), UPathSegmentsComponent::BeginPlay(), UPathSegmentsComponent::ConfigurePiece(), UPathSegmentsComponent::CreatePiece(), UPathSegmentsComponent::OnLookAtTimerTick()

### Community 111 - "UGeneric_NameComponent"
Cohesion: 0.22
Nodes (3): UGeneric_NameComponent, NameLeftComponent, NameRightComponent

### Community 112 - "showTip"
Cohesion: 0.28
Nodes (13): countFlags(), CSSCoordinates(), getTooltipContent(), getViewportCollisions(), isMouseOver(), TooltipController(), beginShowTip(), closeDesyncedTip() (+5 more)

### Community 113 - "UTemplateGeneratorUtilityWidget"
Cohesion: 0.15
Nodes (4): UTemplateGeneratorUtilityWidget, CheckpointParentClass, PoiParentClass, TeamParentClass

### Community 114 - "PhotoComponent.cpp"
Cohesion: 0.19
Nodes (3): UPhotoComponent::ClearPhoto(), UPhotoComponent::UpdateDayNight(), UPhotoComponent::UpdatePhoto()

### Community 115 - "UTrailsSubsystem"
Cohesion: 0.15
Nodes (6): UTrailsSubsystem, CurrentRaceSetup, LoadingSubsystem, OnRacesDatasGathered, RacesArray, RaceSubsystem

### Community 116 - "WeatherSubsystem.cpp"
Cohesion: 0.20
Nodes (5): ConvertWeatherJson(), UWeatherSubsystem::Deinitialize(), UWeatherSubsystem::Initialize(), UWeatherSubsystem::PerformHttpRequestForWeather(), UWeatherSubsystem::SetRaceSetup()

### Community 117 - "FTrailHttpDebug::LogAndIsSuccess"
Cohesion: 0.27
Nodes (3): FTrailHttpDebug::LogAndIsSuccess(), FTrailHttpDebug::LogStart(), IsLikelyBinaryContent()

### Community 118 - "USettingsSaveGame"
Cohesion: 0.15
Nodes (7): USettingsSubsystem::Load(), USettingsSubsystem::SetMainURL(), USettingsSaveGame, GlobalLength, GlobalPitch, GlobalZAnchor, MainURL

### Community 119 - "FRacePOI"
Cohesion: 0.20
Nodes (9): FRacePOI, elevation, lat, lon, name, poiId, type, weather (+1 more)

### Community 121 - "FBenchShot"
Cohesion: 0.09
Nodes (20): UWorldAmbienceSubsystem::ApplyBenchShot(), UWorldAmbienceSubsystem::PlaceCameraOnGround(), FBenchShot, AboveGroundM, HeightM, Lat, Lon, Name (+12 more)

### Community 122 - "FSubmixEffectSanitizer"
Cohesion: 0.17
Nodes (5): FSubmixEffectSanitizer, BadSamples, ClippedSamples, CurrentGain, TargetGain

### Community 123 - "FDistanceScaleConfig"
Cohesion: 0.17
Nodes (11): FDistanceScaleConfig, bInterp, bSmoothStep, bUseActorOriginalScaleAsMax, bUseXYOnly, FarDistanceCm, InterpSpeed, NearDistanceCm (+3 more)

### Community 124 - "FStackEntry"
Cohesion: 0.11
Nodes (16): FEntry, Actor, CurrentMinMax, OriginalScale, OverrideConfig, FStackEntry, bClubVisible, bPhotoVisible (+8 more)

### Community 125 - "splinemeshcomponent"
Cohesion: 0.06
Nodes (14): ACheckpoint_Nike::UpdateCheckpoint(), APoi_Nike::UpdatePoi(), UClubComponent::GetLocalSize(), UClubComponent::UClubComponent(), UClubComponent::UpdateClub(), UClubComponent::UpdateMesh(), UGeneric_NameComponent::GetMiddle(), UGeneric_NameComponent::UGeneric_NameComponent() (+6 more)

### Community 126 - "CallBP_GetFloatOut"
Cohesion: 0.18
Nodes (7): ARaceManager::EdUpdateMainUrl(), ARaceManager::HandleRacesDatasGathered(), ARaceManager::RtUpdateMainUrl(), ARaceManager::UpdateMainUrl_Internal(), ARaceManager::UpdateUdsTime(), CallBP_GetFloatOut(), SanitizeKey()

### Community 128 - "UTMB_NameComponent.cpp"
Cohesion: 0.22
Nodes (3): UUTMB_NameComponent::UpdateMesh(), UUTMB_NameComponent::UpdateName(), UUTMB_NameComponent::UUTMB_NameComponent()

### Community 129 - "RawTextMaterial.h"
Cohesion: 0.24
Nodes (4): Apply(), ApplyToAllTexts(), EnableOcclusion(), GetMaterial()

### Community 131 - "FRunnerStackingConfig"
Cohesion: 0.18
Nodes (8): FRunnerStackingConfig, Gamma, HysteresisCm, RadiusFarCm, RadiusNearCm, UpdateIntervalSec, URunnerStackingSubsystem::Initialize(), URunnerStackingSubsystem::SetConfig()

### Community 132 - "UGTWS_ClubComponent"
Cohesion: 0.22
Nodes (3): UGTWS_ClubComponent, AppliedBkgColor, TextColor

### Community 134 - "FRaceCard"
Cohesion: 0.18
Nodes (5): FRaceCard, RaceId, Steps, Title, SplitTaskId()

### Community 135 - "UGTWS_NameComponent"
Cohesion: 0.17
Nodes (3): UGTWS_NameComponent, AppliedBkgColor, TextColor

### Community 137 - "UWaterMaskSubsystem::HandlePathDatasGathered"
Cohesion: 0.20
Nodes (4): UWaterMaskSubsystem::HandlePathDatasGathered(), UWaterMaskSubsystem::OnRaceWaterReady(), UWaterMaskSubsystem::RequestOverpass(), UWaterMaskSubsystem::SendOverpassRequest()

### Community 138 - "FString"
Cohesion: 0.27
Nodes (6): ConvertRunnerJson(), ConvertRunnersJson(), URunnerSubsystem::PerformHttpRequestForRunner(), URunnerSubsystem::PerformHttpRequestForRunners(), URunnerSubsystem::RequestRunnersWithRetry(), URunnerSubsystem::ScheduleRunnersRetry()

### Community 139 - "int64"
Cohesion: 0.15
Nodes (12): FRaceEntry, name, raceId, templateName, FTeams, RaceId, Teams, FTeamStruct (+4 more)

### Community 141 - "FWaterMaskBounds"
Cohesion: 0.29
Nodes (7): UWaterMaskSubsystem::ComputeBounds(), UWaterMaskSubsystem::GetCacheFilePath(), FWaterMaskBounds, East, North, South, West

### Community 143 - "UUTMB_NameComponent"
Cohesion: 0.20
Nodes (3): UUTMB_NameComponent, RightSlashComponent, SplineComponent

### Community 144 - "WorldFlora.h"
Cohesion: 0.22
Nodes (5): FKindSet, Components, UHierarchicalInstancedStaticMeshComponent, UStaticMesh, UWorldAmbienceSubsystem

### Community 145 - "ARunner_Nike"
Cohesion: 0.22
Nodes (7): ARunner_Nike, ClubComponent, ClubFont, ClubHook, FlagComponent, LabelFont, NameComponent

### Community 146 - "AudioSanitizer.h"
Cohesion: 0.17
Nodes (7): SoundName(), ENaNHuntPhase, Done, Idle, Stop, UAudioComponent, USubmixEffectSanitizerPreset

### Community 147 - "BroadcastSettings.h"
Cohesion: 0.25
Nodes (3): UBroadcastSettings, bMirrorCaptureToViewport, UTextureRenderTarget2D

### Community 150 - "DisplayController"
Cohesion: 0.54
Nodes (7): DisplayController(), cancelClose(), cancelTimer(), checkForIntent(), closeAnyDelayed(), closeTooltip(), openTooltip()

### Community 151 - "FTrailLabelStyle"
Cohesion: 0.29
Nodes (4): FTrailLabelStyle, Color, DrawSize, Font

### Community 155 - "ConvertPOIsJson"
Cohesion: 0.67
Nodes (3): ConvertPOIJson(), ConvertPOIsJson(), UPoiSubsystem::PerformHttpRequestForPOIs()

### Community 156 - "TrailSharedTypes.h"
Cohesion: 0.14
Nodes (12): ERunnerTemplate, DisplayName, FRaceLoaded, RaceID, RaceName, RaceTemplate, FRaceTeamsRunners, RaceId (+4 more)

### Community 157 - "FStepDef"
Cohesion: 0.50
Nodes (3): FStepDef, Label, Prefix

### Community 160 - "PlacementCalculator"
Cohesion: 0.47
Nodes (5): isSvgElement(), PlacementCalculator(), computePlacementCoords(), getHtmlPlacement(), getSvgPlacement()

### Community 162 - "FRunnerState"
Cohesion: 0.33
Nodes (4): FRunnerState, CurrentOffset, CurrentScale, Runner

### Community 165 - "MeteoPageSubsystem.cpp"
Cohesion: 0.14
Nodes (3): UHttpGatewaySubsystem::HandleCommandRequest(), UHttpGatewaySubsystem::Initialize(), UMeteoPageSubsystem::Initialize()

### Community 166 - "TeamGroupSubsystem.cpp"
Cohesion: 0.21
Nodes (6): UTeamGroupSubsystem::AddTeamToGroup(), UTeamGroupSubsystem::GetTeamsInGroup(), UTeamGroupSubsystem::Initialize(), UTeamGroupSubsystem::RemoveTeamFromGroup(), UTeamGroupSubsystem::SetRaceSetup(), UTeamGroupSubsystem::ToggleGroup()

### Community 167 - "Runner_GTWS.cpp"
Cohesion: 0.11
Nodes (13): ARunner_Generic::BeginPlay(), ARunner_Generic::ToggleFlag(), ARunner_Generic::TogglePhoto(), ARunner_Generic::UpdateRunner(), ARunner_GTWS::BeginPlay(), ARunner_GTWS::ToggleClub(), ARunner_GTWS::ToggleFlag(), ARunner_GTWS::TogglePhoto() (+5 more)

### Community 171 - "FPendingCell"
Cohesion: 0.40
Nodes (3): FPendingCell, X, Y

### Community 172 - "EKind"
Cohesion: 0.40
Nodes (5): EKind, Count, Grass, Rock, Shrub

### Community 176 - "dynsections.js"
Cohesion: 0.50
Nodes (3): codefold, dynsection, toggleVisibility()

### Community 180 - "EBenchPhase"
Cohesion: 0.50
Nodes (4): EBenchPhase, Idle, Measuring, Settling

### Community 182 - "FPointIndexSegment"
Cohesion: 0.29
Nodes (6): FPointIndexSegment, EndDistanceCm, EndIndex, Indices, StartDistanceCm, StartIndex

### Community 185 - "ARaceManager::BeginPlay"
Cohesion: 0.27
Nodes (3): ARaceManager::BeginPlay(), ARaceManager::EdTeleportToCheckpoint(), ARaceManager::RtTeleportToCheckpoint()

### Community 189 - "ARaceManager::ToggleRace_Internal"
Cohesion: 0.14
Nodes (9): ARaceManager::EdToggleCheckpoints(), ARaceManager::EdTogglePath(), ARaceManager::EdTogglePois(), ARaceManager::EdToggleSlope(), ARaceManager::RtToggleCheckpoints(), ARaceManager::RtTogglePath(), ARaceManager::RtTogglePois(), ARaceManager::RtToggleSlope() (+1 more)

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

## Knowledge Gaps
- **1151 isolated node(s):** `TeamParentClass`, `PoiParentClass`, `CheckpointParentClass`, `Queued`, `Running` (+1146 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 3102 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **368 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `FRaceSetup` connect `FRaceSetup` to `ARaceManager`, `ARunner`, `RaceManager.cpp`, `ACheckpoint`, `APoi`, `int64`, `UCheckpointSubsystem`, `URunnerSubsystem`, `Checkpoint_GTWS.cpp`, `URaceSubsystem`, `UPoiSubsystem`, `TrailSharedTypes.h`, `gameplaystatics`, `UPathSubsystem`, `TeamGroupSubsystem.cpp`, `UTeamSubsystem`, `UWeatherSubsystem`, `Runner_GTWS.cpp`, `AKm`, `UPROPERTY`, `FSettings`, `RunnerSubsystem.cpp`, `PoiSubsystem.cpp`, `FRaceCheckpoint`, `CheckpointSubsystem.cpp`, `PathSubsystem.cpp`, `TeamSubsystem.cpp`, `UTeamGroupSubsystem`, `ACheckpoint_UTMB`, `UTemplateSubsystem`, `TemplateSubsystem.cpp`, `TrailsSubsystem.cpp`, `UTrailsSubsystem`, `WeatherSubsystem.cpp`, `FRacePOI`, `splinemeshcomponent`?**
  _High betweenness centrality (0.082) - this node is a cross-community bridge._
- **What connects `TeamParentClass`, `PoiParentClass`, `CheckpointParentClass` to the rest of the system?**
  _1151 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ARaceManager` be split into smaller, more focused modules?**
  _Cohesion score 0.010050251256281407 - nodes in this community are weakly interconnected._
- **Why does `UWorldAmbienceSubsystem` connect `UWorldAmbienceSubsystem` to `FHttpResultCallback`, `WorldAmbienceSubsystem.h`, `WorldAmbienceSubsystem.cpp`, `UWorldAmbienceSubsystem::OnWorldBeginPlay`, `UPROPERTY`, `FAutoWeatherInfo`, `UWorldAmbienceSubsystem::Initialize`, `UWorldAmbienceSubsystem::TickBench`, `EBenchPhase`, `FWorldAmbienceSettings`, `FBenchShot`, `Piecewise`?**
  _High betweenness centrality (0.072) - this node is a cross-community bridge._
- **Should `UHttpGatewaySubsystem` be split into smaller, more focused modules?**
  _Cohesion score 0.017391304347826087 - nodes in this community are weakly interconnected._
- **Why does `ARaceManager` connect `ARaceManager` to `.SlateNotify`, `RaceManager.cpp`, `APath`, `FRunnerStruct`, `Checkpoint_GTWS.cpp`, `ARaceManager::RtToggleCheckpoint`, `TrailSharedTypes.h`, `gameplaystatics`, `ARaceManager::HideLoadingOverlay`, `Runner_GTWS.cpp`, `FRaceSetup`, `UPROPERTY`, `ARaceManager::BeginPlay`, `ARaceManager::ToggleRace_Internal`, `PathSubsystem.cpp`, `ARaceManager::SpawnInitialRunners`, `FPathSegment`, `PathSegmentsComponent.cpp`, `CallBP_GetFloatOut`?**
  _High betweenness centrality (0.069) - this node is a cross-community bridge._
- **Should `.SlateNotify` be split into smaller, more focused modules?**
  _Cohesion score 0.03507078507078507 - nodes in this community are weakly interconnected._