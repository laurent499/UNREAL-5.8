// All Rights Reserved

#include "RaceManager.h"
#include "TemplateSubsystem.h"
#include "CesiumFlyToComponent.h"
#include "CesiumGeoreference.h"
#include "Cesium3DTileset.h"
#include "CesiumOriginShiftComponent.h"
#include "CheckpointSubsystem.h"
#include "PathSubsystem.h"
#include "PoiSubsystem.h"
#include "HttpGatewaySubsystem.h"
#include "LoadingStatusSubsystem.h"
// Cible jeu : windows.h (inclus plus haut via Cesium) definit la macro GetEnvironmentVariable,
// qui casse FPlatformMisc::GetEnvironmentVariable dans FOWLSaveToFileSettings.h (OWL).
#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#endif
#include "OWLViewportCapture.h"
#include "OWLMediaOutput.h"
#include "OWLMediaOutputComponent.h"
#include "AudioDevice.h"
#include "AudioMixerDevice.h"
#include "RunnerSubsystem.h"
#include "RaceSubsystem.h"
#include "ScaleSubsystem.h"
#include "TeamSubsystem.h"
#include "SettingsSubsystem.h"
#include "SlateNotificationsBFL.h"
#include "TeamGroupSubsystem.h"
#include "WeatherSubsystem.h"
#include "Actors/Path.h"
#include "Components/PathSegmentsComponent.h"
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
#include "Camera/PlayerCameraManager.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "UI/SLoadingOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Engine/EngineTypes.h"
#include "Math/UnrealMathUtility.h" // FMath::IsFinite
#include "Curves/CurveFloat.h"

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

/**
 * Point de vue d'arrivee sur une course (MoveToRace et lancement).
 * Hauteur : point le plus haut du trace + 1000 m (au-dessus des cretes, tuiles grossieres comprises),
 * plutot que depart + 5000 m. Camera inclinee a 45 deg, reculee pour viser le depart, tournee vers le
 * gros du trace (barycentre, ou premier point a plus de 500 m pour une boucle).
 * Sortie : lon/lat/hauteur, yaw et pitch en East-South-Up (convention Cesium FlyTo).
 */
static bool ComputeRaceViewpoint(const FRacePath& Path, FVector& OutLonLatHeight, double& OutYaw, double& OutPitch)
{
	if (Path.Points.IsEmpty()) return false;
	static constexpr double MarginM = 1000.0;
	static constexpr double PitchDeg = -45.0;
	static constexpr double MPerDegLat = 110540.0;

	const FRacePathPoint& P0 = Path.Points[0];
	const double MPerDegLon = 111320.0 * FMath::Max(0.01, FMath::Cos(FMath::DegreesToRadians(double(P0.lat))));

	double MaxEle = P0.ele;
	FVector2D Sum(0.0, 0.0); // (est, nord) en metres depuis le depart
	FVector2D FirstFar(0.0, 0.0);
	for (const FRacePathPoint& P : Path.Points)
	{
		MaxEle = FMath::Max(MaxEle, double(P.ele));
		const FVector2D EN((P.lon - P0.lon) * MPerDegLon, (P.lat - P0.lat) * MPerDegLat);
		Sum += EN;
		if (FirstFar.IsNearlyZero() && EN.SizeSquared() > 500.0 * 500.0)
		{
			FirstFar = EN;
		}
	}
	FVector2D Dir = Sum / double(Path.Points.Num());
	if (Dir.SizeSquared() < 200.0 * 200.0) Dir = FirstFar;
	Dir = Dir.IsNearlyZero() ? FVector2D(1.0, 0.0) : Dir.GetSafeNormal();

	const double Height = MaxEle + MarginM;
	// A 45 deg, reculer d'autant que la hauteur au-dessus du depart pour l'avoir au centre de l'image
	const double Back = FMath::Max(0.0, Height - double(P0.ele)) / FMath::Tan(FMath::DegreesToRadians(-PitchDeg));
	OutLonLatHeight = FVector(
		P0.lon - Dir.X * Back / MPerDegLon,
		P0.lat - Dir.Y * Back / MPerDegLat,
		Height);
	OutYaw = FMath::RadiansToDegrees(FMath::Atan2(-Dir.Y, Dir.X)); // ESU : 0 = est, 90 = sud
	OutPitch = PitchDeg;
	return true;
}

/**
 * Teleportation sur un coureur, un checkpoint ou un POI : la camera arrive face a la cible, a distance,
 * un peu au-dessus, et la regarde (avant : a la verticale de la cible, regard a l'horizontale).
 * Tout est calcule en lon/lat/hauteur au moment de l'appel (l'origine du monde peut etre loin de la course).
 * - coureur, checkpoint : camera en avant sur le trace, tournee vers l'arriere (on voit arriver les coureurs) ;
 * - POI hors du trace : camera du cote du trace, regard du trace vers le POI ; POI sur le trace : vue de cote.
 * Sol : altitude du trace autour de la camera, tuiles Cesium si deja chargees, puis garde au sol apres l'arrivee.
 */
namespace FaceView
{
	enum class ETarget : uint8 { Runner, Checkpoint, Poi };

	static TAutoConsoleVariable<float> CVarRunnerM(TEXT("ts.Teleport.RunnerM"), 150.f,
		TEXT("Teleportation sur un coureur : distance au sol entre la camera et le coureur (m)"));
	static TAutoConsoleVariable<float> CVarCheckpointM(TEXT("ts.Teleport.CheckpointM"), 250.f,
		TEXT("Teleportation sur un checkpoint : distance au sol entre la camera et le checkpoint (m)"));
	static TAutoConsoleVariable<float> CVarPoiM(TEXT("ts.Teleport.PoiM"), 400.f,
		TEXT("Teleportation sur un POI : distance au sol entre la camera et le POI (m)"));
	static TAutoConsoleVariable<float> CVarPitchDeg(TEXT("ts.Teleport.PitchDeg"), 15.f,
		TEXT("Teleportation : angle sous l'horizontale sous lequel la camera voit la cible (deg)"));
	static TAutoConsoleVariable<float> CVarClearanceM(TEXT("ts.Teleport.ClearanceM"), 30.f,
		TEXT("Teleportation : hauteur minimale de la camera au-dessus du sol (m)"));

	static constexpr double MPerDegLat = 110540.0;
	static double MPerDegLon(double Lat) { return 111320.0 * FMath::Max(0.01, FMath::Cos(FMath::DegreesToRadians(Lat))); }

	/** (est, nord) en metres de (Lon, Lat) par rapport a (Lon0, Lat0). */
	static FVector2D EN(double Lon, double Lat, double Lon0, double Lat0)
	{
		return FVector2D((Lon - Lon0) * MPerDegLon(Lat0), (Lat - Lat0) * MPerDegLat);
	}

	/** Direction de course (est, nord) unitaire au point I, mesuree entre les points a ~30 m de part et d'autre. */
	static FVector2D CourseDir(const FRacePath& Path, int32 I)
	{
		const FRacePathPoint& C = Path.Points[I];
		int32 A = I, B = I;
		while (A > 0 && EN(Path.Points[A].lon, Path.Points[A].lat, C.lon, C.lat).Size() < 30.0) --A;
		while (B < Path.Points.Num() - 1 && EN(Path.Points[B].lon, Path.Points[B].lat, C.lon, C.lat).Size() < 30.0) ++B;
		const FVector2D D = EN(Path.Points[B].lon, Path.Points[B].lat, Path.Points[A].lon, Path.Points[A].lat);
		return D.IsNearlyZero() ? FVector2D::ZeroVector : D.GetSafeNormal();
	}

	/** Hauteur du sol sous (Lon, Lat) d'apres les tuiles Cesium deja chargees ; false si rien n'est touche. */
	static bool TraceGround(UWorld* World, ACesiumGeoreference* Geo, double Lon, double Lat, double TopH, double BottomH, double& OutH)
	{
		const FVector Top = Geo->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Lon, Lat, TopH));
		const FVector Bottom = Geo->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Lon, Lat, BottomH));
		TArray<FHitResult> Hits;
		World->LineTraceMultiByChannel(Hits, Top, Bottom, ECC_Visibility);
		for (const FHitResult& Hit : Hits)
		{
			if (Hit.GetActor() && Hit.GetActor()->IsA<ACesium3DTileset>())
			{
				OutH = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(Hit.ImpactPoint).Z;
				return true;
			}
		}
		return false;
	}

	/**
	 * Point de vue face a la cible. Target : lon/lat/hauteur de la cible.
	 * Sortie : lon/lat/hauteur de la camera, yaw et pitch en East-South-Up (convention Cesium FlyTo).
	 */
	static void Compute(UWorld* World, ACesiumGeoreference* Geo, const FRacePath* Path, const FVector& Target, ETarget Kind,
		FVector& OutLLH, double& OutYaw, double& OutPitch)
	{
		const double Dist = FMath::Max(10.0, double(Kind == ETarget::Runner ? CVarRunnerM.GetValueOnGameThread()
			: Kind == ETarget::Checkpoint ? CVarCheckpointM.GetValueOnGameThread() : CVarPoiM.GetValueOnGameThread()));
		const double Tan = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(double(CVarPitchDeg.GetValueOnGameThread()), 0.0, 80.0)));
		const double Clearance = FMath::Max(0.0, double(CVarClearanceM.GetValueOnGameThread()));

		// Direction cible -> camera (est, nord) ; repli sans trace : camera au sud, regard vers le nord
		FVector2D Dir(0.0, -1.0);
		if (Path && Path->Points.Num() >= 2)
		{
			int32 Nearest = 0;
			double NearestSq = TNumericLimits<double>::Max();
			for (int32 i = 0; i < Path->Points.Num(); ++i)
			{
				const double Sq = EN(Path->Points[i].lon, Path->Points[i].lat, Target.X, Target.Y).SizeSquared();
				if (Sq < NearestSq) { NearestSq = Sq; Nearest = i; }
			}
			const FVector2D Course = CourseDir(*Path, Nearest);
			const FVector2D ToTrail = EN(Path->Points[Nearest].lon, Path->Points[Nearest].lat, Target.X, Target.Y);
			if (Kind == ETarget::Poi && ToTrail.Size() > 50.0)
			{
				Dir = ToTrail.GetSafeNormal(); // POI hors du trace : on le regarde depuis le trace
			}
			else if (Kind == ETarget::Poi)
			{
				if (!Course.IsNearlyZero()) Dir = FVector2D(Course.Y, -Course.X); // POI sur le trace : normale (a droite)
			}
			else if (!Course.IsNearlyZero())
			{
				Dir = Course; // coureur, checkpoint : en avant sur le trace, tourne vers l'arriere
			}
		}
		const FVector2D CamEN = Dir * Dist;
		const double CamLon = Target.X + CamEN.X / MPerDegLon(Target.Y);
		const double CamLat = Target.Y + CamEN.Y / MPerDegLat;

		// Hauteur : l'angle voulu au-dessus de la cible, et jamais sous le sol connu autour de la camera
		double CamH = Target.Z + Dist * Tan;
		if (Path)
		{
			for (const FRacePathPoint& P : Path->Points)
			{
				if (EN(P.lon, P.lat, CamLon, CamLat).SizeSquared() < 100.0 * 100.0) CamH = FMath::Max(CamH, double(P.ele) + Clearance);
			}
		}
		double GroundH = 0.0;
		if (World && TraceGround(World, Geo, CamLon, CamLat, CamH + 5000.0, CamH - 5000.0, GroundH))
		{
			CamH = FMath::Max(CamH, GroundH + Clearance);
		}

		OutLLH = FVector(CamLon, CamLat, CamH);
		OutYaw = FMath::RadiansToDegrees(FMath::Atan2(CamEN.Y, -CamEN.X)); // regard camera -> cible ; ESU : 0 = est, 90 = sud
		OutPitch = -FMath::RadiansToDegrees(FMath::Atan2(CamH - Target.Z, Dist));
	}

	/** Teleportation en cours ; une nouvelle (ou un MoveToRace) arrete la garde au sol de la precedente. */
	static uint32 GuardSerial = 0;

	/**
	 * Garde au sol : une fois le vol termine, pendant 4 s (le temps que les tuiles fines arrivent), la camera
	 * est remontee si le relief Cesium la depasse, puis tournee de nouveau vers la cible.
	 */
	static void StartGroundGuard(UWorld* World, UCesiumFlyToComponent* Fly, ACesiumGeoreference* Geo, AActor* Target)
	{
		if (!World || !Fly || !Geo) return;
		const uint32 Serial = ++GuardSerial;
		TSharedRef<FTimerHandle> Handle = MakeShared<FTimerHandle>();
		TSharedRef<int32> Ticks = MakeShared<int32>(0);
		TWeakObjectPtr<UWorld> WeakWorld = World;
		TWeakObjectPtr<UCesiumFlyToComponent> WeakFly = Fly;
		TWeakObjectPtr<ACesiumGeoreference> WeakGeo = Geo;
		TWeakObjectPtr<AActor> WeakTarget = Target;
		World->GetTimerManager().SetTimer(*Handle, FTimerDelegate::CreateLambda([=]()
		{
			UWorld* W = WeakWorld.Get();
			UCesiumFlyToComponent* F = WeakFly.Get();
			ACesiumGeoreference* G = WeakGeo.Get();
			APawn* Pawn = F ? Cast<APawn>(F->GetOwner()) : nullptr;
			++(*Ticks);
			// Arret 4 s apres la fin du vol (compteur pose a 104 a l arrivee, 16 x 0,25 s jusqu a 120)
			if (!W || !G || !Pawn || Serial != GuardSerial || *Ticks > 120)
			{
				if (W) W->GetTimerManager().ClearTimer(*Handle);
				return;
			}
			if (F->IsFlightInProgress())
			{
				*Ticks = FMath::Min(*Ticks, 104);
				return;
			}
			*Ticks = FMath::Max(*Ticks, 104);

			const double Clearance = FMath::Max(0.0, double(CVarClearanceM.GetValueOnGameThread()));
			const FVector Cam = G->TransformUnrealPositionToLongitudeLatitudeHeight(Pawn->GetActorLocation());
			double GroundH = 0.0;
			if (!TraceGround(W, G, Cam.X, Cam.Y, Cam.Z + 3000.0, Cam.Z - Clearance, GroundH) || GroundH + Clearance <= Cam.Z + 1.0) return;

			const FVector NewLocation = G->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Cam.X, Cam.Y, GroundH + Clearance));
			Pawn->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			if (AActor* T = WeakTarget.Get())
			{
				if (AController* Controller = Pawn->GetController())
				{
					Controller->SetControlRotation((T->GetActorLocation() - NewLocation).Rotation());
				}
			}
			UE_LOG(LogTemp, Log, TEXT("[Teleport] Camera remontee au-dessus du relief (%.0f m)"), GroundH + Clearance - Cam.Z);
		}), 0.25f, true);
	}
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

#if WITH_METADATA
		// Match par DisplayName (meta) : metadonnees absentes du jeu package
		const FString Display = P->GetMetaData(TEXT("DisplayName"));
		if (!Display.IsEmpty() && SanitizeKey(Display) == WantedKey)
		{
			Best = P;
			break;
		}
#endif
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
	HttpGatewaySubsystem->OnSetPathSegments.AddUObject(this, &ARaceManager::RtSetPathSegments);
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
	HttpGatewaySubsystem->OnCheckpointMove.AddUObject(this, &ARaceManager::RtMoveCheckpoint);
	
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

	// Audio coupe sur toutes les destinations OWL des le BeginPlay, quel que soit le moyen de
	// lancer le flux (case du RaceManager, -StartSRT, bouton OWL) : l'init du resampler audio
	// 7.1 -> stereo precedait chaque crash. -SRTAudio / bSRTEncodeAudio le reactivent, -NoSRTAudio le coupe quoi qu'il arrive.
	if (AOWLMediaOutput* OwlOutput = Cast<AOWLMediaOutput>(UGameplayStatics::GetActorOfClass(GetWorld(), AOWLMediaOutput::StaticClass())))
	{
		if (OwlOutput->MediaOutputComponent)
		{
			const bool bAudio = (bSRTEncodeAudio || FParse::Param(FCommandLine::Get(), TEXT("SRTAudio"))) && !FParse::Param(FCommandLine::Get(), TEXT("NoSRTAudio"));
			for (FOWLMediaOutputDestination& Destination : OwlOutput->MediaOutputComponent->Settings.Destinations)
			{
				Destination.bEncodeAudio = bAudio;
			}
		}
	}

	// Demarrage differe : lancer l'encodeur OWL des la premiere image fait planter le RHI D3D12
	// (CopyCommandList->Reset E_INVALIDARG, UE 5.8). On laisse le rendu et la capture s'installer.
	// -StartSRT (passe par RemoteControl/Lancer_Standalone.bat) force le flux quelle que soit la
	// case de l'acteur dans la map ; -SRTURL=srt://hote:port remplace la destination.
	const bool bForceSRT = FParse::Param(FCommandLine::Get(), TEXT("StartSRT"));
	FString SRTUrlOverride;
	if (FParse::Value(FCommandLine::Get(), TEXT("SRTURL="), SRTUrlOverride) && !SRTUrlOverride.IsEmpty())
	{
		SRTStreamURL = SRTUrlOverride;
	}
	if (!bStartSRTOnBeginPlay && !bForceSRT)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OWL] Demarrage auto du flux SRT desactive (bStartSRTOnBeginPlay = false sur %s). Ajouter -StartSRT a la ligne de commande pour le forcer."), *GetName());
	}
	if (bStartSRTOnBeginPlay || bForceSRT)
	{
		FTimerHandle SRTStartHandle;
		GetWorldTimerManager().SetTimer(SRTStartHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			StartSRTOutput();
		}), 3.f, false);
	}
	
	// UDS Timer
	if (UDSClass)
	{
		UDSActor = UGameplayStatics::GetActorOfClass(GetWorld(), UDSClass);
		if (UDSActor)
		{
			// AlignSkyToGlobe doit passer apres UDS, qui reecrit ses composants a chaque tick
			AddTickPrerequisiteActor(UDSActor);
		}
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
		}
	}

	AlignSkyToGlobe();

	// Deplacement fluide des coureurs entre deux snapshots
	for (auto It = InterpolatingRunners.CreateIterator(); It; ++It)
	{
		ARunner* Runner = It->Get();
		if (!Runner || !Runner->AdvanceTrackInterp(DeltaTime))
		{
			It.RemoveCurrent();
		}
	}
}

void ARaceManager::TrackRunnerInterp(AActor* RunnerActor)
{
	ARunner* Runner = Cast<ARunner>(RunnerActor);
	if (Runner && Runner->IsTrackInterpActive())
	{
		InterpolatingRunners.Add(Runner);
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
	FollowOriginWithCamera();
}

namespace GlobeOrigin
{
	static TAutoConsoleVariable<int32> CVarFollow(TEXT("ts.Globe.FollowOrigin"), 1,
		TEXT("Loin de la course, l'origine Cesium suit la camera (0 = origine fixe sur le depart, comme avant)"));
	static TAutoConsoleVariable<float> CVarLeaveKm(TEXT("ts.Globe.LeaveRaceKm"), 150.f,
		TEXT("Distance au depart de la course au-dela de laquelle l'origine commence a suivre la camera"));
	static TAutoConsoleVariable<float> CVarStepKm(TEXT("ts.Globe.StepKm"), 50.f,
		TEXT("Loin de la course : l'origine est recalee sous la camera des qu'elle s'en eloigne de cette distance"));

	/** Distance au sol (km) entre deux points lon/lat en degres. */
	static double GroundKm(double LonA, double LatA, double LonB, double LatB)
	{
		const double P1 = FMath::DegreesToRadians(LatA), P2 = FMath::DegreesToRadians(LatB);
		const double DP = P2 - P1, DL = FMath::DegreesToRadians(LonB - LonA);
		const double H = FMath::Square(FMath::Sin(DP * 0.5)) + FMath::Cos(P1) * FMath::Cos(P2) * FMath::Square(FMath::Sin(DL * 0.5));
		return 6371.0 * 2.0 * FMath::Asin(FMath::Min(1.0, FMath::Sqrt(H)));
	}
}

/**
 * @brief Loin de la course, garde l'origine Cesium sous la camera (simulateur : la verticale du jeu suit le globe).
 * Pres de la course rien ne change : l'origine reste sur le depart, repere du trace et de tout le live.
 * Au-dela de ts.Globe.LeaveRaceKm du depart, l'origine est reposee a la verticale de la camera a chaque
 * ts.Globe.StepKm parcouru (meme hauteur d'origine, pour que le brouillard ne saute pas) ; en revenant a moins
 * de LeaveRaceKm - StepKm du depart, elle retourne exactement sur le depart. Les acteurs suivent par leur
 * ancre globe, comme lors d'un changement de course. Appelee une fois par seconde.
 */
void ARaceManager::FollowOriginWithCamera()
{
	if (!GlobeOrigin::CVarFollow.GetValueOnGameThread() || !bStartupPlacementDone || !RaceSubsystem) return;
	if (FlyComp && FlyComp->IsFlightInProgress()) return; // MoveToRace : OnFlightComplete repose l'origine
	ACesiumGeoreference* Geo = Georeference.Get();
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	const FRacePath* Path = PathsDatas.Find(RaceSubsystem->GetCurrentRaceId());
	if (!Geo || !CameraManager || !Path || Path->Points.IsEmpty()) return;

	const FRacePathPoint& P0 = Path->Points[0];
	const FVector Cam = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(CameraManager->GetCameraLocation());
	if (!FMath::IsFinite(Cam.X) || !FMath::IsFinite(Cam.Y)) return;
	const FVector Origin = Geo->GetOriginLongitudeLatitudeHeight();

	const double LeaveKm = FMath::Max(10.0, double(GlobeOrigin::CVarLeaveKm.GetValueOnGameThread()));
	const double StepKm = FMath::Clamp(double(GlobeOrigin::CVarStepKm.GetValueOnGameThread()), 1.0, LeaveKm * 0.8);
	const double CamToRaceKm = GlobeOrigin::GroundKm(Cam.X, Cam.Y, P0.lon, P0.lat);
	const bool bOriginOnRace = GlobeOrigin::GroundKm(Origin.X, Origin.Y, P0.lon, P0.lat) < 0.01;

	if (bOriginOnRace)
	{
		if (CamToRaceKm <= LeaveKm) return; // mode course : rien ne bouge
	}
	else if (CamToRaceKm < LeaveKm - StepKm)
	{
		Geo->SetOriginLongitudeLatitudeHeight(FVector(P0.lon, P0.lat, P0.ele));
		UE_LOG(LogTemp, Log, TEXT("[Globe] Origine revenue sur le depart de la course (camera a %.0f km)"), CamToRaceKm);
		return;
	}
	else if (GlobeOrigin::GroundKm(Cam.X, Cam.Y, Origin.X, Origin.Y) < StepKm)
	{
		return;
	}
	Geo->SetOriginLongitudeLatitudeHeight(FVector(Cam.X, Cam.Y, P0.ele));
	UE_LOG(LogTemp, Log, TEXT("[Globe] Origine recalee sous la camera (lon %.3f lat %.3f, a %.0f km de la course)"), Cam.X, Cam.Y, CamToRaceKm);
}

/**
 * @brief Cale le ciel UDS sur le globe Cesium sous la camera.
 * L'origine du georef reste au depart de la course (les splines du trace sont en coordonnees Unreal),
 * donc loin du depart la Terre « descend » sous le plan Unreal (~8 km a 300 km) et penche. Sans recalage,
 * le sol de l'atmosphere et le brouillard (plan horizontal) restent a l'altitude du depart : bande noire
 * a l'horizon et relief noye dans le brouillard (constate le 10/11 a 314 km du Mont-Blanc).
 * On pose donc la planete de l'atmosphere tangente au sol reel sous la camera (centre le long de la
 * verticale locale) et le brouillard a l'altitude du depart sous la camera. Pres du depart, rien ne bouge.
 * Appelee a chaque tick apres UDS, mais n'ecrit que si le calage a bouge de plus de 10 m.
 */
void ARaceManager::AlignSkyToGlobe()
{
	if (!UDSActor) return;
	ACesiumGeoreference* Geo = Georeference.Get();
	if (!Geo)
	{
		Geo = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
		Georeference = Geo;
	}
	USkyAtmosphereComponent* Atmosphere = UDSActor->FindComponentByClass<USkyAtmosphereComponent>();
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!Geo || !Atmosphere || !CameraManager) return;

	if (!bSkyGlobeSetup)
	{
		bSkyGlobeSetup = true;
		// UDS deplacerait le sommet de la planete sous la camera a chaque tick : c'est nous qui la placons
		if (FBoolProperty* Keep = FindFProperty<FBoolProperty>(UDSActor->GetClass(), TEXT("keep Planet Top at Camera XY Location")))
		{
			Keep->SetPropertyValue_InContainer(UDSActor, false);
		}
		Atmosphere->TransformMode = ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform;
		Atmosphere->MarkRenderStateDirty();
	}

	const FVector Camera = CameraManager->GetCameraLocation();
	const FVector LLH = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(Camera);
	if (!FMath::IsFinite(LLH.X) || !FMath::IsFinite(LLH.Y)) return;
	const FVector Ground = Geo->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(LLH.X, LLH.Y, 0.0));
	const FVector Up = (Geo->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(LLH.X, LLH.Y, 1000.0)) - Ground).GetSafeNormal();
	if (Up.IsNearlyZero()) return;

	static constexpr double ToleranceCm = 1000.0;

	// Brouillard (plan horizontal) : a l'altitude du depart a la verticale de la camera.
	// La racine UDS est Static : on deplace le composant de brouillard seul (rendu Movable une fois).
	if (UExponentialHeightFogComponent* Fog = UDSActor->FindComponentByClass<UExponentialHeightFogComponent>())
	{
		const double FogZ = Geo->TransformLongitudeLatitudeHeightPositionToUnreal(
			FVector(LLH.X, LLH.Y, Geo->GetOriginHeight())).Z - 150.0;
		const FVector FogLocation = Fog->GetComponentLocation();
		if (FMath::Abs(FogLocation.Z - FogZ) > ToleranceCm)
		{
			if (Fog->Mobility != EComponentMobility::Movable)
			{
				Fog->SetMobility(EComponentMobility::Movable);
			}
			Fog->SetWorldLocation(FVector(FogLocation.X, FogLocation.Y, FogZ));
		}

		// Coupure du brouillard : par temps clair elle evite de voiler le ciel a l'horizon (bande bleue), le ciel
		// etant a profondeur infinie. Elle doit donc depasser le sol le plus lointain visible, sinon un disque de
		// brouillard net apparait vu de tres haut : au moins 300 km, et l'horizon de la camera + 10 %.
		// Par brouillard dense (~2 sur 10), le ciel sous l'horizon resterait net et formerait une bande sombre :
		// on voile alors tout, ciel compris.
		static constexpr float DenseFogDensity = 0.008f;
		static constexpr double ClearSkyCutoffCm = 30000000.0;
		const double CameraAltCm = FMath::Max(0.0, LLH.Z * 100.0);
		// Horizon du sol + horizon du sommet des nuages : au-dela de l'horizon du sol, des nuages restent
		// visibles ; hors coupure ils ne seraient plus voiles et formeraient un lisere net (Cape Town 13/11).
		double CloudTopCm = 0.0;
		if (const UVolumetricCloudComponent* CloudLayer = UDSActor->FindComponentByClass<UVolumetricCloudComponent>())
		{
			CloudTopCm = FMath::Max(0.0, double(CloudLayer->LayerBottomAltitude + CloudLayer->LayerHeight)) * 100000.0;
		}
		const double RadiusCm = double(Atmosphere->BottomRadius) * 100000.0;
		const double HorizonCm = FMath::Sqrt(2.0 * RadiusCm * CameraAltCm) + FMath::Sqrt(2.0 * RadiusCm * CloudTopCm);
		const float WantedCutoff = Fog->FogDensity > DenseFogDensity ? 0.f
			: float(FMath::Max(ClearSkyCutoffCm, HorizonCm * 1.1));
		if (WantedCutoff == 0.f ? Fog->FogCutoffDistance != 0.f
			: FMath::Abs(Fog->FogCutoffDistance - WantedCutoff) > WantedCutoff * 0.05f)
		{
			Fog->SetFogCutoffDistance(WantedCutoff);
		}
	}

	// Vue de l'espace : le brouillard UDS (pense pour le sol) voile tout le globe en blanc et les nuages 2D
	// de la sphere de ciel deviennent des trainees. Ils s'effacent entre 20 et 60 km d'altitude ; au-dela,
	// la brume vue d'en haut vient de l'atmosphere seule. Sous 20 km, rien ne change.
	{
		static constexpr double FadeStartM = 20000.0;
		static constexpr double FadeEndM = 60000.0;
		const float GroundFade = 1.f - float(FMath::SmoothStep(FadeStartM, FadeEndM, LLH.Z));
		static bool bFaded = false; // un seul RaceManager par monde
		if (GroundFade < 1.f || bFaded)
		{
			bFaded = GroundFade < 1.f;
			if (UExponentialHeightFogComponent* Fog = UDSActor->FindComponentByClass<UExponentialHeightFogComponent>())
			{
				if (GroundFade >= 1.f ? Fog->FogMaxOpacity != 1.f : !FMath::IsNearlyEqual(Fog->FogMaxOpacity, GroundFade, 0.005f))
				{
					Fog->SetFogMaxOpacity(GroundFade);
				}
			}
			// Voiles 2D du ciel UDS : valeurs d'origine retenues au premier fondu, remises en redescendant
			static double WispsClear = -1.0, WispsCloudy = -1.0;
			FDoubleProperty* Clear = FindFProperty<FDoubleProperty>(UDSActor->GetClass(), TEXT("cloud Wisps Opacity (Clear)"));
			FDoubleProperty* Cloudy = FindFProperty<FDoubleProperty>(UDSActor->GetClass(), TEXT("cloud Wisps Opacity (Cloudy)"));
			if (Clear && Cloudy)
			{
				if (WispsClear < 0.0)
				{
					WispsClear = Clear->GetPropertyValue_InContainer(UDSActor);
					WispsCloudy = Cloudy->GetPropertyValue_InContainer(UDSActor);
				}
				Clear->SetPropertyValue_InContainer(UDSActor, WispsClear * GroundFade);
				Cloudy->SetPropertyValue_InContainer(UDSActor, WispsCloudy * GroundFade);
			}
		}
	}

	// Planete de l'atmosphere (et des nuages) : tangente au niveau de la mer sous la camera
	const FVector Center = Ground - Up * (double(Atmosphere->BottomRadius) * 100000.0);
	if (Atmosphere->TransformMode != ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform)
	{
		Atmosphere->TransformMode = ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform;
		Atmosphere->MarkRenderStateDirty();
	}
	if (FVector::DistSquared(Atmosphere->GetComponentLocation(), Center) > ToleranceCm * ToleranceCm)
	{
		Atmosphere->SetWorldLocation(Center);
	}

	// Nuages jusqu'a l'horizon : en montant, l'horizon de la couche s'eloigne (au-dela de ~23 km d'altitude
	// il depasse la portee UDS) et un anneau sans nuages apparait au bord du globe. On allonge la portee
	// de depart des rayons a la distance de l'horizon de la couche ; le nombre d'echantillons ne change pas.
	if (UVolumetricCloudComponent* Cloud = UDSActor->FindComponentByClass<UVolumetricCloudComponent>())
	{
		const double RadiusKm = Atmosphere->BottomRadius;
		const double CameraAltKm = FMath::Max(0.0, (Camera - Center).Size() / 100000.0 - RadiusKm);
		const double CloudTopKm = FMath::Max(0.0, double(Cloud->LayerBottomAltitude + Cloud->LayerHeight));
		const double HorizonKm = FMath::Sqrt(2.0 * RadiusKm * CameraAltKm) + FMath::Sqrt(2.0 * RadiusKm * CloudTopKm);
		// Suit l'horizon dans les deux sens (UDS ne remet pas sa valeur en redescendant)
		const float Wanted = float(FMath::Clamp(HorizonKm * 1.05, 400.0, 5000.0));
		if (FMath::Abs(Cloud->TracingStartMaxDistance - Wanted) > Wanted * 0.05f)
		{
			Cloud->SetTracingStartMaxDistance(Wanted);
		}
		// UDS borne la distance de trace a ~60 km depuis la camera : la couche s'arretait net quelques degres
		// au-dessus de l'horizon (bande claire sous un plafond couvert, Cape Town 10/11). Comptee depuis
		// l'entree dans la couche, elle va jusqu'a l'horizon pour le meme nombre d'echantillons.
		if (Cloud->TracingMaxDistanceMode != EVolumetricCloudTracingMaxDistanceMode::DistanceFromCloudLayerEntryPoint)
		{
			Cloud->TracingMaxDistanceMode = EVolumetricCloudTracingMaxDistanceMode::DistanceFromCloudLayerEntryPoint;
			Cloud->MarkRenderStateDirty();
		}
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
		// Le libelle de la tache Setup sert de titre a la carte de la course dans l'ecran d'initialisation
		LoadingSubsystem->RegisterTask(IdSetup, FText::FromString(RaceEntry.name.IsEmpty()
			? FString::Printf(TEXT("Course %lld"), RaceId)
			: FString::Printf(TEXT("%s"), *RaceEntry.name)));

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
		// Course sans setup valide : pas de trace a attendre pour le placement de depart
		StartupPendingRaces.Remove(RaceID);
		TryStartupPlacement();
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
			ApplyPathSegments(RaceID);
			LoadingSubsystem->Complete(IdPath, FText::FromString(FString::Printf(TEXT("Path for Race %lld drawn"), RaceID)));
			
		} else
		{
			LoadingSubsystem->Fail(IdPath, FText::FromString(FString::Printf(TEXT("Path for Race %lld failed to spawn"), RaceID)));
		}
	} else
	{
		LoadingSubsystem->Fail(IdPath, FText::FromString(FString::Printf(TEXT("No Point in Path for Race %lld failed"), RaceID)));
	}

	// DrawPath vient de caler l'origine du georef sur le depart de CETTE course : une fois la camera
	// placee, on remet l'origine sur la course affichee, sinon la vue partirait sur la derniere course chargee.
	RestoreOriginOnCurrentRace(RaceID);
	StartupPendingRaces.Remove(RaceID);
	TryStartupPlacement();
}

/**
 * @brief Place la camera sur la premiere course de la regie ayant un trace.
 * Attend que les courses precedentes dans la liste soient tranchees (trace recu ou echec), sauf bForce (filet 30 s).
 */
void ARaceManager::TryStartupPlacement(bool bForce)
{
	if (bStartupPlacementDone || StartupRaceOrder.IsEmpty()) return;

	for (const int64 RaceID : StartupRaceOrder)
	{
		const FRacePath* Path = PathsDatas.Find(RaceID);
		if (Path && Path->Points.Num() > 0 && RacePaths.Contains(RaceID))
		{
			bStartupPlacementDone = true;
			GetWorldTimerManager().ClearTimer(StartupPlacementHandle);
			PlaceOnRaceAtStartup(RaceID);
			return;
		}
		if (StartupPendingRaces.Contains(RaceID) && !bForce)
		{
			return; // une course mieux placee dans la liste peut encore arriver
		}
	}

	if (bForce || StartupPendingRaces.IsEmpty())
	{
		bStartupPlacementDone = true;
		GetWorldTimerManager().ClearTimer(StartupPlacementHandle);
		UE_LOG(LogTemp, Warning, TEXT("[Startup] Aucune course avec trace : camera laissee en place"));
	}
}

/**
 * @brief Teleporte directement (sans vol) au-dessus du depart, comme MoveToRace mais instantane.
 */
void ARaceManager::PlaceOnRaceAtStartup(int64 RaceID)
{
	ACesiumGeoreference* Geo = Georeference.Get();
	if (!Geo)
	{
		Geo = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
		Georeference = Geo;
	}
	const FRacePath* Path = PathsDatas.Find(RaceID);
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Geo || !Path || Path->Points.IsEmpty() || !Pawn || !RaceSubsystem)
	{
		MoveToPath_Internal(RaceID); // repli : le vol habituel
		return;
	}

	const int64 PreviousRaceID = RaceSubsystem->GetCurrentRaceId();
	if (PreviousRaceID != -1 && PreviousRaceID != RaceID)
	{
		ToggleRace_Internal(PreviousRaceID, false);
	}
	RaceSubsystem->SetCurrentRaceId(RaceID);

	const FRacePathPoint& P0 = Path->Points[0];
	FVector Viewpoint;
	double ViewYaw = 0.0, ViewPitch = 0.0;
	ComputeRaceViewpoint(*Path, Viewpoint, ViewYaw, ViewPitch);
	if (ShiftComp) ShiftComp->SetActive(false);
	// Origine sur le depart d'abord (repere des splines), puis le pawn par-dessus.
	// Pres de l'origine, East-South-Up et Unreal coincident : le yaw/pitch ESU sert tel quel.
	Geo->SetOriginLongitudeLatitudeHeight(FVector(P0.lon, P0.lat, P0.ele));
	const FVector Location = Geo->TransformLongitudeLatitudeHeightPositionToUnreal(Viewpoint);
	const FRotator ViewRotation(ViewPitch, ViewYaw, 0.0);
	Pawn->SetActorLocationAndRotation(Location, FRotator(0.0, ViewYaw, 0.0), false, nullptr, ETeleportType::TeleportPhysics);
	if (AController* Controller = Pawn->GetController())
	{
		Controller->SetControlRotation(ViewRotation);
	}

	UE_LOG(LogTemp, Log, TEXT("[Startup] Camera placee sur la course %lld"), RaceID);
	ToggleRace_Internal(RaceID, true);
}

/**
 * @brief Apres le dessin d'un autre trace (qui deplace l'origine), remet l'origine sur la course courante.
 */
void ARaceManager::RestoreOriginOnCurrentRace(int64 JustDrawnRaceID)
{
	if (!bStartupPlacementDone || !RaceSubsystem) return;
	const int64 CurrentRaceID = RaceSubsystem->GetCurrentRaceId();
	if (CurrentRaceID == JustDrawnRaceID) return;
	const FRacePath* Path = PathsDatas.Find(CurrentRaceID);
	ACesiumGeoreference* Geo = Georeference.Get();
	if (!Path || Path->Points.IsEmpty() || !Geo) return;
	const FRacePathPoint& P0 = Path->Points[0];
	Geo->SetOriginLongitudeLatitudeHeight(FVector(P0.lon, P0.lat, P0.ele));
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
	const FString Base = UTemplateSubsystem::GetForRace(this, RaceSetup).Base;
	if (Base == TEXT("UTMB"))
	{
		return APoi_UTMB::StaticClass();
	}
	if (Base == TEXT("GTWS"))
	{
		return APoi_GTWS::StaticClass();
	}
	if (Base == TEXT("Nike"))
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

	// Hauteur du cartouche au-dessus du coureur, réglée dans le template
	Runner.VDelta = UTemplateSubsystem::GetRunnerHeight(this, RaceSetup);
}

TSubclassOf<AActor> ARaceManager::GetRunnerClassForRace(const FRaceSetup& RaceSetup) const
{
	const FString Base = UTemplateSubsystem::GetForRace(this, RaceSetup).Base;
	if (Base == TEXT("UTMB"))
	{
		return ARunner_UTMB::StaticClass();
	}
	if (Base == TEXT("GTWS"))
	{
		return ARunner_GTWS::StaticClass();
	}
	if (Base == TEXT("Nike"))
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
		// Course sans coureur pour l'instant : pas une erreur, les teams arriveront
		// par UpdateRunnersFromSnapshot (spawn a chaud)
		LoadingSubsystem->Complete(IdRunners, FText::FromString(FString::Printf(TEXT("No runners yet for Race %lld"), RaceID)));
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

	// Le nouveau snapshot remplace les mises a jour pas encore traitees du precedent
	PendingRunnerUpdates.Reset();
	PendingRunnerRaceID = RaceID;

	for (FRunnerStruct& Runner : RunnersDatas.Runners)
	{
		TObjectPtr<AActor> CurrentRunner = RunnerSubsystem->GetRunnerActorByTeam(RaceID, Runner.canalId);

		// Coureur deja present : mise a jour etalee sur les images suivantes
		if (CurrentRunner)
		{
			PendingRunnerUpdates.Add(Runner);
			continue;
		}

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
				TrackRunnerInterp(CurrentRunner);
			}
		}
	}

	if (!PendingRunnerUpdates.IsEmpty() && !PendingRunnerHandle.IsValid())
	{
		PendingRunnerHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ARaceManager::ProcessPendingRunnerUpdates);
	}
}

/**
 * @brief Applique quelques mises a jour de coureurs par image, puis se replanifie s'il en reste
 */
void ARaceManager::ProcessPendingRunnerUpdates()
{
	PendingRunnerHandle.Invalidate();
	if (!RunnerSubsystem || !RaceSubsystem) return;

	TObjectPtr<APath>* PathPtr = RacePaths.Find(PendingRunnerRaceID);
	if (!PathPtr || !*PathPtr)
	{
		PendingRunnerUpdates.Reset();
		return;
	}
	const FRaceSetup& RaceSetup = RaceSubsystem->GetRaceSetupById(PendingRunnerRaceID);

	constexpr int32 RunnersPerFrame = 2;
	const int32 Count = FMath::Min(RunnersPerFrame, PendingRunnerUpdates.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FRunnerStruct& Runner = PendingRunnerUpdates[i];
		AActor* CurrentRunner = RunnerSubsystem->GetRunnerActorByTeam(PendingRunnerRaceID, Runner.canalId);
		if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(CurrentRunner))
		{
			RunnerInterface->AssignRunnerToTeam(Runner.canalId);
			RunnerInterface->UpdateRunner(Runner, RaceSetup);
			RunnerInterface->UpdateRunnerLocation(Runner, *PathPtr);
			TrackRunnerInterp(CurrentRunner);
		}
	}
	PendingRunnerUpdates.RemoveAt(0, Count, EAllowShrinking::No);

	if (!PendingRunnerUpdates.IsEmpty())
	{
		PendingRunnerHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ARaceManager::ProcessPendingRunnerUpdates);
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

	// La camera ira sur la premiere course de la liste ayant un trace, des que les precedentes sont
	// tranchees. Filet : au bout de 30 s on se place avec ce qui est arrive (requete perdue sans reponse).
	if (!bStartupPlacementDone && StartupRaceOrder.IsEmpty())
	{
		for (const FRaceEntry& RaceEntry : AllRaces.RacesEntries)
		{
			StartupRaceOrder.Add(RaceEntry.raceId);
			StartupPendingRaces.Add(RaceEntry.raceId);
		}
		GetWorldTimerManager().SetTimer(StartupPlacementHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]() { TryStartupPlacement(true); }), 30.f, false);
	}

	PerformHttpRequestForRaces();
}

void ARaceManager::OnFlightComplete()
{	
	if (!Georeference) return;
	if (!RaceSubsystem) return;
	const int64 CurrentRaceID = RaceSubsystem->GetCurrentRaceId();
	const FRacePath* Path = PathsDatas.Find(CurrentRaceID);
	if (!Path || Path->Points.IsEmpty()) return;
	const FRacePathPoint& P0 = Path->Points[0];
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
		// Les composants du pawn peuvent manquer (appel hors BeginPlay, pawn recree) : on les resout a nouveau
		if (!FlyComp || !ShiftComp)
		{
			if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
			{
				if (!FlyComp)
				{
					FlyComp = Pawn->FindComponentByClass<UCesiumFlyToComponent>();
					if (FlyComp) FlyComp->OnFlightComplete.AddUniqueDynamic(this, &ARaceManager::OnFlightComplete);
				}
				if (!ShiftComp) ShiftComp = Pawn->FindComponentByClass<UCesiumOriginShiftComponent>();
			}
		}
		if (!FlyComp || !ShiftComp)
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Teleport cancelled : pawn components not found (Race %lld)"), RaceID)), EMessageType::Error);
			return;
		}
		ShiftComp->SetActive(false);
		FRacePath NewPath = PathsDatas.FindRef(RaceID);
		FVector Viewpoint;
		double ViewYaw = 0.0, ViewPitch = 0.0;
		if (!ComputeRaceViewpoint(NewPath, Viewpoint, ViewYaw, ViewPitch))
		{
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Teleport cancelled : empty path (Race %lld)"), RaceID)), EMessageType::Error);
			return;
		}

		// Montee en cours de vol plafonnee : la courbe Cesium par defaut grimpe tres haut entre deux courses
		if (!FlyHeightCurve)
		{
			FlyHeightCurve = NewObject<UCurveFloat>(this);
			// distance du vol (m) -> surelevation max a mi-parcours (m)
			FlyHeightCurve->FloatCurve.AddKey(0.f, 0.f);
			FlyHeightCurve->FloatCurve.AddKey(10000.f, 500.f);
			FlyHeightCurve->FloatCurve.AddKey(100000.f, 8000.f);
			FlyHeightCurve->FloatCurve.AddKey(1000000.f, 40000.f);
		}
		FlyComp->MaximumHeightByDistanceCurve = FlyHeightCurve;

		++FaceView::GuardSerial; // la garde au sol d'une teleportation precedente ne doit pas retourner la camera
		FlyComp->FlyToLocationLongitudeLatitudeHeight(Viewpoint, ViewYaw, ViewPitch, false);
				
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
/**
 * @brief Segments du trace : recolore des portions du trace et affiche leur nom
 * @param RaceID 
 * @param Segments liste complete des segments de la course (remplace la precedente)
 */
void ARaceManager::RtSetPathSegments(int64 RaceID, const TArray<FPathSegment>& Segments)
{
	PathSegmentsByRace.Add(RaceID, Segments);
	ApplyPathSegments(RaceID);
}
void ARaceManager::ApplyPathSegments(int64 RaceID)
{
	const TArray<FPathSegment>* Segments = PathSegmentsByRace.Find(RaceID);
	const TObjectPtr<APath>* PathRef = RacePaths.Find(RaceID);
	if (!Segments || !PathRef || !IsValid(PathRef->Get())) return; // appliques a la construction du trace
	APath* Path = PathRef->Get();
	UPathSegmentsComponent* SegmentsComp = Path->FindComponentByClass<UPathSegmentsComponent>();
	if (!SegmentsComp)
	{
		SegmentsComp = NewObject<UPathSegmentsComponent>(Path, TEXT("PathSegments"));
		Path->AddInstanceComponent(SegmentsComp);
		SegmentsComp->RegisterComponent();
	}
	SegmentsComp->SetSegments(*Segments);
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
 * @brief Vol face a une cible (coureur 0, checkpoint 1, POI 2), voir FaceView.
 * @return false si le pawn, son FlyTo ou le georef manquent (l'appelant garde l'ancien vol)
 */
bool ARaceManager::FlyFacing(AActor* Target, int64 RaceID, uint8 Kind) const
{
	UCesiumFlyToComponent* Fly = FlyComp;
	if (!Fly)
	{
		APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
		Fly = Pawn ? Pawn->FindComponentByClass<UCesiumFlyToComponent>() : nullptr;
	}
	ACesiumGeoreference* Geo = Georeference.Get();
	if (!Geo) Geo = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	if (!Target || !Fly || !Geo) return false;

	// Position de la cible lue maintenant (l'origine du monde a pu bouger depuis son placement)
	const FVector TargetLLH = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(Target->GetActorLocation());
	if (!FMath::IsFinite(TargetLLH.X) || !FMath::IsFinite(TargetLLH.Y) || !FMath::IsFinite(TargetLLH.Z)) return false;

	FVector CamLLH;
	double Yaw = 0.0, Pitch = 0.0;
	FaceView::Compute(GetWorld(), Geo, PathsDatas.Find(RaceID), TargetLLH, static_cast<FaceView::ETarget>(Kind), CamLLH, Yaw, Pitch);
	Fly->FlyToLocationLongitudeLatitudeHeight(CamLLH, Yaw, Pitch, false);
	FaceView::StartGroundGuard(GetWorld(), Fly, Geo, Target);
	UE_LOG(LogTemp, Log, TEXT("[Teleport] Camera face a %s : lon %.5f lat %.5f h %.0f m, yaw %.0f pitch %.0f"),
		*Target->GetName(), CamLLH.X, CamLLH.Y, CamLLH.Z, Yaw, Pitch);
	return true;
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
	AActor* RunnerActor = TeamSubsystem->GetRunnerActorByRaceTeam(RaceID, TeamID);
	if (!RunnerActor || !FlyFacing(RunnerActor, RaceID, 0))
	{
		TeamSubsystem->Teleport(TeamID, RaceID); // messages d'erreur (team ou coureur introuvable) et ancien vol
		return;
	}
	const FRunnerStruct* Runner = TeamSubsystem->GetRunnerByRaceTeam(RaceID, TeamID);
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Teleport To Team %lld - %s"),
		TeamID, Runner ? *Runner->nom : TEXT(""))), EMessageType::Success);
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
	AActor* CheckpointActor = CheckpointSubsystem->GetCheckpointActor(RaceID, CheckpointID);
	if (!CheckpointActor || !FlyFacing(CheckpointActor, RaceID, 1))
	{
		CheckpointSubsystem->Teleport(CheckpointID, RaceID); // messages d'erreur et ancien vol
	}
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
 * @brief Move Checkpoint along the path
 * @param Move nouveau point du trace et valeurs recalculees par le serveur
 * @note Course pas chargee : rien a faire, le checkpoint sera a sa nouvelle place au prochain chargement
 */
void ARaceManager::RtMoveCheckpoint(const FCheckpointMove& Move)
{
	const TObjectPtr<APath>* PathRef = RacePaths.Find(Move.RaceID);
	if (!PathRef || !IsValid(PathRef->Get()))
	{
		return;
	}
	if (!PathRef->Get()->MoveCheckpoint(Move.CheckpointID, Move.PathIndex, Move.Distance, Move.Elevation, Move.Altitude, Move.Lat, Move.Lon))
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[MoveCheckpoint] Deplacement impossible (RaceID=%lld CheckpointID=%lld Point=%d)"), Move.RaceID, Move.CheckpointID, Move.PathIndex)), EMessageType::Error);
	}
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
	AActor* PoiActor = PoiSubsystem->GetPoiActor(PoiID, RaceID);
	if (!PoiActor || !FlyFacing(PoiActor, RaceID, 2))
	{
		PoiSubsystem->Teleport(PoiID, RaceID); // messages d'erreur et ancien vol
	}
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
		return; // reglage sauvegarde, applique au prochain chargement du trace
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
			// Bornes km masquees comme le reste : sinon celles de la course quittee restent affichees
			ToggleKms_Internal(RaceID, false);
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

/**
 * @brief Configure la sortie OWL de la map en SRT (premiere destination) et la demarre
 */
void ARaceManager::StartSRTOutput()
{
	AOWLMediaOutput* Output = Cast<AOWLMediaOutput>(UGameplayStatics::GetActorOfClass(GetWorld(), AOWLMediaOutput::StaticClass()));
	if (!Output || !Output->MediaOutputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[OWL] Aucun OWLMediaOutput dans la map : flux SRT non demarre"));
		USlateNotificationsBFL::SlateNotify(FText::FromString(TEXT("[OWL] Aucun OWLMediaOutput dans la map : flux SRT non demarre")), EMessageType::Error);
		return;
	}

	FOWLMediaOutputSettings& Settings = Output->MediaOutputComponent->Settings;
	if (Settings.Destinations.IsEmpty())
	{
		Settings.Destinations.AddDefaulted();
	}
	FOWLMediaOutputDestination& Destination = Settings.Destinations[0];
	Destination.bEnabled = true;
	Destination.OutputType = EOWLMediaOutputType::T_SRT;
	Destination.SRTSettings.StreamURL = SRTStreamURL;
	Destination.bEncodeAudio = (bSRTEncodeAudio || FParse::Param(FCommandLine::Get(), TEXT("SRTAudio"))) && !FParse::Param(FCommandLine::Get(), TEXT("NoSRTAudio"));
	UE_LOG(LogTemp, Log, TEXT("[OWL] Audio du flux SRT : %s"), Destination.bEncodeAudio ? TEXT("active") : TEXT("coupe"));

	// OWL ne gere correctement l'audio que si le mixeur UE est en stereo : avec un peripherique 7.1
	// (casque PRO X), le resampler 8 -> 2 corrompt le tas (crash), Stereo Downmix sort des NaN et
	// 7.1 aussi. On coupe donc l'audio si la sortie Windows par defaut n'est pas stereo.
	if (Destination.bEncodeAudio)
	{
		const FAudioDeviceHandle AudioDevice = GetWorld()->GetAudioDevice();
		const Audio::FMixerDevice* Mixer = static_cast<const Audio::FMixerDevice*>(AudioDevice.GetAudioDevice());
		const int32 NumChannels = Mixer ? Mixer->GetNumDeviceChannels() : 0;
		if (NumChannels != 2)
		{
			Destination.bEncodeAudio = false;
			const FString Msg = FString::Printf(TEXT("[OWL] Audio du flux coupe : la sortie audio Windows a %d canaux, il faut un peripherique stereo"), NumChannels);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
			USlateNotificationsBFL::SlateNotify(FText::FromString(Msg), EMessageType::Error);
		}
	}

	// -SRTAudioLayout=71|Downmix permet de retester les autres formats OWL (instables, voir plus haut).
	FString AudioLayout = TEXT("Stereo");
	FParse::Value(FCommandLine::Get(), TEXT("SRTAudioLayout="), AudioLayout);
	if (AudioLayout == TEXT("71"))
	{
		Settings.AudioChannelLayout = EOWLAudioChannelLayout::ACL_7POINT1;
	}
	else if (AudioLayout == TEXT("Downmix"))
	{
		Settings.AudioChannelLayout = EOWLAudioChannelLayout::ACL_STEREO_DOWNMIX;
	}
	else
	{
		Settings.AudioChannelLayout = EOWLAudioChannelLayout::ACL_STEREO;
	}
	UE_LOG(LogTemp, Log, TEXT("[OWL] Format audio du flux : %s"), *UEnum::GetValueAsString(Settings.AudioChannelLayout));

	// Start peut etre asynchrone au demarrage : le resultat definitif arrive par OnStart
	if (Output->Start())
	{
		UE_LOG(LogTemp, Log, TEXT("[OWL] Flux SRT demande vers %s"), *SRTStreamURL);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[OWL] Echec du demarrage du flux SRT vers %s"), *SRTStreamURL);
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[OWL] Echec du demarrage du flux SRT vers %s"), *SRTStreamURL)), EMessageType::Error);
	}
}
