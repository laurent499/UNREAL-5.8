# Graph Report - UNREAL 5.8  (2026-10-16)

## Corpus Check
- 537 files · ~1,775,229 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5284 file(s) not represented in the graph (top: .uasset 4702, .tex 369, .eps 71)

## Summary
- 4683 nodes · 6806 edges · 542 communities (168 shown, 374 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 543 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `f6ff183b`
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
- gameplaystatics
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
- RawTextMaterial.h
- UWaterMaskSubsystem
- UAudioSanitizerSubsystem
- Path.cpp
- Poi.cpp
- coreminimal
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
- PoiSubsystem.cpp
- WaterMaskSubsystem.cpp
- ScaleSubsystem.cpp
- ARunner_UTMB
- AudioSanitizer.cpp
- FRaceCheckpoint
- FOpenWeatherResponse
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
- Km.h
- PathSegmentsComponent.cpp
- WorldHeadlamps.cpp
- UMeteoPageSubsystem
- UTeamGroupSubsystem
- NameComponent.cpp
- ACheckpoint_GTWS
- ACheckpoint_UTMB
- UTemplateSubsystem
- TemplateSubsystem.cpp
- OverpassToFeatures
- TrailsSubsystem.cpp
- .GetWorld
- WorldAmbienceSubsystem.h
- UWorld
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
- ClubComponent.cpp
- USettingsSaveGame
- int64
- FTrailTemplateRunner
- FBenchShot
- FSubmixEffectSanitizer
- FDistanceScaleConfig
- FStackEntry
- TemplateSubsystem.h
- CallBP_GetFloatOut
- Runner_GTWS.cpp
- UCameraControlComponent
- SlateNotificationsBFL.h
- CesiumBudgetSubsystem.cpp
- FRunnerStackingConfig
- TeamGroupSubsystem.cpp
- ICameraControlInterface
- FRaceCard
- UGTWS_NameComponent
- FWaterMaskBounds
- UWaterMaskSubsystem::RequestOverpass
- FString
- FTrailLabelStyle
- FRaceTeamRunnerIndex
- ICheckpointInterface
- FRaceTeamsRunners
- WorldFlora.h
- ARunner_Nike
- AudioSanitizer.h
- BroadcastSettings.h
- APath::DrawPath
- ScaleSubsystem.h
- DisplayController
- TrailHttpDebug.h
- UTeamSubsystem::GetRunnerActorByRaceTeam
- FWaterPolygon
- FlagComponent.cpp
- FTrailHttpDebug::LogAndIsSuccess
- UMaterialInstanceDynamic
- IPoiInterface
- TeamInterface.h
- PlacementCalculator
- NameComponent.h
- FRunnerState
- httpmodule
- IPathInterface
- MakeStep
- TrailUtilsLog.h
- UFlagComponent
- TrailSharedTypes.h
- UWorldAmbienceSubsystem::OnWorldBeginPlay
- UGeneric_NameComponent
- FPendingCell
- EKind
- UWorldAmbienceSubsystem::UpdateState
- IRunnerStackableInterface
- TSubclassOf
- dynsections.js
- UWorldAmbienceSubsystem::Initialize
- AWorldFlora::ContinueBuild
- AWorldFlora::Tick
- EBenchPhase
- FlagComponent.h
- ARaceManager::RtToggleCheckpoint
- Generic_FlagComponent.h
- makeTree
- ARaceManager::BeginPlay
- FState
- CLAUDE.md
- Piecewise
- ARaceManager::ToggleRace_Internal
- UUTMB_NameComponent
- FTrackSample
- cookie.js
- .mcp.json
- UBroadcastCaptureSubsystem::RequestViewTarget
- URunnerSubsystem::Deinitialize
- USettingsSubsystem::Initialize
- USettingsSubsystem::Save
- Automation_TrailSimulator.slnx
- TrailSimulator.slnx
- BuildMesh
- Nike_FlagComponent.h
- UTMB_FlagComponent.h
- ActionValue
- Km_Generic.h
- Km_Nike.h
- UPoiSubsystem::SpawnPoi
- UAudioSanitizerSubsystem::CheckNaN
- FRunners
- ARunner::UpdateRunnerLocation
- ARaceManager::HideLoadingOverlay

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

## Communities (542 total, 374 thin omitted)

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
Nodes (56): UWorldAmbienceSubsystem::TickBench(), UWorldAmbienceSubsystem, AppliedWeather, AutoWeather, bBegunPlay, bBenchGrounded, BenchCsvPath, BenchFrameSamples (+48 more)

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
Cohesion: 0.05
Nodes (5): FSlateNotificationsModule, FTemplateGeneratorModule, FSharedTypesModule, FTrailInterfacesModule, FTrailUtilsModule

### Community 13 - "URunnerSubsystem"
Cohesion: 0.04
Nodes (16): URunnerSubsystem, ActiveRunnerRequests, CurrentRaceSetup, CurrentRunnerId, MaxRunnersFetchAttempts, OnRunnerDatasGathered, OnRunnersDatasGathered, OnRunnersUpdateDatas (+8 more)

### Community 14 - "FRunnerStruct"
Cohesion: 0.10
Nodes (21): FRunnerStruct, canal, canalId, club, dossard, elevation, IndexM, kmActuel (+13 more)

### Community 15 - "gameplaystatics"
Cohesion: 0.12
Nodes (5): ACheckpoint_UTMB::BeginPlay(), ACheckpoint_UTMB::UpdateCheckpoint(), APoi_UTMB::BeginPlay(), APoi_UTMB::UpdatePoi(), ARunner_UTMB::BeginPlay()

### Community 16 - "Runner.cpp"
Cohesion: 0.05
Nodes (10): ARunner::AdvanceTrackInterp(), ARunner::AssignRunnerToTeam(), ARunner::GetStackAttachComponent(), ARunner::GetStackHookComponent(), ARunner::HandleTemplateChanged(), ARunner::OnLookAtTimerTick(), ARunner::SetMinMax(), ARunner::TriggerUpdateAfterHidden() (+2 more)

### Community 17 - "UNameComponent"
Cohesion: 0.08
Nodes (11): UNameComponent, bShow3DText, bShowWidgetText, LabelFont, LastLocation, NameBkgComponent, NameFont, NameSplineComponent (+3 more)

### Community 18 - "SearchBox"
Cohesion: 0.13
Nodes (7): createResults(), init_search(), SearchBox(), getXPos(), getYPos(), searchResults, convertToId()

### Community 19 - "UTMB_NameComponent.cpp"
Cohesion: 0.22
Nodes (3): UUTMB_NameComponent::UpdateMesh(), UUTMB_NameComponent::UpdateName(), UUTMB_NameComponent::UUTMB_NameComponent()

### Community 20 - "AWorldFauna"
Cohesion: 0.06
Nodes (22): AWorldFauna::Tick(), AWorldFauna, Ambience, Anchor, AnchorGroundZ, AnchorTimer, AppliedDensity, bHasAnchor (+14 more)

### Community 21 - "ULoadingStatusSubsystem"
Cohesion: 0.08
Nodes (12): Clamp01(), ULoadingStatusSubsystem::Complete(), ULoadingStatusSubsystem::Fail(), ULoadingStatusSubsystem::MarkDirty(), ULoadingStatusSubsystem::RegisterTask(), ULoadingStatusSubsystem::SetRunning(), ULoadingStatusSubsystem::Update(), ULoadingStatusSubsystem (+4 more)

### Community 22 - "jquery.js"
Cohesion: 0.08
Nodes (13): at(), getEventsNS(), getViewportDimensions(), initMouseDetection(), initTracking(), isTouchEvent(), lt(), trackMouse() (+5 more)

### Community 23 - "URaceSubsystem"
Cohesion: 0.06
Nodes (13): UCheckpointSubsystem::Initialize(), UPoiSubsystem::Initialize(), URunnerSubsystem::Initialize(), UTeamGroupSubsystem::Initialize(), UTeamSubsystem::Initialize(), URaceSubsystem, CurrentRaceId, CurrentRaceSetup (+5 more)

### Community 24 - "UPoiSubsystem"
Cohesion: 0.06
Nodes (10): UPoiSubsystem, ActivePoiRequests, CurrentRaceSetup, OnPoiDatasGathered, OnPoisDatasGathered, POI, POIs, PoisActorsMap (+2 more)

### Community 25 - "AWorldFlora"
Cohesion: 0.06
Nodes (21): AWorldFlora, AllComponents, Ambience, bBuilding, bShown, BuildCenter, BuildDensity, BuildRadius (+13 more)

### Community 26 - "Checkpoint.cpp"
Cohesion: 0.05
Nodes (6): ACheckpoint::ACheckpoint(), ACheckpoint::CapitalizeFirst(), ACheckpoint::GetBoundsLocation(), ACheckpoint::HandleTemplateChanged(), ACheckpoint::OnLookAtTimerTick(), ACheckpoint::SetMinMax()

### Community 27 - "RawTextMaterial.h"
Cohesion: 0.24
Nodes (4): Apply(), ApplyToAllTexts(), EnableOcclusion(), GetMaterial()

### Community 28 - "UWaterMaskSubsystem"
Cohesion: 0.09
Nodes (8): UWaterMaskSubsystem::HandlePathDatasGathered(), UWaterMaskSubsystem, ActiveRequests, bWaterPolygonsDirty, MaterialLayerKey, PathSubsystem, RaceFeatures, WaterPolygons

### Community 29 - "UAudioSanitizerSubsystem"
Cohesion: 0.06
Nodes (19): UAudioSanitizerSubsystem, bCulpritRetrying, BeginPlayTime, bSoundOpened, CheckTimer, ClippedSinceLog, Culprit, CulpritRetryDelay (+11 more)

### Community 30 - "Path.cpp"
Cohesion: 0.09
Nodes (12): APath::BuildKms(), APath::HandleCheckpointsDatasGathered(), APath::RebuildPathSplineMeshes(), APath::RebuildSlopeSplineMeshes(), APath::StartPulse(), APath::StartTravelBackward(), APath::StartTravelForward(), APath::StopTravel() (+4 more)

### Community 31 - "Poi.cpp"
Cohesion: 0.07
Nodes (5): APoi::CapitalizeFirst(), APoi::GetBoundsLocation(), APoi::HandleTemplateChanged(), APoi::OnLookAtTimerTick(), APoi::SetMinMax()

### Community 33 - "splinemeshcomponent"
Cohesion: 0.08
Nodes (10): ACheckpoint_Nike::UpdateCheckpoint(), APoi_Nike::UpdatePoi(), UGeneric_NameComponent::GetMiddle(), UGeneric_NameComponent::UGeneric_NameComponent(), UGeneric_NameComponent::UpdateName(), UNike_ClubComponent::UNike_ClubComponent(), UNike_ClubComponent::UpdateClub(), UNike_NameComponent::UNike_NameComponent() (+2 more)

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
Cohesion: 0.12
Nodes (6): UWeatherSubsystem, ActiveWeatherRequests, CurrentRaceSetup, OnWeatherGathered, RaceSubsystem, WeatherResponse

### Community 41 - "AKm"
Cohesion: 0.09
Nodes (11): AKm, DynaPawn, Georeference, GlobeAnchorComponent, LabelComp, LookAtTimerHandle, MainRoot, RaceManager (+3 more)

### Community 42 - "FRaceSetup"
Cohesion: 0.09
Nodes (26): FRaceSetup, arrivalCity, arrivalCountry, arrivalCountryFlag, cat, color, departureCity, departureCountry (+18 more)

### Community 43 - "unrealbuildtool"
Cohesion: 0.11
Nodes (9): SlateNotifications, TemplateGenerator, SharedTypes, Subsystems, TrailInterfaces, TrailSimulatorTarget, TrailSimulator, TrailSimulatorEditorTarget (+1 more)

### Community 44 - "UPROPERTY"
Cohesion: 0.09
Nodes (22): FCheckpoints, Checkpoints, FGroups, Groups, RaceId, FGroupStruct, groupId, Teams (+14 more)

### Community 45 - "UBroadcastCaptureSubsystem"
Cohesion: 0.07
Nodes (12): UBroadcastCaptureSubsystem, Accum, ActiveSource, CachedPC, DefaultSource, MirrorViewport, MirrorWidget, MonitorHz (+4 more)

### Community 46 - "FAutoWeatherInfo"
Cohesion: 0.08
Nodes (24): FAutoWeatherInfo, AgeMinutes, bHasData, CameraAltitudeM, CameraTempC, CloudBaseM, Clouds, Description (+16 more)

### Community 47 - "UClubComponent"
Cohesion: 0.05
Nodes (15): UClubComponent, bShow3DText, bShowWidgetText, ClubBkgComponent, ClubFont, ClubSplineComponent, ClubTextComponent, LastLocation (+7 more)

### Community 48 - "FSettings"
Cohesion: 0.07
Nodes (26): FMinMax, Max, Min, FPointIndexSegment, EndDistanceCm, EndIndex, Indices, StartDistanceCm (+18 more)

### Community 49 - "RunnerSubsystem.cpp"
Cohesion: 0.18
Nodes (16): URunnerSubsystem::Animation(), URunnerSubsystem::AnimationByTeam(), URunnerSubsystem::CancelRunnersRetry(), URunnerSubsystem::DoesRunnerBelongsToRace(), URunnerSubsystem::GetRunnerActor(), URunnerSubsystem::GetRunnerActorByTeam(), URunnerSubsystem::GetRunnersByRaceId(), URunnerSubsystem::RemoveTeamsNotInSnapshot() (+8 more)

### Community 50 - "UTrailTextWidgetComponent"
Cohesion: 0.09
Nodes (7): UTrailTextWidgetComponent, CurrentColor, CurrentFont, CurrentJustification, CurrentSize, CurrentText, SlateTextBlock

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
Cohesion: 0.25
Nodes (11): ConvertPOIJson(), ConvertPOIsJson(), UPoiSubsystem::Animation(), UPoiSubsystem::DoesPoiBelongsToRace(), UPoiSubsystem::PerformHttpRequestForPOIs(), UPoiSubsystem::SetRaceSetup(), UPoiSubsystem::StopAnimation(), UPoiSubsystem::Teleport() (+3 more)

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
Cohesion: 0.14
Nodes (6): DayPhase(), FSubmixEffectSanitizer::OnProcessAudio(), SetWorldMute(), UAudioSanitizerSubsystem::Deinitialize(), UAudioSanitizerSubsystem::Initialize(), UAudioSanitizerSubsystem::UpdateForest()

### Community 59 - "FRaceCheckpoint"
Cohesion: 0.14
Nodes (14): FRaceCheckpoint, altitude, checkpointId, distance, elevation, name, type, weather (+6 more)

### Community 60 - "FOpenWeatherResponse"
Cohesion: 0.12
Nodes (14): FOpenWeatherResponse, current, lat, lon, timezone, timezone_offset, FWeatherResult, bSuccess (+6 more)

### Community 61 - "TrailTextWidgetComponent.cpp"
Cohesion: 0.12
Nodes (9): UTrailTextWidgetComponent::GetFontInfo(), UTrailTextWidgetComponent::OnRegister(), UTrailTextWidgetComponent::SetLabelCasseToLower(), UTrailTextWidgetComponent::SetLabelCasseToUpper(), UTrailTextWidgetComponent::SetLabelColor(), UTrailTextWidgetComponent::SetLabelFont(), UTrailTextWidgetComponent::SetLabelHJustification(), UTrailTextWidgetComponent::SetLabelText() (+1 more)

### Community 62 - "WorldFlora.cpp"
Cohesion: 0.12
Nodes (6): AWorldFlora::BeginPlay(), AWorldFlora::HandlePathGathered(), AWorldFlora::IsOnTrail(), AWorldFlora::StartBuild(), AWorldFlora::TraceGround(), Hash()

### Community 63 - "SLoadingOverlay.cpp"
Cohesion: 0.11
Nodes (5): Font(), FStepDef, Label, Prefix, SLoadingOverlay::GetOverallPercent()

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

### Community 69 - "FTrailTemplate"
Cohesion: 0.06
Nodes (34): FTrailTemplate, Base, Checkpoint, Globals, Name, Poi, Runner, FTrailTemplateGlobals (+26 more)

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
Cohesion: 0.10
Nodes (14): ACheckpoint_Nike, Infos1BkgComponent, Infos1SplineComponent, Infos1WidgetComponent, Infos2BkgComponent, Infos2SplineComponent, Infos2WidgetComponent, InfosFont (+6 more)

### Community 76 - "initResizable"
Cohesion: 0.16
Nodes (10): initNavTree(), initPageToc(), render(), initResizable(), constrainPanelWidths(), resizeHeight(), resizeWidth(), restoreWidth() (+2 more)

### Community 77 - "UPhotoComponent"
Cohesion: 0.12
Nodes (7): UPhotoComponent, BaseMat, CurrentPhotoTexture, CurrentPhotoUrl, PendingPhotoUrl, PhotoMID, PhotoSMC

### Community 79 - "APoi_GTWS"
Cohesion: 0.14
Nodes (9): APoi_GTWS, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, RaceManager, WeatherPictoMat (+1 more)

### Community 80 - "AWorldHeadlamps"
Cohesion: 0.13
Nodes (7): AWorldHeadlamps::Tick(), AWorldHeadlamps, Ambience, Lamps, LitCount, RefreshTimer, Runners

### Community 81 - "APoi_Nike"
Cohesion: 0.12
Nodes (13): APoi_Nike, Infos1BkgComponent, Infos1SplineComponent, Infos1WidgetComponent, Infos2BkgComponent, Infos2SplineComponent, Infos2WidgetComponent, InfosFont (+5 more)

### Community 82 - "MeteoPageSubsystem.cpp"
Cohesion: 0.09
Nodes (9): UHttpGatewaySubsystem::HandleCommandRequest(), UHttpGatewaySubsystem::Initialize(), FindGeoreference(), GeoJson(), JsonResponse(), UMeteoPageSubsystem::HandleGeoRequest(), UMeteoPageSubsystem::HandleGeoSetRequest(), UMeteoPageSubsystem::HandlePageRequest() (+1 more)

### Community 83 - "UPathSegmentsComponent"
Cohesion: 0.05
Nodes (23): FPathSegment, bVisible, Color, KmIn, KmOut, Label, UPathSegmentsComponent::Apply(), UPathSegmentsComponent::RestoreTrace() (+15 more)

### Community 84 - "Ee"
Cohesion: 0.16
Nodes (16): A(), b(), be(), Ee(), f(), he(), je(), k() (+8 more)

### Community 85 - "e"
Cohesion: 0.19
Nodes (16): ce(), e(), fe(), H(), I(), l(), le(), M() (+8 more)

### Community 86 - "PathSubsystem.cpp"
Cohesion: 0.21
Nodes (7): ConvertPathJson(), UPathSubsystem::GetPathById(), UPathSubsystem::Initialize(), UPathSubsystem::PerformHttpRequestForPath(), UPathSubsystem::SetRaceSetup(), UPathSubsystem::SpawnKmActor(), UPathSubsystem::SpawnPathActor()

### Community 87 - "ARaceManager::SpawnInitialRunners"
Cohesion: 0.31
Nodes (4): ARaceManager::ProcessPendingRunnerUpdates(), ARaceManager::SpawnInitialRunners(), ARaceManager::UpdateRunnersFromSnapshot(), LogRaceTeamIndex()

### Community 88 - "GTWS_NameComponent.cpp"
Cohesion: 0.10
Nodes (10): FStartAndEnd, EndLocation, StartLocation, UClubComponent::GetStartAndEnd(), UGTWS_NameComponent::GetSplineLastPoint(), UGTWS_NameComponent::GetStartAndEnd(), UGTWS_NameComponent::SetColors(), UGTWS_NameComponent::UGTWS_NameComponent() (+2 more)

### Community 89 - "RunnerStackingSubsystem.cpp"
Cohesion: 0.11
Nodes (13): URunnerStackingSubsystem::ApplyNewStackingState(), URunnerStackingSubsystem::AttachRunnerToBase(), URunnerStackingSubsystem::ComputeDynamicRadiusCm(), URunnerStackingSubsystem::ComputeOverlapRadiusCm(), URunnerStackingSubsystem::Deinitialize(), URunnerStackingSubsystem::DetachRunnerToTrack(), URunnerStackingSubsystem::FindEntryIndex(), URunnerStackingSubsystem::HandleActorEndPlay() (+5 more)

### Community 90 - "APoi_Generic"
Cohesion: 0.12
Nodes (11): APoi_Generic, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, NameLeftComponent, NameRightComponent (+3 more)

### Community 91 - "SBroadcastMirror"
Cohesion: 0.14
Nodes (5): SBroadcastMirror, Background, bLogged, Brush, Target

### Community 92 - "Km.h"
Cohesion: 0.12
Nodes (3): IKmInterface, AKm_GTWS, AKm_UTMB

### Community 93 - "PathSegmentsComponent.cpp"
Cohesion: 0.12
Nodes (7): UGTWS_ClubComponent::SetColors(), UGTWS_ClubComponent::UGTWS_ClubComponent(), UGTWS_ClubComponent::UpdateClub(), UPathSegmentsComponent::BeginPlay(), UPathSegmentsComponent::ConfigurePiece(), UPathSegmentsComponent::CreatePiece(), UPathSegmentsComponent::OnLookAtTimerTick()

### Community 94 - "WorldHeadlamps.cpp"
Cohesion: 0.15
Nodes (4): AWorldHeadlamps::BeginPlay(), AWorldHeadlamps::GetLamp(), UPointLightComponent, UWorldAmbienceSubsystem

### Community 95 - "UMeteoPageSubsystem"
Cohesion: 0.10
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
Nodes (10): ACheckpoint_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, DistPictoMID, MainPictoMat, MainPictoMID, RaceManager (+2 more)

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
Cohesion: 0.24
Nodes (4): ConvertRacesJson(), UTrailsSubsystem::Initialize(), UTrailsSubsystem::PerformHttpRequestForRaces(), UTrailsSubsystem::SetRaceSetup()

### Community 104 - ".GetWorld"
Cohesion: 0.11
Nodes (24): URunnerSubsystem::CancelAllRunnersRetries(), UWorldAmbienceSubsystem::DoesSupportWorldType(), Type, ITrailInputModeInterface, ACheckpoint::BeginPlay(), ACheckpoint::EndPlay(), ACheckpoint::StartAnimation(), ACheckpoint::StopAnimation() (+16 more)

### Community 105 - "WorldAmbienceSubsystem.h"
Cohesion: 0.07
Nodes (26): UWorldAmbienceSubsystem::FindSunLight(), FBenchResult, FrameAvg, FrameP99, GpuAvg, GpuP99, Name, Samples (+18 more)

### Community 106 - "UWorld"
Cohesion: 0.15
Nodes (16): FindPlayWorld(), FindActorByClassToken(), ReadNumber(), SetPawnInput(), UAudioSanitizerSubsystem::OnWorldBeginPlay(), UAudioSanitizerSubsystem::ShouldCreateSubsystem(), FWorldUtils::GetGameInstance(), FWorldUtils::GetGameInstanceChecked() (+8 more)

### Community 107 - "ARunner_Generic"
Cohesion: 0.14
Nodes (7): ARunner_Generic, FlagComponent, NameComponent, PhotoComponent, PhotoHook, RaceManager, SlashComponent

### Community 108 - "APoi_UTMB"
Cohesion: 0.14
Nodes (9): APoi_UTMB, AltPictoMat, AltPictoMID, DistPictoMat, MainPictoMat, MainPictoMID, RaceManager, WeatherPictoMat (+1 more)

### Community 109 - "TeamSubsystem.cpp"
Cohesion: 0.23
Nodes (13): UCesiumFlyToComponent, UTeamSubsystem::Animation(), UTeamSubsystem::ClearRaceTeamIndex(), UTeamSubsystem::DisplayClub(), UTeamSubsystem::DisplayFlag(), UTeamSubsystem::DisplayPhoto(), UTeamSubsystem::DisplayTeams(), UTeamSubsystem::GetRunnerByRaceTeam() (+5 more)

### Community 110 - "Km.cpp"
Cohesion: 0.14
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

### Community 117 - "ClubComponent.cpp"
Cohesion: 0.18
Nodes (4): UClubComponent::GetLocalSize(), UClubComponent::UClubComponent(), UClubComponent::UpdateClub(), UClubComponent::UpdateMesh()

### Community 118 - "USettingsSaveGame"
Cohesion: 0.15
Nodes (7): USettingsSubsystem::Load(), USettingsSubsystem::SetMainURL(), USettingsSaveGame, GlobalLength, GlobalPitch, GlobalZAnchor, MainURL

### Community 119 - "int64"
Cohesion: 0.08
Nodes (25): FRaceEntry, name, raceId, templateName, FRaceLoaded, RaceID, RaceName, RaceTemplate (+17 more)

### Community 120 - "FTrailTemplateRunner"
Cohesion: 0.15
Nodes (13): FTrailTemplateRunner, BkgLeftMargin, BkgRightMargin, ClubBkg, ClubText, Foot, Height, NameBkg (+5 more)

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

### Community 125 - "TemplateSubsystem.h"
Cohesion: 0.09
Nodes (9): UseRoundedBkg_PoiGeneric(), ACheckpoint_GTWS::BeginPlay(), ACheckpoint_GTWS::UpdateCheckpoint(), UseRoundedBkg_CheckpointGTWS(), APoi_GTWS::BeginPlay(), UseRoundedBkg_PoiGTWS(), FitLine(), GetBkgColor() (+1 more)

### Community 126 - "CallBP_GetFloatOut"
Cohesion: 0.18
Nodes (7): ARaceManager::EdUpdateMainUrl(), ARaceManager::HandleRacesDatasGathered(), ARaceManager::RtUpdateMainUrl(), ARaceManager::UpdateMainUrl_Internal(), ARaceManager::UpdateUdsTime(), CallBP_GetFloatOut(), SanitizeKey()

### Community 127 - "Runner_GTWS.cpp"
Cohesion: 0.11
Nodes (12): ARunner_Generic::BeginPlay(), ARunner_Generic::ToggleFlag(), ARunner_Generic::TogglePhoto(), ARunner_Generic::UpdateRunner(), ARunner_GTWS::BeginPlay(), ARunner_GTWS::ToggleClub(), ARunner_GTWS::ToggleFlag(), ARunner_GTWS::TogglePhoto() (+4 more)

### Community 128 - "UCameraControlComponent"
Cohesion: 0.15
Nodes (4): UCameraControlComponent, CameraComponent, OwlCapture, SpringArmComponent

### Community 131 - "FRunnerStackingConfig"
Cohesion: 0.18
Nodes (8): FRunnerStackingConfig, Gamma, HysteresisCm, RadiusFarCm, RadiusNearCm, UpdateIntervalSec, URunnerStackingSubsystem::Initialize(), URunnerStackingSubsystem::SetConfig()

### Community 132 - "TeamGroupSubsystem.cpp"
Cohesion: 0.33
Nodes (5): UTeamGroupSubsystem::AddTeamToGroup(), UTeamGroupSubsystem::GetTeamsInGroup(), UTeamGroupSubsystem::RemoveTeamFromGroup(), UTeamGroupSubsystem::SetRaceSetup(), UTeamGroupSubsystem::ToggleGroup()

### Community 134 - "FRaceCard"
Cohesion: 0.18
Nodes (5): FRaceCard, RaceId, Steps, Title, SplitTaskId()

### Community 135 - "UGTWS_NameComponent"
Cohesion: 0.17
Nodes (3): UGTWS_NameComponent, AppliedBkgColor, TextColor

### Community 136 - "FWaterMaskBounds"
Cohesion: 0.29
Nodes (7): UWaterMaskSubsystem::ComputeBounds(), UWaterMaskSubsystem::GetCacheFilePath(), FWaterMaskBounds, East, North, South, West

### Community 137 - "UWaterMaskSubsystem::RequestOverpass"
Cohesion: 0.29
Nodes (3): UWaterMaskSubsystem::OnRaceWaterReady(), UWaterMaskSubsystem::RequestOverpass(), UWaterMaskSubsystem::SendOverpassRequest()

### Community 138 - "FString"
Cohesion: 0.27
Nodes (6): ConvertRunnerJson(), ConvertRunnersJson(), URunnerSubsystem::PerformHttpRequestForRunner(), URunnerSubsystem::PerformHttpRequestForRunners(), URunnerSubsystem::RequestRunnersWithRetry(), URunnerSubsystem::ScheduleRunnersRetry()

### Community 139 - "FTrailLabelStyle"
Cohesion: 0.29
Nodes (4): FTrailLabelStyle, Color, DrawSize, Font

### Community 141 - "FRaceTeamRunnerIndex"
Cohesion: 0.40
Nodes (4): FRaceTeamRunnerIndex, RunnerToTeam, TeamToRunner, UTeamSubsystem::GetTeamsForRace()

### Community 143 - "FRaceTeamsRunners"
Cohesion: 0.25
Nodes (6): FRaceTeamsRunners, RaceId, TeamRunners, FTeamRunner, RunnerId, TeamId

### Community 144 - "WorldFlora.h"
Cohesion: 0.22
Nodes (5): FKindSet, Components, UHierarchicalInstancedStaticMeshComponent, UStaticMesh, UWorldAmbienceSubsystem

### Community 145 - "ARunner_Nike"
Cohesion: 0.22
Nodes (7): ARunner_Nike, ClubComponent, ClubFont, ClubHook, FlagComponent, LabelFont, NameComponent

### Community 146 - "AudioSanitizer.h"
Cohesion: 0.22
Nodes (5): ENaNHuntPhase, Done, Idle, Stop, USubmixEffectSanitizerPreset

### Community 147 - "BroadcastSettings.h"
Cohesion: 0.25
Nodes (3): UBroadcastSettings, bMirrorCaptureToViewport, UTextureRenderTarget2D

### Community 148 - "APath::DrawPath"
Cohesion: 0.24
Nodes (3): APath::BeginPlay(), APath::DrawPath(), FGeoidGrid

### Community 150 - "DisplayController"
Cohesion: 0.54
Nodes (7): DisplayController(), cancelClose(), cancelTimer(), checkForIntent(), closeAnyDelayed(), closeTooltip(), openTooltip()

### Community 151 - "TrailHttpDebug.h"
Cohesion: 0.18
Nodes (6): FTrailHttpDebug, FTrailHttpDebugOptions, bLogBody, bLogHeaders, bSkipBodyIfBinary, MaxBodyChars

### Community 152 - "UTeamSubsystem::GetRunnerActorByRaceTeam"
Cohesion: 0.33
Nodes (3): UTeamSubsystem::DisplayTeam(), UTeamSubsystem::GetRunnerActorByRaceTeam(), UTeamSubsystem::Teleport()

### Community 154 - "FWaterPolygon"
Cohesion: 0.33
Nodes (3): FWaterPolygon, Bounds, Rings

### Community 155 - "FlagComponent.cpp"
Cohesion: 0.20
Nodes (3): UFlagComponent::UFlagComponent(), UFlagComponent::UpdateDayNight(), UFlagComponent::UpdateFlag()

### Community 156 - "FTrailHttpDebug::LogAndIsSuccess"
Cohesion: 0.27
Nodes (3): FTrailHttpDebug::LogAndIsSuccess(), FTrailHttpDebug::LogStart(), IsLikelyBinaryContent()

### Community 157 - "UMaterialInstanceDynamic"
Cohesion: 0.25
Nodes (6): AWorldFauna::BuildMeshes(), UInstancedStaticMeshComponent, UMaterialInstanceDynamic, UStaticMesh, UWorldAmbienceSubsystem, UPhotoComponent::EnsurePhotoMID()

### Community 160 - "PlacementCalculator"
Cohesion: 0.47
Nodes (5): isSvgElement(), PlacementCalculator(), computePlacementCoords(), getHtmlPlacement(), getSvgPlacement()

### Community 162 - "FRunnerState"
Cohesion: 0.33
Nodes (4): FRunnerState, CurrentOffset, CurrentScale, Runner

### Community 165 - "MakeStep"
Cohesion: 0.38
Nodes (6): SWidget, MakePill(), MakeStep(), Rounded(), SLoadingOverlay::Rebuild(), StateColor()

### Community 167 - "UFlagComponent"
Cohesion: 0.18
Nodes (5): UFlagComponent, bIsDayCached, CurrentCountry, FlagMID, FlagSMC

### Community 170 - "UGeneric_NameComponent"
Cohesion: 0.22
Nodes (3): UGeneric_NameComponent, NameLeftComponent, NameRightComponent

### Community 171 - "FPendingCell"
Cohesion: 0.40
Nodes (3): FPendingCell, X, Y

### Community 172 - "EKind"
Cohesion: 0.40
Nodes (5): EKind, Count, Grass, Rock, Shrub

### Community 175 - "TSubclassOf"
Cohesion: 0.36
Nodes (6): APath::GetCheckpointClassForRace(), APath::GetKmClassForRace(), ARaceManager::GetPoiClassForRace(), ARaceManager::GetRunnerClassForRace(), ARaceManager::TrackRunnerInterp(), TSubclassOf

### Community 176 - "dynsections.js"
Cohesion: 0.50
Nodes (3): codefold, dynsection, toggleVisibility()

### Community 180 - "EBenchPhase"
Cohesion: 0.50
Nodes (4): EBenchPhase, Idle, Measuring, Settling

### Community 183 - "Generic_FlagComponent.h"
Cohesion: 0.25
Nodes (3): UGeneric_FlagComponent::UGeneric_FlagComponent(), UGeneric_FlagComponent, FlagHook

### Community 185 - "ARaceManager::BeginPlay"
Cohesion: 0.27
Nodes (3): ARaceManager::BeginPlay(), ARaceManager::EdTeleportToCheckpoint(), ARaceManager::RtTeleportToCheckpoint()

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

### Community 194 - "UBroadcastCaptureSubsystem::RequestViewTarget"
Cohesion: 0.40
Nodes (3): UBroadcastCaptureSubsystem::RequestViewTarget(), UBroadcastCaptureSubsystem::Tick(), APlayerController

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
- **1157 isolated node(s):** `TeamParentClass`, `PoiParentClass`, `CheckpointParentClass`, `Queued`, `Running` (+1152 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 3113 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **374 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `FRaceSetup` connect `FRaceSetup` to `ARaceManager`, `ARunner`, `TeamGroupSubsystem.cpp`, `RaceManager.cpp`, `ACheckpoint`, `APoi`, `UCheckpointSubsystem`, `URunnerSubsystem`, `gameplaystatics`, `URaceSubsystem`, `UPoiSubsystem`, `splinemeshcomponent`, `UPathSubsystem`, `UTeamSubsystem`, `TrailSharedTypes.h`, `UWeatherSubsystem`, `UPROPERTY`, `TSubclassOf`, `RunnerSubsystem.cpp`, `PoiSubsystem.cpp`, `FRaceCheckpoint`, `CheckpointSubsystem.cpp`, `FTrailTemplate`, `PathSubsystem.cpp`, `UTeamGroupSubsystem`, `UTemplateSubsystem`, `TemplateSubsystem.cpp`, `TrailsSubsystem.cpp`, `TeamSubsystem.cpp`, `UTrailsSubsystem`, `WeatherSubsystem.cpp`, `int64`, `TemplateSubsystem.h`, `Runner_GTWS.cpp`?**
  _High betweenness centrality (0.076) - this node is a cross-community bridge._
- **What connects `TeamParentClass`, `PoiParentClass`, `CheckpointParentClass` to the rest of the system?**
  _1157 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ARaceManager` be split into smaller, more focused modules?**
  _Cohesion score 0.010050251256281407 - nodes in this community are weakly interconnected._
- **Why does `UWorldAmbienceSubsystem` connect `UWorldAmbienceSubsystem` to `FHttpResultCallback`, `WorldAmbienceSubsystem.h`, `WorldAmbienceSubsystem.cpp`, `UWorldAmbienceSubsystem::OnWorldBeginPlay`, `UPROPERTY`, `UWorldAmbienceSubsystem::UpdateState`, `FAutoWeatherInfo`, `UWorldAmbienceSubsystem::Initialize`, `EBenchPhase`, `FWorldAmbienceSettings`, `FBenchShot`, `Piecewise`?**
  _High betweenness centrality (0.071) - this node is a cross-community bridge._
- **Should `UHttpGatewaySubsystem` be split into smaller, more focused modules?**
  _Cohesion score 0.017391304347826087 - nodes in this community are weakly interconnected._
- **Why does `ARunner` connect `ARunner` to `coreminimal`, `IRunnerInterface`, `ARunner_GTWS`, `FTrackSample`, `BroadcastCaptureSubsystem.cpp`, `ICameraControlInterface`, `FRunnerState`, `.GetFootHook`, `.GetFootMesh`, `FRaceSetup`, `ARunner_Generic`, `FRunnerStruct`, `IRunnerStackableInterface`, `FSettings`, `ARunner_Nike`, `Runner.cpp`, `ARunner_UTMB`, `Runner_GTWS.cpp`?**
  _High betweenness centrality (0.063) - this node is a cross-community bridge._
- **Should `.SlateNotify` be split into smaller, more focused modules?**
  _Cohesion score 0.03507078507078507 - nodes in this community are weakly interconnected._