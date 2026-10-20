# Graph Report - UNREAL 5.8  (2026-10-20)

## Corpus Check
- 537 files · ~1,776,923 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5284 file(s) not represented in the graph (top: .uasset 4702, .tex 369, .eps 71)

## Summary
- 4752 nodes · 6869 edges · 553 communities (166 shown, 387 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 552 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `9ca682f9`
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
- .GetWorld
- Runner.cpp
- UNameComponent
- SearchBox
- UTMB_NameComponent.cpp
- AWorldFauna
- ULoadingStatusSubsystem
- jquery.js
- URaceSubsystem
- UPoiSubsystem
- AWorldFlora
- Checkpoint.cpp
- PathSegmentsComponent.cpp
- UWaterMaskSubsystem
- UAudioSanitizerSubsystem
- Path.cpp
- Poi.cpp
- Runner.h
- splinemeshcomponent
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
- FRaceStruct
- UBroadcastCaptureSubsystem
- FAutoWeatherInfo
- UClubComponent
- FSettings
- RunnerSubsystem.cpp
- UTrailTextWidgetComponent
- FOpenWeatherCurrent
- UScaleSubsystem
- FWorldAmbienceSettings
- PoiSubsystem.cpp
- WaterMaskSubsystem.cpp
- ScaleSubsystem.cpp
- ARunner_UTMB
- AudioSanitizer.cpp
- FRaceCheckpoint
- FWeatherSample
- TrailTextWidgetComponent.cpp
- WorldFlora.cpp
- SLoadingOverlay.cpp
- CheckpointSubsystem.cpp
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
- UPhotoComponent
- CameraControlComponent.cpp
- APoi_GTWS
- AWorldHeadlamps
- APoi_Nike
- MeteoPageSubsystem.cpp
- UPathSegmentsComponent
- Ee
- e
- PathSubsystem.cpp
- ARaceManager::SpawnInitialRunners
- GTWS_NameComponent.cpp
- RunnerStackingSubsystem.cpp
- APoi_Generic
- SBroadcastMirror
- APath::GetKmClassForRace
- HttpGatewaySubsystem.h
- UMeteoPageSubsystem
- UTeamGroupSubsystem
- NameComponent.cpp
- ACheckpoint_GTWS
- ACheckpoint_UTMB
- UTemplateSubsystem
- TemplateSubsystem.cpp
- OverpassToFeatures
- HttpGatewaySubsystem.cpp
- .GetGameInstance
- FWorldAmbienceState
- UObject
- ARunner_Generic
- APoi_UTMB
- TeamSubsystem.cpp
- Km.cpp
- FVector
- showTip
- UTemplateGeneratorUtilityWidget
- PhotoComponent.cpp
- UTrailsSubsystem
- WeatherSubsystem.cpp
- FTrailTemplateStack
- USettingsSaveGame
- int64
- RaceSubsystem.cpp
- FBenchShot
- FSubmixEffectSanitizer
- FDistanceScaleConfig
- FStackEntry
- Checkpoint_GTWS.cpp
- CallBP_GetFloatOut
- gameplaystatics
- UCameraControlComponent
- SlateNotificationsBFL.h
- CesiumBudgetSubsystem.cpp
- FRunnerStackingConfig
- FPointIndexSegment
- ICameraControlInterface
- FRaceCard
- UGTWS_NameComponent
- FMinMax
- FWaterMaskBounds
- FString
- FTrailLabelStyle
- UPROPERTY
- ICheckpointInterface
- FRaceTeamsRunners
- WorldFlora.h
- ARunner_Nike
- AudioSanitizer.h
- BroadcastSettings.h
- FRacePath
- UCesiumBudgetSubsystem
- DisplayController
- TrailHttpDebug.h
- UTeamSubsystem::GetRunnerActorByRaceTeam
- FWaterPolygon
- FlagComponent.cpp
- FCheckpoints
- WorldFauna.h
- IPoiInterface
- FTeamStruct
- PlacementCalculator
- UNike_NameComponent
- FRunnerState
- FPathSegment
- IPathInterface
- MakeStep
- WorldAmbienceSubsystem.h
- UFlagComponent
- coreminimal
- UWorldAmbienceSubsystem::OnWorldBeginPlay
- UGeneric_NameComponent
- FPendingCell
- EKind
- UWorldAmbienceSubsystem::UpdateState
- IRunnerStackableInterface
- UCheckpointSubsystem::SpawnCheckpointActor
- dynsections.js
- UWorldAmbienceSubsystem::Initialize
- AWorldFlora::ContinueBuild
- AWorldFlora::Tick
- EBenchPhase
- ClubComponent.h
- ARaceManager::RtToggleCheckpoint
- UGeneric_FlagComponent
- makeTree
- AWorldFauna::Tick
- FState
- CLAUDE.md
- Piecewise
- ARaceManager::ToggleRace_Internal
- UUTMB_NameComponent
- FTrackSample
- cookie.js
- .mcp.json
- BroadcastCaptureSubsystem.h
- URunnerSubsystem::Deinitialize
- USettingsSubsystem::Initialize
- USettingsSubsystem::Save
- Automation_TrailSimulator.slnx
- TrailSimulator.slnx
- BuildMesh
- ACheckpoint::GetBoundsLocation
- UGeneric_NameComponent::UpdateName
- ARaceManager::EndPlay
- ARaceManager::RtMoveToPath
- ARaceManager::RtToggleCheckpointWeather
- ARaceManager::RtUpdateGlowIntensity
- UAudioSanitizerSubsystem::CheckNaN
- FRunners
- ARaceManager::RtUpdatePulseGlow
- ARaceManager::RtMoveCheckpoint

## God Nodes (most connected - your core abstractions)
1. `ARaceManager` - 276 edges
2. `ARunner` - 132 edges
3. `UWorldAmbienceSubsystem` - 126 edges
4. `UHttpGatewaySubsystem` - 118 edges
5. `ACheckpoint` - 111 edges
6. `APoi` - 107 edges
7. `APath` - 105 edges
8. `FRaceSetup` - 71 edges
9. `USettingsSubsystem` - 71 edges
10. `FWorldUtils` - 61 edges

## Surprising Connections (you probably didn't know these)
- `UTemplateGeneratorUtilityWidget` --references--> `TSubclassOf`  [EXTRACTED]
  Plugins/TemplateGenerator/Source/TemplateGenerator/Public/TemplateGeneratorUtilityWidget.h → Source/TrailUtils/Public/WorldUtils.h
- `URaceSubsystem::IsRaceSetupValid()` --references--> `FRaceSetup`  [EXTRACTED]
  Source/Subsystems/Private/RaceSubsystem.cpp → Source/SharedTypes/Public/TrailSharedTypes.h
- `URaceSubsystem::SetCurrentRaceSetup()` --references--> `FRaceSetup`  [EXTRACTED]
  Source/Subsystems/Private/RaceSubsystem.cpp → Source/SharedTypes/Public/TrailSharedTypes.h
- `UClubComponent::UpdateClub()` --references--> `FRunnerStruct`  [EXTRACTED]
  Source/TrailSimulator/Private/Components/ClubComponent.cpp → Source/SharedTypes/Public/TrailSharedTypes.h
- `UScaleSubsystem::SetGlobalConfig()` --references--> `FDistanceScaleConfig`  [EXTRACTED]
  Source/Subsystems/Private/ScaleSubsystem.cpp → Source/SharedTypes/Public/TrailSharedTypes.h

## Import Cycles
- None detected.

## Communities (553 total, 387 thin omitted)

### Community 0 - "ARaceManager"
Cohesion: 0.01
Nodes (71): ARaceManager::HideLoadingOverlay(), ARaceManager, AllRaces, bAfficher, bIsDay, bOverlayHidden, bSRTEncodeAudio, bStartSRTOnBeginPlay (+63 more)

### Community 1 - "UHttpGatewaySubsystem"
Cohesion: 0.02
Nodes (57): UHttpGatewaySubsystem, CommandRouteHandle, HttpRouter, ListenPort, OnAddTeamToGroup, OnArmLengthUpdated, OnCameraAngleUpdated, OnCheckpointAnimation (+49 more)

### Community 2 - ".SlateNotify"
Cohesion: 0.03
Nodes (75): ARaceManager::BeginPlay(), ARaceManager::EdAddTeamToGroup(), ARaceManager::EdAnimCheckpoint(), ARaceManager::EdAnimPoi(), ARaceManager::EdAnimTeam(), ARaceManager::EdChangePathOffset(), ARaceManager::EdPauseTravel(), ARaceManager::EdRefreshRace() (+67 more)

### Community 3 - "ARunner"
Cohesion: 0.02
Nodes (59): ARunner, ActiveState, ArmLength, bHasDayNight, bHasStackChildren, bHasTrackDist, bLastIsDay, bLookAtEnabled (+51 more)

### Community 4 - "RaceManager.cpp"
Cohesion: 0.05
Nodes (56): ARaceManager::AddTeamToGroup_Internal(), ARaceManager::AnimCheckpoint_Internal(), ARaceManager::AnimPoi_Internal(), ARaceManager::AnimTeam_Internal(), ARaceManager::ApplyPathSegments(), ARaceManager::ChangePathOffset_Internal(), ARaceManager::EdStopAnimTeam(), ARaceManager::EdToggleRace() (+48 more)

### Community 5 - "APath"
Cohesion: 0.02
Nodes (58): APath, bForward, bPathShown, BroadCastSubsystem, CameraComponent, CheckpointSubsystem, DistanceCm, DynaPawn (+50 more)

### Community 6 - "ACheckpoint"
Cohesion: 0.02
Nodes (57): ACheckpoint, AltPicto, AltText, ArmLength, bHasDayNight, bLastIsDay, bLookAtEnabled, bOrbitEnabled (+49 more)

### Community 7 - "APoi"
Cohesion: 0.02
Nodes (56): IScaleInterface, APoi, AltPicto, AltText, ArmLength, bHasDayNight, bLastIsDay, bLookAtEnabled (+48 more)

### Community 8 - "UWorldAmbienceSubsystem"
Cohesion: 0.02
Nodes (56): UWorldAmbienceSubsystem::TickBench(), UWorldAmbienceSubsystem, AppliedWeather, AutoWeather, bBegunPlay, bBenchGrounded, BenchCsvPath, BenchFrameSamples (+48 more)

### Community 9 - "USettingsSubsystem"
Cohesion: 0.03
Nodes (24): USettingsSubsystem, CachedSave, LookAtInterpSpeed, LookAtIntervalSeconds, OnFetchUpdate, OnLengthChanged, OnMinMaxDirty, OnMinMaxSet (+16 more)

### Community 10 - "WorldAmbienceSubsystem.cpp"
Cohesion: 0.05
Nodes (19): Average(), ImportProperty(), JsonResponse(), Percentile(), SettingsFilePath(), UWorldAmbienceSubsystem::ApplySetting(), UWorldAmbienceSubsystem::BenchJson(), UWorldAmbienceSubsystem::Deinitialize() (+11 more)

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
Cohesion: 0.09
Nodes (23): FRunnerStruct, canal, canalId, club, dossard, elevation, IndexM, kmActuel (+15 more)

### Community 15 - ".GetWorld"
Cohesion: 0.10
Nodes (19): ITrailInputModeInterface, ACheckpoint::StartAnimation(), ACheckpoint::StopAnimation(), ARunner_Generic::BeginPlay(), ACheckpoint_GTWS::BeginPlay(), APoi_GTWS::BeginPlay(), ARunner_GTWS::BeginPlay(), APoi::StartAnimation() (+11 more)

### Community 16 - "Runner.cpp"
Cohesion: 0.05
Nodes (8): ARunner::AssignRunnerToTeam(), ARunner::GetStackAttachComponent(), ARunner::GetStackHookComponent(), ARunner::HandleTemplateChanged(), ARunner::SetMinMax(), ARunner::TriggerUpdateAfterHidden(), UCesiumFlyToComponent, USettingsSubsystem

### Community 17 - "UNameComponent"
Cohesion: 0.08
Nodes (11): UNameComponent, bShow3DText, bShowWidgetText, LabelFont, LastLocation, NameBkgComponent, NameFont, NameSplineComponent (+3 more)

### Community 18 - "SearchBox"
Cohesion: 0.13
Nodes (7): createResults(), init_search(), SearchBox(), getXPos(), getYPos(), searchResults, convertToId()

### Community 19 - "UTMB_NameComponent.cpp"
Cohesion: 0.12
Nodes (3): UUTMB_NameComponent::UpdateMesh(), UUTMB_NameComponent::UpdateName(), UUTMB_NameComponent::UUTMB_NameComponent()

### Community 20 - "AWorldFauna"
Cohesion: 0.07
Nodes (21): AWorldFauna, Ambience, Anchor, AnchorGroundZ, AnchorTimer, AppliedDensity, bHasAnchor, BirdMaterial (+13 more)

### Community 21 - "ULoadingStatusSubsystem"
Cohesion: 0.07
Nodes (13): Clamp01(), ULoadingStatusSubsystem::Complete(), ULoadingStatusSubsystem::Fail(), ULoadingStatusSubsystem::MarkDirty(), ULoadingStatusSubsystem::RegisterTask(), ULoadingStatusSubsystem::SetRunning(), ULoadingStatusSubsystem::Update(), ULoadingStatusSubsystem (+5 more)

### Community 22 - "jquery.js"
Cohesion: 0.08
Nodes (13): at(), getEventsNS(), getViewportDimensions(), initMouseDetection(), initTracking(), isTouchEvent(), lt(), trackMouse() (+5 more)

### Community 23 - "URaceSubsystem"
Cohesion: 0.07
Nodes (10): UCheckpointSubsystem::Initialize(), UPathSubsystem::Initialize(), URaceSubsystem, CurrentRaceId, CurrentRaceSetup, MainURL, OnRaceSetupDatasGathered, RaceRequest (+2 more)

### Community 24 - "UPoiSubsystem"
Cohesion: 0.06
Nodes (10): UPoiSubsystem, ActivePoiRequests, CurrentRaceSetup, OnPoiDatasGathered, OnPoisDatasGathered, POI, POIs, PoisActorsMap (+2 more)

### Community 25 - "AWorldFlora"
Cohesion: 0.06
Nodes (21): AWorldFlora, AllComponents, Ambience, bBuilding, bShown, BuildCenter, BuildDensity, BuildRadius (+13 more)

### Community 26 - "Checkpoint.cpp"
Cohesion: 0.07
Nodes (8): ACheckpoint::ACheckpoint(), ACheckpoint::CapitalizeFirst(), ACheckpoint::HandleTemplateChanged(), ACheckpoint::OnLookAtTimerTick(), ACheckpoint::RefreshCheckpointValues(), ACheckpoint::SetMinMax(), ACheckpoint::ToggleCheckpointWeather(), ACheckpoint::UpdateCheckpoint()

### Community 27 - "PathSegmentsComponent.cpp"
Cohesion: 0.10
Nodes (6): UPathSegmentsComponent::ConfigurePiece(), UPathSegmentsComponent::CreatePiece(), Apply(), ApplyToAllTexts(), EnableOcclusion(), GetMaterial()

### Community 28 - "UWaterMaskSubsystem"
Cohesion: 0.09
Nodes (8): UWaterMaskSubsystem::HandlePathDatasGathered(), UWaterMaskSubsystem, ActiveRequests, bWaterPolygonsDirty, MaterialLayerKey, PathSubsystem, RaceFeatures, WaterPolygons

### Community 29 - "UAudioSanitizerSubsystem"
Cohesion: 0.06
Nodes (19): UAudioSanitizerSubsystem, bCulpritRetrying, BeginPlayTime, bSoundOpened, CheckTimer, ClippedSinceLog, Culprit, CulpritRetryDelay (+11 more)

### Community 30 - "Path.cpp"
Cohesion: 0.06
Nodes (20): ActionValue(), APath::APath(), APath::BuildKms(), APath::MoveCheckpoint(), APath::RebuildPathSplineMeshes(), APath::RebuildSlopeSplineMeshes(), APath::StartPulse(), APath::StartTravelBackward() (+12 more)

### Community 31 - "Poi.cpp"
Cohesion: 0.07
Nodes (5): APoi::CapitalizeFirst(), APoi::GetBoundsLocation(), APoi::HandleTemplateChanged(), APoi::OnLookAtTimerTick(), APoi::SetMinMax()

### Community 33 - "splinemeshcomponent"
Cohesion: 0.06
Nodes (10): UClubComponent::GetLocalSize(), UClubComponent::UClubComponent(), UClubComponent::UpdateClub(), UClubComponent::UpdateMesh(), UGeneric_NameComponent::UGeneric_NameComponent(), UGTWS_ClubComponent::SetColors(), UGTWS_ClubComponent::UGTWS_ClubComponent(), UNike_ClubComponent::UNike_ClubComponent() (+2 more)

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
Cohesion: 0.06
Nodes (20): FOpenWeatherResponse, current, lat, lon, timezone, timezone_offset, FWeatherResult, bSuccess (+12 more)

### Community 41 - "AKm"
Cohesion: 0.08
Nodes (12): IKmInterface, AKm, DynaPawn, Georeference, GlobeAnchorComponent, LabelComp, LookAtTimerHandle, MainRoot (+4 more)

### Community 42 - "FRaceSetup"
Cohesion: 0.07
Nodes (28): FRacePOI, elevation, lat, lon, name, poiId, type, weather (+20 more)

### Community 43 - "unrealbuildtool"
Cohesion: 0.11
Nodes (9): SlateNotifications, TemplateGenerator, SharedTypes, Subsystems, TrailInterfaces, TrailSimulatorTarget, TrailSimulator, TrailSimulatorEditorTarget (+1 more)

### Community 44 - "FRaceStruct"
Cohesion: 0.20
Nodes (9): FPOIs, POIs, FRaceStruct, Checkpoints, path, poi, raceId, RaceRunners (+1 more)

### Community 45 - "UBroadcastCaptureSubsystem"
Cohesion: 0.08
Nodes (10): UBroadcastCaptureSubsystem, Accum, ActiveSource, CachedPC, DefaultSource, MirrorViewport, MirrorWidget, MonitorHz (+2 more)

### Community 46 - "FAutoWeatherInfo"
Cohesion: 0.08
Nodes (24): FAutoWeatherInfo, AgeMinutes, bHasData, CameraAltitudeM, CameraTempC, CloudBaseM, Clouds, Description (+16 more)

### Community 47 - "UClubComponent"
Cohesion: 0.06
Nodes (15): UClubComponent, bShow3DText, bShowWidgetText, ClubBkgComponent, ClubFont, ClubSplineComponent, ClubTextComponent, LastLocation (+7 more)

### Community 48 - "FSettings"
Cohesion: 0.13
Nodes (14): FSettings, bGeoidCorrected, FetchFrequency, GlowValue, Length, MinMax, Pitch, PulseFrequency (+6 more)

### Community 49 - "RunnerSubsystem.cpp"
Cohesion: 0.18
Nodes (16): URunnerSubsystem::Animation(), URunnerSubsystem::AnimationByTeam(), URunnerSubsystem::CancelRunnersRetry(), URunnerSubsystem::DoesRunnerBelongsToRace(), URunnerSubsystem::GetRunnerActor(), URunnerSubsystem::GetRunnerActorByTeam(), URunnerSubsystem::GetRunnersByRaceId(), URunnerSubsystem::RemoveTeamsNotInSnapshot() (+8 more)

### Community 50 - "UTrailTextWidgetComponent"
Cohesion: 0.08
Nodes (8): STextBlock, UTrailTextWidgetComponent, CurrentColor, CurrentFont, CurrentJustification, CurrentSize, CurrentText, SlateTextBlock

### Community 51 - "FOpenWeatherCurrent"
Cohesion: 0.09
Nodes (22): FOpenWeatherCondition, description, icon, id, main, FOpenWeatherCurrent, clouds, dew_point (+14 more)

### Community 52 - "UScaleSubsystem"
Cohesion: 0.08
Nodes (4): UScaleSubsystem::Tick(), UScaleSubsystem, Entries, GlobalConfig

### Community 53 - "FWorldAmbienceSettings"
Cohesion: 0.09
Nodes (22): FWorldAmbienceSettings, AutoWeatherRefreshMinutes, bAutoWeather, bCityLights, bFauna, bFlora, bHeadlamps, bLowClouds (+14 more)

### Community 54 - "PoiSubsystem.cpp"
Cohesion: 0.17
Nodes (14): ConvertPOIJson(), ConvertPOIsJson(), UPoiSubsystem::Animation(), UPoiSubsystem::DoesPoiBelongsToRace(), UPoiSubsystem::GetPoiActor(), UPoiSubsystem::PerformHttpRequestForPOIs(), UPoiSubsystem::SetRaceSetup(), UPoiSubsystem::SpawnPoi() (+6 more)

### Community 55 - "WaterMaskSubsystem.cpp"
Cohesion: 0.15
Nodes (3): UWaterMaskSubsystem::ApplyToTilesets(), UWaterMaskSubsystem::Initialize(), UWaterMaskSubsystem::IsWaterAt()

### Community 56 - "ScaleSubsystem.cpp"
Cohesion: 0.11
Nodes (10): UScaleSubsystem::ComputeTargetScale(), UScaleSubsystem::GetCameraLocation(), UScaleSubsystem::GetStatId(), UScaleSubsystem::HandleActorEndPlay(), UScaleSubsystem::Initialize(), UScaleSubsystem::IsTickable(), UScaleSubsystem::RegisterScalableActor(), UScaleSubsystem::SetActorScaleMinMax() (+2 more)

### Community 57 - "ARunner_UTMB"
Cohesion: 0.10
Nodes (15): ARunner_UTMB, FlagComponent, FlagHook, IndexBarreComponent, IndexBkgComponent, IndexBkgHook, IndexLabelComponent, IndexLeftComponent (+7 more)

### Community 58 - "AudioSanitizer.cpp"
Cohesion: 0.13
Nodes (7): DayPhase(), FSubmixEffectSanitizer::OnProcessAudio(), SetWorldMute(), UAudioSanitizerSubsystem::Deinitialize(), UAudioSanitizerSubsystem::Initialize(), UAudioSanitizerSubsystem::OnWorldBeginPlay(), UAudioSanitizerSubsystem::UpdateForest()

### Community 59 - "FRaceCheckpoint"
Cohesion: 0.12
Nodes (17): FRaceCheckpoint, altitude, checkpointId, distance, elevation, name, type, weather (+9 more)

### Community 60 - "FWeatherSample"
Cohesion: 0.11
Nodes (15): FBenchResult, FrameAvg, FrameP99, GpuAvg, GpuP99, Name, Samples, FWeatherSample (+7 more)

### Community 61 - "TrailTextWidgetComponent.cpp"
Cohesion: 0.12
Nodes (8): UTrailTextWidgetComponent::GetFontInfo(), UTrailTextWidgetComponent::OnRegister(), UTrailTextWidgetComponent::SetLabelCasseToLower(), UTrailTextWidgetComponent::SetLabelCasseToUpper(), UTrailTextWidgetComponent::SetLabelColor(), UTrailTextWidgetComponent::SetLabelFont(), UTrailTextWidgetComponent::SetLabelText(), UTrailTextWidgetComponent::UTrailTextWidgetComponent()

### Community 62 - "WorldFlora.cpp"
Cohesion: 0.12
Nodes (6): AWorldFlora::BeginPlay(), AWorldFlora::HandlePathGathered(), AWorldFlora::IsOnTrail(), AWorldFlora::StartBuild(), AWorldFlora::TraceGround(), Hash()

### Community 63 - "SLoadingOverlay.cpp"
Cohesion: 0.11
Nodes (5): Font(), FStepDef, Label, Prefix, SLoadingOverlay::GetOverallPercent()

### Community 64 - "CheckpointSubsystem.cpp"
Cohesion: 0.31
Nodes (9): UCheckpointSubsystem::Animation(), UCheckpointSubsystem::DoesChkBelongsToRace(), UCheckpointSubsystem::SetRaceSetup(), UCheckpointSubsystem::StopAnimation(), UCheckpointSubsystem::Teleport(), UCheckpointSubsystem::ToggleCheckpoint(), UCheckpointSubsystem::ToggleCheckpoints(), UCheckpointSubsystem::ToggleWeather() (+1 more)

### Community 66 - "ARunner_GTWS"
Cohesion: 0.11
Nodes (11): ARunner_GTWS, ClubComponent, ClubHook, DefaultClubLocation, DefaultNameLocation, FlagComponent, FlagHook, NameComponent (+3 more)

### Community 67 - "Templates Unreal (TrailSimulator) servis par OnGraphics"
Cohesion: 0.11
Nodes (18): 1. Le contexte en deux minutes, 2. Réponse à « conventions Figma ou JSON de template ? », 3.1 Routes attendues sur OnGraphics, 3.2 Format (version 1), 3.3 Règles de valeur, 3.4 Ce qu'Unreal ne lit pas (inutile de l'envoyer), 3. Le contrat : `GET /unreal/template/{nom}`, 4.1 Trois graphics de plus (+10 more)

### Community 68 - "BroadcastCaptureSubsystem.cpp"
Cohesion: 0.13
Nodes (6): UBroadcastCaptureSubsystem::ActivateSource(), UBroadcastCaptureSubsystem::Deinitialize(), UBroadcastCaptureSubsystem::Initialize(), UBroadcastCaptureSubsystem::ResolveCaptureSourceFromViewTarget(), UBroadcastCaptureSubsystem::UpdateViewportMirror(), IBroadcastInterface

### Community 69 - "FTrailTemplate"
Cohesion: 0.07
Nodes (29): FTrailTemplate, Base, Checkpoint, Globals, Name, Poi, Runner, FTrailTemplateGlobals (+21 more)

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

### Community 77 - "UPhotoComponent"
Cohesion: 0.13
Nodes (7): UPhotoComponent, BaseMat, CurrentPhotoTexture, CurrentPhotoUrl, PendingPhotoUrl, PhotoMID, PhotoSMC

### Community 79 - "APoi_GTWS"
Cohesion: 0.12
Nodes (12): AWorldFauna::BuildMeshes(), UMaterialInstanceDynamic, UPhotoComponent::EnsurePhotoMID(), APoi_GTWS, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat (+4 more)

### Community 80 - "AWorldHeadlamps"
Cohesion: 0.08
Nodes (11): AWorldHeadlamps::BeginPlay(), AWorldHeadlamps::GetLamp(), AWorldHeadlamps::Tick(), AWorldHeadlamps, Ambience, Lamps, LitCount, RefreshTimer (+3 more)

### Community 81 - "APoi_Nike"
Cohesion: 0.12
Nodes (13): APoi_Nike, Infos1BkgComponent, Infos1SplineComponent, Infos1WidgetComponent, Infos2BkgComponent, Infos2SplineComponent, Infos2WidgetComponent, InfosFont (+5 more)

### Community 82 - "MeteoPageSubsystem.cpp"
Cohesion: 0.20
Nodes (8): FindGeoreference(), FindPlayWorld(), GeoJson(), JsonResponse(), UMeteoPageSubsystem::HandleGeoRequest(), UMeteoPageSubsystem::HandleGeoSetRequest(), UMeteoPageSubsystem::HandlePageRequest(), UMeteoPageSubsystem::Initialize()

### Community 83 - "UPathSegmentsComponent"
Cohesion: 0.05
Nodes (22): UPathSegmentsComponent::Apply(), UPathSegmentsComponent::Refresh(), UPathSegmentsComponent::RestoreTrace(), UPathSegmentsComponent::SetSegments(), UPathSegmentsComponent, ExtraPieces, HighlightedBaseScales, HighlightedPieces (+14 more)

### Community 84 - "Ee"
Cohesion: 0.16
Nodes (16): A(), b(), be(), Ee(), f(), he(), je(), k() (+8 more)

### Community 85 - "e"
Cohesion: 0.19
Nodes (16): ce(), e(), fe(), H(), I(), l(), le(), M() (+8 more)

### Community 86 - "PathSubsystem.cpp"
Cohesion: 0.10
Nodes (10): ConvertPathJson(), UPathSubsystem::GetPathById(), UPathSubsystem::PerformHttpRequestForPath(), UPathSubsystem::SetRaceSetup(), UPathSubsystem::SpawnKmActor(), UPathSubsystem::SpawnPathActor(), ConvertRacesJson(), UTrailsSubsystem::Initialize() (+2 more)

### Community 87 - "ARaceManager::SpawnInitialRunners"
Cohesion: 0.27
Nodes (4): ARaceManager::ProcessPendingRunnerUpdates(), ARaceManager::SpawnInitialRunners(), ARaceManager::UpdateRunnersFromSnapshot(), LogRaceTeamIndex()

### Community 88 - "GTWS_NameComponent.cpp"
Cohesion: 0.09
Nodes (12): FStartAndEnd, EndLocation, StartLocation, UClubComponent::GetStartAndEnd(), UGTWS_NameComponent::GetSplineLastPoint(), UGTWS_NameComponent::GetStartAndEnd(), UGTWS_NameComponent::SetColors(), UGTWS_NameComponent::UGTWS_NameComponent() (+4 more)

### Community 89 - "RunnerStackingSubsystem.cpp"
Cohesion: 0.11
Nodes (13): URunnerStackingSubsystem::ApplyNewStackingState(), URunnerStackingSubsystem::AttachRunnerToBase(), URunnerStackingSubsystem::ComputeDynamicRadiusCm(), URunnerStackingSubsystem::ComputeOverlapRadiusCm(), URunnerStackingSubsystem::Deinitialize(), URunnerStackingSubsystem::DetachRunnerToTrack(), URunnerStackingSubsystem::FindEntryIndex(), URunnerStackingSubsystem::HandleActorEndPlay() (+5 more)

### Community 90 - "APoi_Generic"
Cohesion: 0.12
Nodes (11): APoi_Generic, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, NameLeftComponent, NameRightComponent (+3 more)

### Community 91 - "SBroadcastMirror"
Cohesion: 0.14
Nodes (5): SBroadcastMirror, Background, bLogged, Brush, Target

### Community 92 - "APath::GetKmClassForRace"
Cohesion: 0.10
Nodes (6): APath::GetKmClassForRace(), APath::SpawnCheckpointAt(), AKm_Generic, AKm_GTWS, AKm_Nike, AKm_UTMB

### Community 94 - "HttpGatewaySubsystem.h"
Cohesion: 0.12
Nodes (9): FCheckpointMove, Altitude, CheckpointID, Distance, Elevation, Lat, Lon, PathIndex (+1 more)

### Community 95 - "UMeteoPageSubsystem"
Cohesion: 0.11
Nodes (8): FHttpServerRequest, IHttpRouter, UMeteoPageSubsystem, GeoRouteHandle, GeoSetRouteHandle, HttpRouter, ListenPort, PageRouteHandle

### Community 96 - "UTeamGroupSubsystem"
Cohesion: 0.09
Nodes (11): UTeamGroupSubsystem::AddTeamToGroup(), UTeamGroupSubsystem::GetTeamsInGroup(), UTeamGroupSubsystem::Initialize(), UTeamGroupSubsystem::RemoveTeamFromGroup(), UTeamGroupSubsystem::SetRaceSetup(), UTeamGroupSubsystem::ToggleGroup(), UTeamGroupSubsystem, CurrentRaceSetup (+3 more)

### Community 97 - "NameComponent.cpp"
Cohesion: 0.15
Nodes (6): UNameComponent::GetLocalSize(), UNameComponent::GetSplineComponent(), UNameComponent::GetWorldLastPointLocation(), UNameComponent::UNameComponent(), UNameComponent::UpdateMesh(), UNameComponent::UpdateName()

### Community 98 - "ACheckpoint_GTWS"
Cohesion: 0.12
Nodes (11): APath::GetCheckpointClassForRace(), ACheckpoint_GTWS, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID (+3 more)

### Community 99 - "ACheckpoint_UTMB"
Cohesion: 0.13
Nodes (10): ACheckpoint_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID, RaceManager (+2 more)

### Community 100 - "UTemplateSubsystem"
Cohesion: 0.11
Nodes (6): UTemplateSubsystem, ActiveRequests, OnTemplateChanged, Templates, ARunner::UpdateRunner(), ARaceManager::PrepareRunnerRuntimeData()

### Community 101 - "TemplateSubsystem.cpp"
Cohesion: 0.24
Nodes (8): UTemplateSubsystem::Fetch(), UTemplateSubsystem::GetBase(), UTemplateSubsystem::GetBuiltin(), UTemplateSubsystem::GetForRace(), UTemplateSubsystem::GetRunnerHeight(), UTemplateSubsystem::GetTemplate(), UTemplateSubsystem::ReloadTemplate(), UTemplateSubsystem::RequestTemplate()

### Community 102 - "OverpassToFeatures"
Cohesion: 0.26
Nodes (8): BufferLine(), IsClosed(), LineWidthMeters(), OverpassToFeatures(), PointInRing(), ReadGeometry(), RingToJson(), StitchRings()

### Community 104 - ".GetGameInstance"
Cohesion: 0.10
Nodes (17): UPoiSubsystem::Initialize(), URunnerSubsystem::CancelAllRunnersRetries(), URunnerSubsystem::Initialize(), UTeamSubsystem::Initialize(), UWorldAmbienceSubsystem::DoesSupportWorldType(), Type, ACheckpoint::BeginPlay(), ACheckpoint::EndPlay() (+9 more)

### Community 105 - "FWorldAmbienceState"
Cohesion: 0.14
Nodes (13): FWorldAmbienceState, CloudCoverage, Fog, FrameMs, GpuMs, GuardLevel, Night, Rain (+5 more)

### Community 106 - "UObject"
Cohesion: 0.14
Nodes (14): ExportProperty(), FindActorByClassToken(), ReadNumber(), UAudioSanitizerSubsystem::ShouldCreateSubsystem(), FWorldUtils::GetGameInstance(), FWorldUtils::GetGameInstanceChecked(), FWorldUtils::GetRuntimeWorldFallback(), FWorldUtils::GetWorld() (+6 more)

### Community 107 - "ARunner_Generic"
Cohesion: 0.14
Nodes (7): ARunner_Generic, FlagComponent, NameComponent, PhotoComponent, PhotoHook, RaceManager, SlashComponent

### Community 108 - "APoi_UTMB"
Cohesion: 0.12
Nodes (11): ARaceManager::GetPoiClassForRace(), ARaceManager::TrackRunnerInterp(), APoi_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID (+3 more)

### Community 109 - "TeamSubsystem.cpp"
Cohesion: 0.21
Nodes (15): UCesiumFlyToComponent, UTeamSubsystem::Animation(), UTeamSubsystem::ClearRaceTeamIndex(), UTeamSubsystem::DisplayClub(), UTeamSubsystem::DisplayFlag(), UTeamSubsystem::DisplayPhoto(), UTeamSubsystem::DisplayTeams(), UTeamSubsystem::GetRunnerByRaceTeam() (+7 more)

### Community 110 - "Km.cpp"
Cohesion: 0.17
Nodes (4): AKm::AKm(), AKm::BeginPlay(), AKm::OnLookAtTimerTick(), AKm::UpdateKm()

### Community 111 - "FVector"
Cohesion: 0.28
Nodes (9): APath::GetClosestSplineLocation(), APath::GetDistanceAlongSpline(), APath::GetLocationAtDistance(), BuildRail(), BuildSimplifiedRailLocked(), BuildSmoothRail(), DistPointSegment3D(), RDP_Keep() (+1 more)

### Community 112 - "showTip"
Cohesion: 0.28
Nodes (13): countFlags(), CSSCoordinates(), getTooltipContent(), getViewportCollisions(), isMouseOver(), TooltipController(), beginShowTip(), closeDesyncedTip() (+5 more)

### Community 113 - "UTemplateGeneratorUtilityWidget"
Cohesion: 0.15
Nodes (4): UTemplateGeneratorUtilityWidget, CheckpointParentClass, PoiParentClass, TeamParentClass

### Community 114 - "PhotoComponent.cpp"
Cohesion: 0.21
Nodes (3): UPhotoComponent::ClearPhoto(), UPhotoComponent::UpdateDayNight(), UPhotoComponent::UpdatePhoto()

### Community 115 - "UTrailsSubsystem"
Cohesion: 0.15
Nodes (6): UTrailsSubsystem, CurrentRaceSetup, LoadingSubsystem, OnRacesDatasGathered, RacesArray, RaceSubsystem

### Community 116 - "WeatherSubsystem.cpp"
Cohesion: 0.20
Nodes (5): ConvertWeatherJson(), UWeatherSubsystem::Deinitialize(), UWeatherSubsystem::Initialize(), UWeatherSubsystem::PerformHttpRequestForWeather(), UWeatherSubsystem::SetRaceSetup()

### Community 117 - "FTrailTemplateStack"
Cohesion: 0.15
Nodes (13): FTrailTemplateStack, Foot, InfosBkg, InfosPicto, InfosText, Line1, Line2, MainPicto (+5 more)

### Community 118 - "USettingsSaveGame"
Cohesion: 0.20
Nodes (7): USettingsSubsystem::Load(), USettingsSubsystem::SetMainURL(), USettingsSaveGame, GlobalLength, GlobalPitch, GlobalZAnchor, MainURL

### Community 119 - "int64"
Cohesion: 0.18
Nodes (8): FRaceEntry, name, raceId, templateName, FRaceLoaded, RaceID, RaceName, RaceTemplate

### Community 120 - "RaceSubsystem.cpp"
Cohesion: 0.24
Nodes (9): ConvertRaceJson(), URaceSubsystem::GetCurrentRaceId(), URaceSubsystem::GetMainURL(), URaceSubsystem::GetRaceSetupById(), URaceSubsystem::GetRaceSetupByName(), URaceSubsystem::IsRaceSetupValid(), URaceSubsystem::PerformHttpRequestForRaceSetup(), URaceSubsystem::SetCurrentRaceId() (+1 more)

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
Cohesion: 0.11
Nodes (16): FEntry, Actor, CurrentMinMax, OriginalScale, OverrideConfig, FStackEntry, bClubVisible, bPhotoVisible (+8 more)

### Community 125 - "Checkpoint_GTWS.cpp"
Cohesion: 0.09
Nodes (7): ACheckpoint_Generic::UpdateCheckpoint(), UseRoundedBkg_CheckpointGeneric(), APoi_Generic::UpdatePoi(), UseRoundedBkg_PoiGeneric(), ACheckpoint_GTWS::UpdateCheckpoint(), UseRoundedBkg_CheckpointGTWS(), FitLine()

### Community 126 - "CallBP_GetFloatOut"
Cohesion: 0.17
Nodes (7): ARaceManager::EdUpdateMainUrl(), ARaceManager::HandleRacesDatasGathered(), ARaceManager::RtUpdateMainUrl(), ARaceManager::UpdateMainUrl_Internal(), ARaceManager::UpdateUdsTime(), CallBP_GetFloatOut(), SanitizeKey()

### Community 127 - "gameplaystatics"
Cohesion: 0.07
Nodes (14): ARunner_Generic::ToggleFlag(), ARunner_Generic::TogglePhoto(), ARunner_Generic::UpdateRunner(), APoi_GTWS::UpdatePoi(), UseRoundedBkg_PoiGTWS(), ARunner_GTWS::ToggleClub(), ARunner_GTWS::ToggleFlag(), ARunner_GTWS::TogglePhoto() (+6 more)

### Community 128 - "UCameraControlComponent"
Cohesion: 0.15
Nodes (4): UCameraControlComponent, CameraComponent, OwlCapture, SpringArmComponent

### Community 131 - "FRunnerStackingConfig"
Cohesion: 0.18
Nodes (8): FRunnerStackingConfig, Gamma, HysteresisCm, RadiusFarCm, RadiusNearCm, UpdateIntervalSec, URunnerStackingSubsystem::Initialize(), URunnerStackingSubsystem::SetConfig()

### Community 132 - "FPointIndexSegment"
Cohesion: 0.18
Nodes (8): FPointIndexSegment, EndDistanceCm, EndIndex, Indices, StartDistanceCm, StartIndex, FRaceEntries, RacesEntries

### Community 134 - "FRaceCard"
Cohesion: 0.18
Nodes (5): FRaceCard, RaceId, Steps, Title, SplitTaskId()

### Community 135 - "UGTWS_NameComponent"
Cohesion: 0.17
Nodes (3): UGTWS_NameComponent, AppliedBkgColor, TextColor

### Community 136 - "FMinMax"
Cohesion: 0.22
Nodes (6): FMinMax, Max, Min, ACheckpoint::GetMinMax(), APoi::GetMinMax(), ARunner::GetRunnerMinMax()

### Community 137 - "FWaterMaskBounds"
Cohesion: 0.15
Nodes (10): UWaterMaskSubsystem::ComputeBounds(), UWaterMaskSubsystem::GetCacheFilePath(), UWaterMaskSubsystem::OnRaceWaterReady(), UWaterMaskSubsystem::RequestOverpass(), UWaterMaskSubsystem::SendOverpassRequest(), FWaterMaskBounds, East, North (+2 more)

### Community 138 - "FString"
Cohesion: 0.27
Nodes (6): ConvertRunnerJson(), ConvertRunnersJson(), URunnerSubsystem::PerformHttpRequestForRunner(), URunnerSubsystem::PerformHttpRequestForRunners(), URunnerSubsystem::RequestRunnersWithRetry(), URunnerSubsystem::ScheduleRunnersRetry()

### Community 139 - "FTrailLabelStyle"
Cohesion: 0.29
Nodes (4): FTrailLabelStyle, Color, DrawSize, Font

### Community 141 - "UPROPERTY"
Cohesion: 0.18
Nodes (9): FGroups, Groups, RaceId, FGroupStruct, groupId, Teams, FRaceTeamRunnerIndex, RunnerToTeam (+1 more)

### Community 143 - "FRaceTeamsRunners"
Cohesion: 0.25
Nodes (6): FRaceTeamsRunners, RaceId, TeamRunners, FTeamRunner, RunnerId, TeamId

### Community 144 - "WorldFlora.h"
Cohesion: 0.22
Nodes (5): FKindSet, Components, UHierarchicalInstancedStaticMeshComponent, UStaticMesh, UWorldAmbienceSubsystem

### Community 145 - "ARunner_Nike"
Cohesion: 0.18
Nodes (8): ARaceManager::GetRunnerClassForRace(), ARunner_Nike, ClubComponent, ClubFont, ClubHook, FlagComponent, LabelFont, NameComponent

### Community 146 - "AudioSanitizer.h"
Cohesion: 0.22
Nodes (5): ENaNHuntPhase, Done, Idle, Stop, USubmixEffectSanitizerPreset

### Community 147 - "BroadcastSettings.h"
Cohesion: 0.25
Nodes (3): UBroadcastSettings, bMirrorCaptureToViewport, UTextureRenderTarget2D

### Community 148 - "FRacePath"
Cohesion: 0.18
Nodes (6): FRacePath, Points, APath::BeginPlay(), APath::DrawPath(), APath::SetRacePath(), FGeoidGrid

### Community 150 - "DisplayController"
Cohesion: 0.54
Nodes (7): DisplayController(), cancelClose(), cancelTimer(), checkForIntent(), closeAnyDelayed(), closeTooltip(), openTooltip()

### Community 151 - "TrailHttpDebug.h"
Cohesion: 0.11
Nodes (9): FTrailHttpDebug::LogAndIsSuccess(), FTrailHttpDebug::LogStart(), IsLikelyBinaryContent(), FTrailHttpDebug, FTrailHttpDebugOptions, bLogBody, bLogHeaders, bSkipBodyIfBinary (+1 more)

### Community 154 - "FWaterPolygon"
Cohesion: 0.33
Nodes (3): FWaterPolygon, Bounds, Rings

### Community 155 - "FlagComponent.cpp"
Cohesion: 0.20
Nodes (3): UFlagComponent::UFlagComponent(), UFlagComponent::UpdateDayNight(), UFlagComponent::UpdateFlag()

### Community 156 - "FCheckpoints"
Cohesion: 0.25
Nodes (7): FCheckpoints, Checkpoints, ConvertCheckpointJson(), ConvertCheckpointsJson(), UCheckpointSubsystem::GetCheckpointsByRaceId(), UCheckpointSubsystem::PerformHttpRequestForCheckpoints(), APath::HandleCheckpointsDatasGathered()

### Community 157 - "WorldFauna.h"
Cohesion: 0.40
Nodes (3): UInstancedStaticMeshComponent, UStaticMesh, UWorldAmbienceSubsystem

### Community 159 - "FTeamStruct"
Cohesion: 0.22
Nodes (8): FTeams, RaceId, Teams, FTeamStruct, canal, canalId, RaceId, runnerId

### Community 160 - "PlacementCalculator"
Cohesion: 0.47
Nodes (5): isSvgElement(), PlacementCalculator(), computePlacementCoords(), getHtmlPlacement(), getSvgPlacement()

### Community 162 - "FRunnerState"
Cohesion: 0.33
Nodes (4): FRunnerState, CurrentOffset, CurrentScale, Runner

### Community 163 - "FPathSegment"
Cohesion: 0.25
Nodes (8): FPathSegment, bVisible, CheckpointIn, CheckpointOut, Color, KmIn, KmOut, Label

### Community 165 - "MakeStep"
Cohesion: 0.38
Nodes (6): SWidget, MakePill(), MakeStep(), Rounded(), SLoadingOverlay::Rebuild(), StateColor()

### Community 166 - "WorldAmbienceSubsystem.h"
Cohesion: 0.25
Nodes (6): UWorldAmbienceSubsystem::FindSunLight(), FHttpServerRequest, IHttpRouter, UDirectionalLightComponent, UMaterialParameterCollection, UVolumetricCloudComponent

### Community 167 - "UFlagComponent"
Cohesion: 0.14
Nodes (8): UFlagComponent, bIsDayCached, CurrentCountry, FlagMID, FlagSMC, UGTWS_FlagComponent, UNike_FlagComponent, UUTMB_FlagComponent

### Community 170 - "UGeneric_NameComponent"
Cohesion: 0.29
Nodes (3): UGeneric_NameComponent, NameLeftComponent, NameRightComponent

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

### Community 181 - "ClubComponent.h"
Cohesion: 0.06
Nodes (5): ARunner_Nike::UpdateRunner(), UGeneric_FlagComponent::UGeneric_FlagComponent(), UGTWS_FlagComponent::UGTWS_FlagComponent(), UNike_FlagComponent::UNike_FlagComponent(), UUTMB_FlagComponent::UUTMB_FlagComponent()

### Community 186 - "FState"
Cohesion: 0.25
Nodes (8): FState, bSpeedDownHeld, bSpeedUpHeld, HeightM, SpeedMult, Tight, ZoomMult, ResetPad()

### Community 189 - "ARaceManager::ToggleRace_Internal"
Cohesion: 0.14
Nodes (9): ARaceManager::EdToggleCheckpoints(), ARaceManager::EdTogglePath(), ARaceManager::EdTogglePois(), ARaceManager::EdToggleSlope(), ARaceManager::RtToggleCheckpoints(), ARaceManager::RtTogglePath(), ARaceManager::RtTogglePois(), ARaceManager::RtToggleSlope() (+1 more)

### Community 190 - "UUTMB_NameComponent"
Cohesion: 0.25
Nodes (3): UUTMB_NameComponent, RightSlashComponent, SplineComponent

### Community 191 - "FTrackSample"
Cohesion: 0.67
Nodes (3): FTrackSample, Dist, Time

### Community 194 - "BroadcastCaptureSubsystem.h"
Cohesion: 0.20
Nodes (5): UBroadcastCaptureSubsystem::RequestViewTarget(), UBroadcastCaptureSubsystem::Tick(), APlayerController, UGameViewportClient, UTextureRenderTarget2D

### Community 201 - "Automation_TrailSimulator.slnx"
Cohesion: 0.06
Nodes (35): Apple.Automation, AutomationTool, AutomationUtils.Automation, CookedEditor.Automation, CrowdinLocalization.Automation, EpicGames.Analytics.Telemetry, EpicGames.Build, EpicGames.Build.Tests (+27 more)

### Community 530 - "TrailSimulator.slnx"
Cohesion: 0.06
Nodes (34): Android.Automation, AutomationScripts.Automation, BuildGraph.Automation, EpicGames.Analytics.Generators, EpicGames.Analytics.PerformanceTrends, EpicGames.AspNet, EpicGames.Benchmarks, EpicGames.BuildGraph (+26 more)

### Community 538 - "UAudioSanitizerSubsystem::CheckNaN"
Cohesion: 0.40
Nodes (3): SoundName(), UAudioSanitizerSubsystem::CheckNaN(), UAudioComponent

## Knowledge Gaps
- **1172 isolated node(s):** `DisplayName`, `Min`, `Max`, `RaceID`, `RaceURL` (+1167 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 3161 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **387 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `FRaceSetup` connect `FRaceSetup` to `ARaceManager`, `ARunner`, `RaceManager.cpp`, `ACheckpoint`, `APoi`, `UCheckpointSubsystem`, `URunnerSubsystem`, `ARunner_Nike`, `URaceSubsystem`, `UPoiSubsystem`, `Checkpoint.cpp`, `UPathSubsystem`, `UTeamSubsystem`, `coreminimal`, `UWeatherSubsystem`, `FRaceStruct`, `RunnerSubsystem.cpp`, `ClubComponent.h`, `PoiSubsystem.cpp`, `FRaceCheckpoint`, `CheckpointSubsystem.cpp`, `PathSubsystem.cpp`, `APath::GetKmClassForRace`, `UTeamGroupSubsystem`, `ACheckpoint_GTWS`, `UTemplateSubsystem`, `TemplateSubsystem.cpp`, `APoi_UTMB`, `TeamSubsystem.cpp`, `UTrailsSubsystem`, `WeatherSubsystem.cpp`, `int64`, `RaceSubsystem.cpp`, `Checkpoint_GTWS.cpp`, `gameplaystatics`?**
  _High betweenness centrality (0.092) - this node is a cross-community bridge._
- **What connects `DisplayName`, `Min`, `Max` to the rest of the system?**
  _1172 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ARaceManager` be split into smaller, more focused modules?**
  _Cohesion score 0.010101010101010102 - nodes in this community are weakly interconnected._
- **Why does `ARaceManager` connect `ARaceManager` to `.SlateNotify`, `FPointIndexSegment`, `APath`, `RaceManager.cpp`, `FRunnerStruct`, `.GetWorld`, `FRacePath`, `ARaceManager::EndPlay`, `ARaceManager::RtMoveToPath`, `ARaceManager::RtToggleCheckpointWeather`, `ARaceManager::RtUpdateGlowIntensity`, `ARaceManager::RtUpdatePulseGlow`, `FPathSegment`, `FRaceSetup`, `FRaceStruct`, `ARaceManager::RtToggleCheckpoint`, `ARaceManager::ToggleRace_Internal`, `ARaceManager::SpawnInitialRunners`, `Km.cpp`, `int64`, `CallBP_GetFloatOut`, `gameplaystatics`?**
  _High betweenness centrality (0.083) - this node is a cross-community bridge._
- **Should `UHttpGatewaySubsystem` be split into smaller, more focused modules?**
  _Cohesion score 0.017241379310344827 - nodes in this community are weakly interconnected._
- **Why does `ARunner` connect `ARunner` to `ICameraControlInterface`, `APoi`, `FMinMax`, `FRunnerStruct`, `Runner.cpp`, `ARunner_Nike`, `Runner.h`, `FRunnerState`, `FRaceSetup`, `IRunnerStackableInterface`, `ARunner_UTMB`, `FTrackSample`, `IRunnerInterface`, `ARunner_GTWS`, `BroadcastCaptureSubsystem.cpp`, `.GetFootHook`, `.GetFootMesh`, `ARunner_Generic`, `gameplaystatics`?**
  _High betweenness centrality (0.058) - this node is a cross-community bridge._
- **Should `.SlateNotify` be split into smaller, more focused modules?**
  _Cohesion score 0.033783783783783786 - nodes in this community are weakly interconnected._