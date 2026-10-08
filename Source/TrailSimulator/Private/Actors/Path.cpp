// All Rights Reserved

#include "TrailSimulator/Public/Actors/Path.h"
#include "BroadcastCaptureSubsystem.h"
#include "CesiumFlyToComponent.h"
#include "Cesium3DTileset.h"
#include "CesiumSampleHeightResult.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "CesiumGeoreference.h"
#include "CesiumGlobeAnchorComponent.h"
#include "CesiumOriginShiftComponent.h"
#include "SettingsSubsystem.h"
#include "RaceSubsystem.h"
#include "CheckpointSubsystem.h"
#include "CineCameraComponent.h"
#include "LoadingStatusSubsystem.h"
#include "OWLCaptureComponent.h"
#include "WeatherSubsystem.h"
#include "PathSubsystem.h"
#include "WorldUtils.h"
#include "Actors/Checkpoint.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Actors/Km.h"
#include "Actors/Generic/Checkpoint_Generic.h"
#include "Actors/GTWS/Checkpoint_GTWS.h"
#include "Actors/UTMB/Checkpoint_UTMB.h"
#include "Actors/Generic/Km_Generic.h"
#include "Actors/GTWS/Km_GTWS.h"
#include "Actors/Nike/Checkpoint_Nike.h"
#include "Actors/Nike/Km_Nike.h"
#include "Actors/UTMB/Km_UTMB.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

// -------------------- Geometry helpers
static float DistPointSegment3D(const FVector& P, const FVector& A, const FVector& B)
{
	const FVector AB = B - A;
	const float AB2 = AB.SizeSquared();
	if (AB2 <= KINDA_SMALL_NUMBER) return FVector::Dist(P, A);

	const float T = FMath::Clamp(FVector::DotProduct(P - A, AB) / AB2, 0.f, 1.f);
	return FVector::Dist(P, A + T * AB);
}

// -------------------- RDP on [First..Last]
static void RDP_Keep(const TArray<FVector>& Pts, int32 First, int32 Last, float EpsCm, TArray<bool>& bKeep)
{
	float MaxD = -1.f; int32 MaxI = INDEX_NONE;
	for (int32 i = First + 1; i < Last; ++i)
	{
		const float D = DistPointSegment3D(Pts[i], Pts[First], Pts[Last]);
		if (D > MaxD) { MaxD = D; MaxI = i; }
	}
	if (MaxI != INDEX_NONE && MaxD > EpsCm)
	{
		bKeep[MaxI] = true;
		RDP_Keep(Pts, First, MaxI, EpsCm, bKeep);
		RDP_Keep(Pts, MaxI, Last, EpsCm, bKeep);
	}
}

// -------------------- Constrained RDP: keep all LockedIndices
static TArray<int32> SimplifyRDP_Constrained(const TArray<FVector>& Pts, float EpsCm, TArray<int32> LockedIndices)
{
	TArray<int32> Keep;
	if (Pts.Num() < 2) return Keep;

	LockedIndices.AddUnique(0);
	LockedIndices.AddUnique(Pts.Num() - 1);
	LockedIndices.Sort();

	TArray<bool> bKeep;
	bKeep.Init(false, Pts.Num());
	for (int32 Idx : LockedIndices)
		if (Pts.IsValidIndex(Idx))
			bKeep[Idx] = true;

	for (int32 k = 0; k < LockedIndices.Num() - 1; ++k)
	{
		const int32 A = LockedIndices[k];
		const int32 B = LockedIndices[k + 1];
		if (B - A >= 2)
			RDP_Keep(Pts, A, B, EpsCm, bKeep);
	}

	for (int32 i = 0; i < Pts.Num(); ++i)
		if (bKeep[i])
			Keep.Add(i);

	return Keep;
}

// -------------------- Main: build simplified rail, preserving remarkable points
void BuildSimplifiedRailLocked(
	USplineComponent* Source,
	USplineComponent* Rail,
	float ToleranceMeters,                 // ex: 100.f → facteur de simplification
	const TArray<FVector>& RemarkableWorldPoints,
	float SampleStepMeters = 10.f          // ex: 10m (≈ epsilon/10)
)
{
	if (!Source || !Rail) return;

	const float EpsCm  = ToleranceMeters * 100.f;          // 100m -> 10 000cm
	const float StepCm = FMath::Max(100.f, SampleStepMeters * 100.f);

	// 1) Convert remarkable world points -> distances along Source spline (projection)
	TArray<float> LockedDistances;
	LockedDistances.Reserve(RemarkableWorldPoints.Num());
	for (const FVector& WP : RemarkableWorldPoints)
	{
		const float Key  = Source->FindInputKeyClosestToWorldLocation(WP);
		const float Dist = Source->GetDistanceAlongSplineAtSplineInputKey(Key);
		LockedDistances.Add(Dist);
	}

	// 2) Build sample distances: fixed step + locked distances
	const float Len = Source->GetSplineLength();
	TArray<float> SampleDists;
	for (float d = 0.f; d < Len; d += StepCm) SampleDists.Add(d);
	SampleDists.Add(Len);
	for (float d : LockedDistances) SampleDists.Add(FMath::Clamp(d, 0.f, Len));

	SampleDists.Sort();
	// unique (1cm tolerance)
	{
		TArray<float> Unique;
		const float Tol = 1.f;
		for (float d : SampleDists)
			if (Unique.Num() == 0 || FMath::Abs(Unique.Last() - d) > Tol)
				Unique.Add(d);
		SampleDists = MoveTemp(Unique);
	}

	// 3) Sample positions
	TArray<FVector> Samples;
	Samples.Reserve(SampleDists.Num());
	for (float d : SampleDists)
	{
		Samples.Add(Source->GetLocationAtDistanceAlongSpline(d, ESplineCoordinateSpace::World));
	}

	// 4) Map locked distances -> locked indices in Samples (nearest)
	TArray<int32> LockedIndices;
	LockedIndices.Reserve(LockedDistances.Num());
	for (float LockD : LockedDistances)
	{
		int32 BestIdx = 0;
		float BestAbs = FLT_MAX;
		for (int32 i = 0; i < SampleDists.Num(); ++i)
		{
			const float Abs = FMath::Abs(SampleDists[i] - LockD);
			if (Abs < BestAbs) { BestAbs = Abs; BestIdx = i; }
		}
		LockedIndices.AddUnique(BestIdx);
	}

	// 5) Constrained simplification
	const TArray<int32> KeepIdx = SimplifyRDP_Constrained(Samples, EpsCm, LockedIndices);

	// 6) Rebuild rail spline
	Rail->ClearSplinePoints(false);
	for (int32 i = 0; i < KeepIdx.Num(); ++i)
	{
		Rail->AddSplinePoint(Samples[KeepIdx[i]], ESplineCoordinateSpace::World, false);
		Rail->SetSplinePointType(i, ESplinePointType::Curve, false);
		// Rail->SetSplinePointType(i, ESplinePointType::CurveClamped, false);
	}
	Rail->SetClosedLoop(Source->IsClosedLoop(), false);
	Rail->UpdateSpline();
}

// -------------------- Runtime follow (Option A): tangent + WorldUp
static void FollowRail(USceneComponent* Follower, USplineComponent* Rail, float DistanceCm)
{
	if (!Follower || !Rail) return;

	const FVector Pos = Rail->GetLocationAtDistanceAlongSpline(DistanceCm, ESplineCoordinateSpace::World);
	const FVector Fwd = Rail->GetDirectionAtDistanceAlongSpline(DistanceCm, ESplineCoordinateSpace::World);
	const FRotator Rot = FRotationMatrix::MakeFromXZ(Fwd, FVector::UpVector).Rotator();

	Follower->SetWorldLocationAndRotation(Pos, Rot);
}

APath::APath()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetActorEnableCollision(false);
	
	Root = CreateDefaultSubobject<USceneComponent>("RootComponent");
	SetRootComponent(Root);
	Root->SetComponentTickEnabled(false);
	
	// Regular path
	SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("SplinePath"));
	SplinePath->SetupAttachment(Root);
	SplinePath->SetComponentTickEnabled(false);
	SplinePath->SetGenerateOverlapEvents(false);
	SplinePath->ClearSplinePoints();
	
	// Slope path
	SlopePath = CreateDefaultSubobject<USplineComponent>(TEXT("SlopePath"));
	SlopePath->SetupAttachment(Root);
	SlopePath->SetComponentTickEnabled(false);
	SlopePath->SetGenerateOverlapEvents(false);
	SlopePath->ClearSplinePoints();
	
	// Travel path
	TravelPath = CreateDefaultSubobject<USplineComponent>(TEXT("TravelPath"));
	TravelPath->SetupAttachment(Root);
	TravelPath->SetComponentTickEnabled(false);
	TravelPath->SetGenerateOverlapEvents(false);
	TravelPath->ClearSplinePoints();
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(TravelPath);
	SpringArmComponent->SetComponentTickEnabled(false);
	SpringArmComponent->TargetArmLength = 4000.f;
	SpringArmComponent->SetRelativeRotation(FRotator(-45.f, 180.f, 0.f));
	SpringArmComponent->AddWorldOffset(FVector(0.f, 0.f, 3000.f));
	SpringArmComponent->SocketOffset = FVector(0.f, 0.f, 3000.f);
	SpringArmComponent->TargetOffset = FVector(0.f, 0.f, 13000.f);
	SpringArmComponent->bDoCollisionTest = true;
	
	CameraComponent = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	CameraComponent->SetCurrentFocalLength(24.f);
	CameraComponent->Overscan = 1.0f;
	FCameraFilmbackSettings CameraFilmbackSettings = FCameraFilmbackSettings();
	CameraFilmbackSettings.SensorWidth = 23.76f;
	CameraFilmbackSettings.SensorHeight = 13.365f;
	CameraComponent->Filmback = CameraFilmbackSettings;
	
	FPlateCropSettings PlateCropSettings = FPlateCropSettings();
	PlateCropSettings.AspectRatio = 1.77f;
	CameraComponent->CropSettings = PlateCropSettings;
	
	FPostProcessSettings PostProcessSettings = FPostProcessSettings();
	PostProcessSettings.bOverride_AutoExposureMethod = 1;
	PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
	PostProcessSettings.bOverride_AutoExposureBias = 1;
	PostProcessSettings.AutoExposureBias = 0.f;
	PostProcessSettings.bOverride_AutoExposureSpeedDown = 1;
	PostProcessSettings.AutoExposureSpeedDown = 100.f;
	PostProcessSettings.bOverride_AutoExposureSpeedUp = 1;
	PostProcessSettings.AutoExposureSpeedUp = 100.f;
	CameraComponent->PostProcessSettings = PostProcessSettings;
	
	OwlCapture = CreateDefaultSubobject<UOWLCaptureComponent>(TEXT("OwlCapture"));
	OwlCapture->SetupAttachment(CameraComponent);
	OwlCapture->SetRelativeTransform(FTransform::Identity);
	OwlCapture->bPauseRendering = true;
	OwlCapture->PrimaryComponentTick.bStartWithTickEnabled = false;
	OwlCapture->SetComponentTickEnabled(false);
	
	// Globe anchor
	GlobeAnchorComponent = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchorComponent"));
	GlobeAnchorComponent->SetAdjustOrientationForGlobeWhenMoving(true);
	Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	GlobeAnchorComponent->SetGeoreference(Georeference);
	
	// Spline Static Mesh for Path & Slope
	ConstructorHelpers::FObjectFinder<UStaticMesh> SM(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/SM_Cylindre_1x1.SM_Cylindre_1x1"));
	if (SM.Succeeded())
	{
		SplineStaticMesh = SM.Object;
	}
	
	// MPC
	ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> MPCFinder(
		TEXT("/Game/LTVContent/Materials/Masters/MPC_Trail.MPC_Trail"));

	if (MPCFinder.Succeeded())
	{
		PulseMPC = MPCFinder.Object;
	}
}

void APath::BeginPlay()
{
	Super::BeginPlay();
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), true);
	WeatherSubsystem = FWorldUtils::GetGISubsystemOrLog<UWeatherSubsystem>(this, TEXT(__FUNCTION__), true);
	LoadingSubsystem = FWorldUtils::GetGISubsystemOrLog<ULoadingStatusSubsystem>(this, TEXT(__FUNCTION__), true);
	PathSubsystem = FWorldUtils::GetGISubsystemOrLog<UPathSubsystem>(this, TEXT(__FUNCTION__), true);
	BroadCastSubsystem = FWorldUtils::GetWorldSubsystemOrLog<UBroadcastCaptureSubsystem>(this, TEXT(__FUNCTION__), false);
	CheckpointSubsystem = FWorldUtils::GetGISubsystemOrLog<UCheckpointSubsystem>(this, TEXT(__FUNCTION__), true);
	CheckpointSubsystem->OnCheckpointsDatasGathered.AddDynamic(this, &APath::HandleCheckpointsDatasGathered);
	
	DynaPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

/**
 * @brief Drawing the path
 * @note Méthode interne
 */


TSubclassOf<AActor> APath::GetCheckpointClassForRace(const FRaceSetup& RaceSetup) const
{
	if (RaceSetup.templateName == TEXT("UTMB"))
	{
		return ACheckpoint_UTMB::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("GTWS"))
	{
		return ACheckpoint_GTWS::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("Nike"))
	{
		return ACheckpoint_Nike::StaticClass();
	}
	return ACheckpoint_Generic::StaticClass();
}


void APath::DrawPath(int64 RaceID, FRacePath RacePathDatas)
{
	DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	FlyComp = Cast<UCesiumFlyToComponent>(DynaPawn->GetComponentByClass(UCesiumFlyToComponent::StaticClass()));
	ShiftComp = Cast<UCesiumOriginShiftComponent>(DynaPawn->GetComponentByClass(UCesiumOriginShiftComponent::StaticClass()));

	RaceId = RaceID;
	const FName IdPath(*FString::Printf(TEXT("SpawnPath_%lld"), RaceId));
	// LoadingSubsystem->RegisterTask(IdPath, FText::FromString(FString::Printf(TEXT("Path %lld construction"), RaceID)));
	const FName IdKm(*FString::Printf(TEXT("BuildKms_%lld"), RaceID));
	LoadingSubsystem->RegisterTask(IdKm, FText::FromString(FString::Printf(TEXT("Kms %lld construction"), RaceID)));

	if (RacePathDatas.Points.Num() == 0)
	{
		LoadingSubsystem->Fail(IdPath, FText::FromString(FString::Printf(TEXT("Path %lld is empty"), RaceID)));
		return;
	}
	const FName IdCheckpoints(*FString::Printf(TEXT("SpawnCheckpoints_%lld"), RaceID));
	LoadingSubsystem->SetRunning(IdCheckpoints);
	LoadingSubsystem->SetRunning(IdPath);

	if (!Georeference)
	{
		LoadingSubsystem->Fail(IdPath, FText::FromString(FString::Printf(TEXT("Path %lld : no georeference"), RaceID)));
		return;
	}

	RacePath = RacePathDatas;
	RaceId = RaceID;

	if (!SettingsSubsystem->DoesSettingsExist(RaceID))
	{
		FSettings NewSettings = FSettings(
			RaceID,
			TEXT("https://simulacre.ltvprod.cc/"),
			1.0f,
			0.f,
			1.f,
			10.f,
			0,
			FMinMax(FVector(30.0f), FVector(200.f)),
			-25.f,
			50000.f,
			-500.f);
			SettingsSubsystem->CreateTrailSettingsById(RaceID, NewSettings);
	}

	// Avec le recalage sur les tuiles, le ZOffset n'est plus qu'un reglage fin (0 par defaut)
	ZOffset = SettingsSubsystem->GetZOffsetById(RaceID);

	if (ShiftComp)
		ShiftComp->SetActive(false);

	Root->SetMobility(EComponentMobility::Movable);
	Georeference->SetOriginLongitudeLatitudeHeight(
		FVector(RacePath.Points[0].lon, RacePath.Points[0].lat, RacePath.Points[0].ele));
	OnGeoRefLocation.Broadcast(RaceID,
		FVector(RacePath.Points[0].lon, RacePath.Points[0].lat, RacePath.Points[0].ele));

	FVector PathLocation = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(
		FVector(RacePath.Points[0].lon, RacePath.Points[0].lat, RacePath.Points[0].ele));
	PathLocation.Z += ZOffset;
	SetActorLocation(PathLocation, false);

	StartDrape(RaceID);
}

/**
 * @brief Tuileset du terrain : Google Photorealistic 3D Tiles de preference, sinon le premier tileset du niveau
 */
ACesium3DTileset* APath::FindTerrainTileset() const
{
	ACesium3DTileset* Fallback = nullptr;
	for (TActorIterator<ACesium3DTileset> It(GetWorld()); It; ++It)
	{
		ACesium3DTileset* Tileset = *It;
		if (!IsValid(Tileset) || Tileset->IsHidden()) continue;
		if (Tileset->GetIonAssetID() == 2275207 || Tileset->GetUrl().Contains(TEXT("tile.googleapis.com")))
		{
			return Tileset;
		}
		if (!Fallback) Fallback = Tileset;
	}
	return Fallback;
}

FString APath::GetDrapeCachePath(int64 RaceID, uint32 Hash) const
{
	return FPaths::ProjectSavedDir() / TEXT("DrapeCache") / FString::Printf(TEXT("Race_%lld_%08x.bin"), RaceID, Hash);
}

/**
 * @brief Recalage : l'altitude GPS (au-dessus du niveau de la mer, bruitee) est remplacee par la hauteur
 * des tuiles sous chaque point (au-dessus de l'ellipsoide, ce qu'attend Cesium). Des echantillons
 * supplementaires sur chaque troncon detectent les cretes que la ligne droite entre deux points couperait.
 * Les hauteurs sont mises en cache sur disque : le chargement suivant de la meme course est immediat.
 */
void APath::StartDrape(int64 RaceID)
{
	const int32 NumPts = RacePath.Points.Num();

	// Requete : les points du trace, puis les echantillons intermediaires de chaque troncon
	TArray<FVector> Query;
	TArray<int32> SubSegment;
	TArray<float> SubAlpha;
	Query.Reserve(NumPts * 2);
	for (const FRacePathPoint& P : RacePath.Points)
	{
		Query.Add(FVector(P.lon, P.lat, P.ele));
	}
	const double SpacingCm = FMath::Max(5.0, (double)DrapeSampleSpacingM) * 100.0;
	for (int32 i = 0; i < NumPts - 1; ++i)
	{
		const FRacePathPoint& A = RacePath.Points[i];
		const FRacePathPoint& B = RacePath.Points[i + 1];
		const FVector UA = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(A.lon, A.lat, 0.0));
		const FVector UB = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(B.lon, B.lat, 0.0));
		const int32 NumSub = FMath::Min(FMath::FloorToInt32(FVector::Dist2D(UA, UB) / SpacingCm), 32);
		for (int32 k = 1; k <= NumSub; ++k)
		{
			const float T = float(k) / float(NumSub + 1);
			Query.Add(FVector(FMath::Lerp((double)A.lon, (double)B.lon, (double)T), FMath::Lerp((double)A.lat, (double)B.lat, (double)T), FMath::Lerp(A.ele, B.ele, T)));
			SubSegment.Add(i);
			SubAlpha.Add(T);
		}
	}

	const uint32 Hash = FCrc::MemCrc32(Query.GetData(), Query.Num() * Query.GetTypeSize());
	const int32 Serial = ++DrapeSerial;

	// Cache disque
	{
		TArray<uint8> Bytes;
		const int32 Expected = Query.Num() * (sizeof(double) + 1);
		if (FFileHelper::LoadFileToArray(Bytes, *GetDrapeCachePath(RaceID, Hash), FILEREAD_Silent) && Bytes.Num() == Expected)
		{
			TArray<FVector> Cached = Query;
			TArray<bool> bOk;
			bOk.SetNumUninitialized(Query.Num());
			const double* Heights = reinterpret_cast<const double*>(Bytes.GetData());
			const uint8* Flags = Bytes.GetData() + Query.Num() * sizeof(double);
			for (int32 i = 0; i < Query.Num(); ++i)
			{
				Cached[i].Z = Heights[i];
				bOk[i] = Flags[i] != 0;
			}
			UE_LOG(LogTemp, Log, TEXT("[Path] Race %lld : hauteurs du trace lues dans le cache (%d echantillons)"), RaceID, Query.Num());
			HandleDrapeHeights(RaceID, Serial, Cached, bOk, NumPts, SubSegment, SubAlpha);
			return;
		}
	}

	ACesium3DTileset* Tileset = FindTerrainTileset();
	if (!Tileset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Path] Race %lld : aucun tileset Cesium, trace place sur l'altitude GPS"), RaceID);
		HandleDrapeHeights(RaceID, Serial, Query, TArray<bool>(), NumPts, SubSegment, SubAlpha);
		return;
	}

	const FName IdPath(*FString::Printf(TEXT("SpawnPath_%lld"), RaceID));
	LoadingSubsystem->Update(IdPath, 0.f, FText::FromString(FString::Printf(TEXT("Recalage sur les tuiles (%d echantillons)"), Query.Num())));

	bDrapePending = true;
	GetWorldTimerManager().SetTimer(DrapeTimeoutHandle,
		FTimerDelegate::CreateUObject(this, &APath::DrapeTimedOut, RaceID, Serial),
		FMath::Max(5.f, DrapeTimeoutSeconds), false);

	TWeakObjectPtr<APath> WeakThis(this);
	Tileset->SampleHeightMostDetailed(Query, FCesiumSampleHeightMostDetailedCallback::CreateLambda(
		[WeakThis, RaceID, Serial, Query, NumPts, SubSegment, SubAlpha, Hash](ACesium3DTileset*, const TArray<FCesiumSampleHeightResult>& Results, const TArray<FString>& Warnings)
		{
			APath* Self = WeakThis.Get();
			if (!Self || !Self->bDrapePending || Serial != Self->DrapeSerial) return;
			Self->bDrapePending = false;
			Self->GetWorldTimerManager().ClearTimer(Self->DrapeTimeoutHandle);

			for (const FString& W : Warnings)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Path] Race %lld : %s"), RaceID, *W);
			}
			TArray<FVector> Sampled = Query;
			TArray<bool> bOk;
			bOk.Init(false, Query.Num());
			int32 NumOk = 0;
			for (int32 i = 0; i < Results.Num() && i < Query.Num(); ++i)
			{
				if (Results[i].SampleSuccess)
				{
					Sampled[i].Z = Results[i].LongitudeLatitudeHeight.Z;
					bOk[i] = true;
					++NumOk;
				}
			}
			UE_LOG(LogTemp, Log, TEXT("[Path] Race %lld : %d/%d hauteurs lues sur les tuiles"), RaceID, NumOk, Query.Num());

			// Cache seulement si l'echantillonnage est quasi complet (sinon on retentera au prochain chargement)
			if (NumOk >= Query.Num() * 95 / 100)
			{
				TArray<uint8> Bytes;
				Bytes.SetNumUninitialized(Query.Num() * (sizeof(double) + 1));
				double* Heights = reinterpret_cast<double*>(Bytes.GetData());
				uint8* Flags = Bytes.GetData() + Query.Num() * sizeof(double);
				for (int32 i = 0; i < Query.Num(); ++i)
				{
					Heights[i] = Sampled[i].Z;
					Flags[i] = bOk[i] ? 1 : 0;
				}
				FFileHelper::SaveArrayToFile(Bytes, *Self->GetDrapeCachePath(RaceID, Hash));
			}
			Self->HandleDrapeHeights(RaceID, Serial, Sampled, bOk, NumPts, SubSegment, SubAlpha);
		}));
}

void APath::DrapeTimedOut(int64 RaceID, int32 Serial)
{
	if (!bDrapePending || Serial != DrapeSerial) return;
	bDrapePending = false;
	UE_LOG(LogTemp, Warning, TEXT("[Path] Race %lld : les tuiles n'ont pas repondu, trace place sur l'altitude GPS"), RaceID);

	TArray<FVector> Query;
	for (const FRacePathPoint& P : RacePath.Points)
	{
		Query.Add(FVector(P.lon, P.lat, P.ele));
	}
	HandleDrapeHeights(RaceID, Serial, Query, TArray<bool>(), RacePath.Points.Num(), TArray<int32>(), TArray<float>());
}

/**
 * @brief Hauteurs finales : surface des tuiles + DrapeClearanceM. Un point sans reponse garde l'ecart
 * tuiles/GPS de ses voisins ; sans aucune reponse, altitude GPS + DrapeFallbackGeoidM. Sur un troncon
 * dont un echantillon depasse la ligne droite, tous ses echantillons deviennent des points du trace.
 */
void APath::HandleDrapeHeights(int64 RaceID, int32 Serial, const TArray<FVector>& Query, const TArray<bool>& bSuccess, int32 NumPathPoints, const TArray<int32>& SubSegment, const TArray<float>& SubAlpha)
{
	if (Serial != DrapeSerial) return;
	const int32 NumPts = NumPathPoints;
	if (NumPts != RacePath.Points.Num()) return;

	auto IsOk = [&bSuccess](int32 i) { return bSuccess.IsValidIndex(i) && bSuccess[i]; };

	// Ecart tuiles - GPS aux points du trace, interpole la ou l'echantillonnage a echoue
	TArray<double> Delta;
	Delta.Init(DrapeFallbackGeoidM, NumPts);
	{
		int32 Prev = INDEX_NONE;
		for (int32 i = 0; i < NumPts; ++i)
		{
			if (!IsOk(i)) continue;
			Delta[i] = Query[i].Z - RacePath.Points[i].ele;
			const double DPrev = Prev == INDEX_NONE ? Delta[i] : Delta[Prev];
			for (int32 j = (Prev == INDEX_NONE ? 0 : Prev + 1); j < i; ++j)
			{
				const double T = Prev == INDEX_NONE ? 1.0 : double(j - Prev) / double(i - Prev);
				Delta[j] = FMath::Lerp(DPrev, Delta[i], T);
			}
			Prev = i;
		}
		if (Prev != INDEX_NONE)
		{
			for (int32 j = Prev + 1; j < NumPts; ++j) Delta[j] = Delta[Prev];
		}
	}

	TArray<double> PointH;
	PointH.SetNumUninitialized(NumPts);
	for (int32 i = 0; i < NumPts; ++i)
	{
		PointH[i] = RacePath.Points[i].ele + Delta[i] + DrapeClearanceM;
	}

	// Troncons dont le relief depasse la ligne droite
	TArray<bool> bDensify;
	bDensify.Init(false, FMath::Max(0, NumPts - 1));
	for (int32 s = 0; s < SubSegment.Num(); ++s)
	{
		const int32 Q = NumPts + s;
		if (!IsOk(Q)) continue;
		const int32 Seg = SubSegment[s];
		const double Chord = FMath::Lerp(PointH[Seg], PointH[Seg + 1], (double)SubAlpha[s]);
		if (Query[Q].Z + DrapeClearanceM > Chord + 0.5) bDensify[Seg] = true;
	}

	FRacePath Draped;
	TArray<double> HeightsM;
	Draped.Points.Reserve(NumPts + SubSegment.Num());
	HeightsM.Reserve(NumPts + SubSegment.Num());
	int32 NumAdded = 0;
	int32 s = 0;
	for (int32 i = 0; i < NumPts; ++i)
	{
		Draped.Points.Add(RacePath.Points[i]);
		HeightsM.Add(PointH[i]);
		for (; s < SubSegment.Num() && SubSegment[s] == i; ++s)
		{
			if (!bDensify[i] || !IsOk(NumPts + s)) continue;
			const FRacePathPoint& A = RacePath.Points[i];
			const FRacePathPoint& B = RacePath.Points[i + 1];
			// Point intermediaire sans checkpoint ; ele interpole (sert au calcul des pentes)
			FRacePathPoint Mid;
			Mid.lon = Query[NumPts + s].X;
			Mid.lat = Query[NumPts + s].Y;
			Mid.ele = FMath::Lerp(A.ele, B.ele, SubAlpha[s]);
			Draped.Points.Add(Mid);
			HeightsM.Add(Query[NumPts + s].Z + DrapeClearanceM);
			++NumAdded;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[Path] Race %lld : %d points recales, %d points ajoutes sur les cretes"), RaceID, NumPts, NumAdded);

	RacePath = MoveTemp(Draped);
	BuildPathGeometry(RaceID, HeightsM);
}

void APath::BuildPathGeometry(int64 RaceID, const TArray<double>& HeightsM)
{
	const FName IdPath(*FString::Printf(TEXT("SpawnPath_%lld"), RaceID));
	const FName IdCheckpoints(*FString::Printf(TEXT("SpawnCheckpoints_%lld"), RaceID));
	const FName IdKm(*FString::Printf(TEXT("BuildKms_%lld"), RaceID));

	SplinePath->ClearSplinePoints(false);
	SlopePath->ClearSplinePoints(false);
	TravelPath->ClearSplinePoints(false);
	LockedPoints.Reset();
	int32 cpt = 0;
	const int32 Total = RacePath.Points.Num();
	for (const FRacePathPoint& Point : RacePath.Points)
	{
		FVector UE = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Point.lon, Point.lat, HeightsM[cpt]));
		UE.Z += ZOffset;
		SplinePath->AddSplinePoint(UE, ESplineCoordinateSpace::World, false);
		SlopePath->AddSplinePoint(UE, ESplineCoordinateSpace::World, false);
		if (FMath::Modulo(cpt, 25) == 0 || cpt == Total -1)
		{
			const float P = float(cpt + 1) / float(Total);
			LoadingSubsystem->Update(IdPath, P,
				FText::FromString(FString::Printf(TEXT("%d/%d"), cpt, Total)));
		}

		// Checkpoints
		LoadingSubsystem->SetRunning(IdCheckpoints);
		if (!Point.datas.name.IsEmpty())
		{
			UWorld* World = GetWorld();
			if (!World) return;

			const FVector Location = UE;
			LockedPoints.Add(UE);

			const FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
			const FVector Scale = FVector(40.0f);

			const FTransform SpawnTransform = FTransform(Rotation, Location, Scale);
			const FRaceSetup& RaceSetup = RaceSubsystem->GetRaceSetupById(RaceID);
			TObjectPtr<AActor> SpawnedActor = nullptr;

			const TSubclassOf<AActor> CheckpointClass = GetCheckpointClassForRace(RaceSetup);
			SpawnedActor = CheckpointSubsystem->SpawnCheckpointActor(RaceID, Point.datas.checkpointId, World, CheckpointClass, SpawnTransform);

			if (ICheckpointInterface* CheckpointInterface = Cast<ICheckpointInterface>(SpawnedActor))
			{
				CheckpointInterface->UpdateCheckpoint(Point.datas, RaceSetup);
				SpawnedActor->SetActorHiddenInGame(true);
				SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
				LoadingSubsystem->Update(IdCheckpoints, 1.f, FText::FromString(FString::Printf(TEXT("Checkpoint %s spawned"), *Point.datas.name)));
			}
		}
		cpt++;
	}
	LoadingSubsystem->Complete(IdCheckpoints, FText::FromString(FString::Printf(TEXT("Checkpoints spawned for race %lld"), RaceID)));

	SplinePath->UpdateSpline();
	SlopePath->UpdateSpline();

	BuildSimplifiedRailLocked(SplinePath, TravelPath, ToleranceMeters, LockedPoints, SampleStepMeters);
	FollowRail(SpringArmComponent, TravelPath, DistanceCm);
	BuildKms(RaceID);

	LoadingSubsystem->Complete(IdKm, FText::FromString(FString::Printf(TEXT("Kms %lld built"), RaceID)));
	RebuildPathSplineMeshes(RaceID);
	RebuildSlopeSplineMeshes(RaceID);
}

/**
 * @brief Création du tracé
 * @note Méthode interne
 */
void APath::RebuildPathSplineMeshes(int64 RaceID)
{	
	RaceSubsystem = FWorldUtils::GetGISubsystemOrLog<URaceSubsystem>(this, TEXT(__FUNCTION__), true);
	const FRaceSetup& CurrentSetup = RaceSubsystem->GetRaceSetupById(RaceID);
	FLinearColor LinearColor = FLinearColor::FromSRGBColor(FColor::FromHex(CurrentSetup.color));
	
	Root->SetMobility(EComponentMobility::Static);
	const float TotalLen = SplinePath->GetSplineLength();
	for (int32 i=0; i< SplinePath->GetNumberOfSplinePoints()-1;++i)
	{
		const float StartDist = SplinePath->GetDistanceAlongSplineAtSplinePoint(i);
		const float EndDist   = SplinePath->GetDistanceAlongSplineAtSplinePoint(i + 1);
		
		USplineMeshComponent* SMC = NewObject<USplineMeshComponent>(this);
		PathSplineMeshes.Add(SMC);
		SMC->CreationMethod = EComponentCreationMethod::Instance;
		this->AddInstanceComponent(SMC);

		SMC->SetComponentTickEnabled(false);
		SMC->SetGenerateOverlapEvents(false);
		SMC->SetMobility(EComponentMobility::Movable);
		SMC->SetStaticMesh(SplineStaticMesh);

		SMC->SetupAttachment(SlopePath);
		SMC->SetTranslucentSortPriority(-20); // trace dessine avant les poteaux (-10) et les panneaux
			// Le materiau rapproche le trace de la camera (CPD 17) : il ne doit pas etre ecarte par l'occlusion
			// des tuiles qu'il recouvre. Le flag custom depth coupe l'occlusion ; materiau translucide sans
			// ecriture custom depth, donc rien n'est dessine dans ce buffer.
			SMC->SetRenderCustomDepth(true);
		SMC->RegisterComponent();
			
		SMC->SetForwardAxis(ESplineMeshAxis::Z);
		SMC->SetStartScale(FVector2D(50.f, 50.f));
		SMC->SetEndScale(FVector2D(50.f, 50.f));
		
		FRacePathPoint Point1 = RacePath.Points[i];
		FRacePathPoint Point2 = RacePath.Points[i+1];
		FVector Point1Location = SplinePath->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		FVector Point2Location = SplinePath->GetLocationAtSplinePoint(i+1, ESplineCoordinateSpace::Local);
		FVector Point1Tangent = SplinePath->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		FVector Point2Tangent = SplinePath->GetTangentAtSplinePoint(i+1, ESplineCoordinateSpace::Local);
		SMC->SetStartAndEnd(Point1Location, Point1Tangent, Point2Location, Point2Tangent);
		
		// StartNorm
		SMC->SetCustomPrimitiveDataFloat(0, StartDist / TotalLen);
		//LenNorm
		SMC->SetCustomPrimitiveDataFloat(1, (EndDist - StartDist) / TotalLen);
		
		// Start / End en local space du composant
		SMC->SetCustomPrimitiveDataVector3(2, Point1Location);	// M_Glow
		SMC->SetCustomPrimitiveDataVector3(6, Point2Location);
		SMC->SetCustomPrimitiveDataFloat(10, 0.f); // Slope value
		SMC->SetCustomPrimitiveDataFloat(11, 0.f); // UseSlope = 0	M_MasterPC
		SMC->SetCustomPrimitiveDataVector4(12, LinearColor); // Color (12->15)
		SMC->SetCustomPrimitiveDataFloat(16, SettingsSubsystem->GetGlowById(RaceID));
		SMC->SetCustomPrimitiveDataFloat(17, CameraBiasRatio); // decalage vers la camera
		SMC->SetHiddenInGame(true);
	}
	Root->SetMobility(EComponentMobility::Movable);
}

/**
 * @brief Création de la slope
 * @note Méthode interne
 */
void APath::RebuildSlopeSplineMeshes(int64 RaceID)
{
	constexpr float SegmentLenM = 150.f;
	TArray<FPointIndexSegment> OutSegments;
	
	OutSegments.Reset();
	const int32 NumPts = SlopePath->GetNumberOfSplinePoints();
	if (NumPts < 2) return;

	FPointIndexSegment CurrentSegment;
	CurrentSegment.StartIndex = 0;
	CurrentSegment.StartDistanceCm = SlopePath->GetDistanceAlongSplineAtSplinePoint(0);
	CurrentSegment.Indices.Add(0);

	for (int32 i = 1; i < NumPts; ++i)
	{
		CurrentSegment.Indices.Add(i);

		const float Di = SlopePath->GetDistanceAlongSplineAtSplinePoint(i);
		const float Delta = Di - CurrentSegment.StartDistanceCm;

		if (Delta >= SegmentLenM && CurrentSegment.Indices.Num() >= 2)
		{
			CurrentSegment.EndIndex = i;
			CurrentSegment.EndDistanceCm = Di;
			OutSegments.Add(CurrentSegment);

			CurrentSegment = FPointIndexSegment{};
			CurrentSegment.StartIndex = i;
			CurrentSegment.StartDistanceCm = Di;
			CurrentSegment.Indices.Add(i);
		}
	}

	if (CurrentSegment.Indices.Num() >= 2)
	{
		const int32 LastIdx = CurrentSegment.Indices.Last();
		CurrentSegment.EndIndex = LastIdx;
		CurrentSegment.EndDistanceCm = SlopePath->GetDistanceAlongSplineAtSplinePoint(LastIdx);
		OutSegments.Add(CurrentSegment);
	}
	const float TotalLen = SlopePath->GetSplineLength();

	// Pente en % (metres de denivele pour 100 m) de chaque troncon entre deux points GPS, et valeur
	// a chaque jonction = moyenne des deux troncons qui s'y touchent. Le materiau garde la couleur
	// du troncon en son centre et fond vers la valeur de jonction pres de ses extremites : deux
	// troncons voisins ont donc la meme couleur a leur jonction, sans coupe nette.
	// CPD 10 = pente du troncon, CPD 12 = jonction de debut, CPD 13 = jonction de fin.
	TArray<float> DistM;
	DistM.SetNumUninitialized(NumPts);
	for (int32 i = 0; i < NumPts; ++i)
	{
		DistM[i] = SlopePath->GetDistanceAlongSplineAtSplinePoint(i) / 100.f;
	}
	// Pente absolue (la couleur ne depend que de la valeur absolue) mesuree sur au moins
	// SlopeWindowM autour du milieu du troncon : l'altitude GPS est bruitee (pentes brutes qui
	// alternent -15 % / +12 % d'un point a l'autre), et moyenner des pentes de signes opposes a
	// une jonction donnait une valeur proche de 0, donc des coupes claires.
	constexpr float SlopeWindowM = 250.f;
	TArray<float> SegGrade; // SegGrade[i] = pente absolue du troncon [i, i+1]
	SegGrade.SetNumZeroed(NumPts - 1);
	for (int32 i = 0; i < NumPts - 1; ++i)
	{
		const float Mid = 0.5f * (DistM[i] + DistM[i + 1]);
		int32 J0 = i, J1 = i + 1;
		while (J0 > 0 && Mid - DistM[J0] < 0.5f * SlopeWindowM) --J0;
		while (J1 < NumPts - 1 && DistM[J1] - Mid < 0.5f * SlopeWindowM) ++J1;
		const float Run = DistM[J1] - DistM[J0];
		SegGrade[i] = Run > 1.f ? FMath::Abs(RacePath.Points[J1].ele - RacePath.Points[J0].ele) / Run * 100.f : 0.f;
	}
	// Lissage supplementaire entre troncons voisins : evite les petits morceaux de couleur isoles
	constexpr int32 SlopeSmoothPasses = 4;
	for (int32 Pass = 0; Pass < SlopeSmoothPasses && SegGrade.Num() > 2; ++Pass)
	{
		TArray<float> Prev = SegGrade;
		for (int32 i = 1; i < SegGrade.Num() - 1; ++i)
		{
			SegGrade[i] = 0.25f * Prev[i - 1] + 0.5f * Prev[i] + 0.25f * Prev[i + 1];
		}
	}
	TArray<float> Junction; // Junction[i] = valeur au point i
	Junction.SetNumZeroed(NumPts);
	for (int32 i = 0; i < NumPts; ++i)
	{
		const float Before = i > 0 ? SegGrade[i - 1] : SegGrade[0];
		const float After = i < NumPts - 1 ? SegGrade[i] : SegGrade[NumPts - 2];
		Junction[i] = 0.5f * (Before + After);
	}

	Root->SetMobility(EComponentMobility::Static);
	for (const FPointIndexSegment& Seg : OutSegments)
	{
		const int32 I0 = Seg.Indices[0];
		const int32 I1 = Seg.Indices.Last();
		
		for (int32 k = 0; k < Seg.Indices.Num() - 1; ++k)
		{
			USplineMeshComponent* SMC = NewObject<USplineMeshComponent>(this);
			SlopeSplineMeshes.Add(SMC);
			SMC->CreationMethod = EComponentCreationMethod::Instance;
			this->AddInstanceComponent(SMC);

			SMC->SetComponentTickEnabled(false);
			SMC->SetGenerateOverlapEvents(false);
			SMC->SetMobility(EComponentMobility::Movable);
			SMC->SetStaticMesh(SplineStaticMesh);

			SMC->SetupAttachment(SlopePath);
			SMC->SetTranslucentSortPriority(-20); // trace dessine avant les poteaux (-10) et les panneaux
			// Le materiau rapproche le trace de la camera (CPD 17) : il ne doit pas etre ecarte par l'occlusion
			// des tuiles qu'il recouvre. Le flag custom depth coupe l'occlusion ; materiau translucide sans
			// ecriture custom depth, donc rien n'est dessine dans ce buffer.
			SMC->SetRenderCustomDepth(true);
			SMC->RegisterComponent();
			
			SMC->SetForwardAxis(ESplineMeshAxis::Z);
			SMC->SetStartScale(FVector2D(50.f, 50.f));
			SMC->SetEndScale(FVector2D(50.f, 50.f));
			
			const int32 A = Seg.Indices[k];
			const int32 B = Seg.Indices[k + 1];
			const FVector StartPos = SlopePath->GetLocationAtSplinePoint(A, ESplineCoordinateSpace::Local);
			const FVector EndPos   = SlopePath->GetLocationAtSplinePoint(B, ESplineCoordinateSpace::Local);
			const FVector StartTan = SlopePath->GetTangentAtSplinePoint(A, ESplineCoordinateSpace::Local);
			const FVector EndTan   = SlopePath->GetTangentAtSplinePoint(B, ESplineCoordinateSpace::Local);
			SMC->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
			
			const float StartDist = SplinePath->GetDistanceAlongSplineAtSplinePoint(I0);
			const float EndDist   = SplinePath->GetDistanceAlongSplineAtSplinePoint(I1);
			
			// StartNorm
			SMC->SetCustomPrimitiveDataFloat(0, StartDist / TotalLen);
			//LenNorm
			SMC->SetCustomPrimitiveDataFloat(1, (EndDist - StartDist) / TotalLen);
		
			// Start / End en local space du composant
			SMC->SetCustomPrimitiveDataVector3(2, StartPos);	// M_Glow
			SMC->SetCustomPrimitiveDataVector3(6, EndPos);
			
			SMC->SetCustomPrimitiveDataFloat(10, SegGrade[A]);	// SlopeMeters (signé) : pente % du troncon
			SMC->SetCustomPrimitiveDataFloat(12, Junction[A]);	// valeur a la jonction de debut
			SMC->SetCustomPrimitiveDataFloat(13, Junction[B]);	// valeur a la jonction de fin
			SMC->SetCustomPrimitiveDataFloat(11, 1.f);	// UseSlope = 1
			SMC->SetCustomPrimitiveDataFloat(16, SettingsSubsystem->GetGlowById(RaceID));
			SMC->SetCustomPrimitiveDataFloat(17, CameraBiasRatio); // decalage vers la camera
			SMC->SetHiddenInGame(true);
		}
	}
	Root->SetMobility(EComponentMobility::Movable);
	OnPathEndDrawing.Broadcast(RaceID);
	
	const FName IdPath(*FString::Printf(TEXT("SpawnPath_%lld"), RaceId));
	LoadingSubsystem->Complete(IdPath, FText::FromString(FString::Printf(TEXT("Path %lld built"), RaceID)));
	const FName IdCheckpoints(*FString::Printf(TEXT("SpawnCheckpoints_%lld"), RaceID));
	LoadingSubsystem->Complete(IdCheckpoints);
}

TSubclassOf<AActor> APath::GetKmClassForRace(const FRaceSetup& RaceSetup) const
{
	if (RaceSetup.templateName == TEXT("UTMB"))
	{
		return AKm_UTMB::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("GTWS"))
	{
		return AKm_GTWS::StaticClass();
	}
	if (RaceSetup.templateName == TEXT("Nike"))
	{
		return AKm_Nike::StaticClass();
	}
	return AKm_Generic::StaticClass();
}

/**
 * @brief Création des bornes kilométriques
 * @return Méthode interne
 */
bool APath::BuildKms(int64 RaceID)
{
	const FName IdKm(*FString::Printf(TEXT("BuildKms_%lld"), RaceID));
	LoadingSubsystem->SetRunning(IdKm, FText::FromString(FString::Printf(TEXT("Kms %lld construction"), RaceID)));
	
	float MaxDist = SplinePath->GetSplineLength();
	for (float dist = 100000.f; dist <= MaxDist; dist+=100000.f)
	{
		FVector KmLocation = SplinePath->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		if (UWorld* World = GetWorld())
		{
			const FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
			const FVector Scale = FVector(5.f);
			const FTransform SpawnTransform = FTransform(Rotation, KmLocation, Scale);
			const FRaceSetup& RaceSetup = RaceSubsystem->GetRaceSetupById(RaceID);
			TObjectPtr<AActor> SpawnedActor = nullptr;
			const TSubclassOf<AActor> KmClass = GetKmClassForRace(RaceSetup);
			SpawnedActor = PathSubsystem->SpawnKmActor(World, KmClass, SpawnTransform);
			
			if(TObjectPtr<AKm> Km = Cast<AKm>(SpawnedActor))
			{
				KmsArray.AddUnique(Km);
			}
			if (IKmInterface* KmInterface = Cast<IKmInterface>(SpawnedActor)){
			
				KmInterface->UpdateKm(RaceID, FText::FromString(FString::Printf(TEXT("%.0f Km"), SplinePath->GetDistanceAlongSplineAtLocation(KmLocation, ESplineCoordinateSpace::World)/100000.f)));
				SpawnedActor->SetActorHiddenInGame(true);
				SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}
	Kms.Add(RaceID, KmsArray);
	return true;
}

/**
 * @brief Travelling
 */
void APath::StartTravelForward()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (!OriginalViewTarget)
		{
			OriginalViewTarget = PC->GetViewTarget();
		}
		BroadCastSubsystem->RequestViewTarget(PC, this, 1.f);
	}
	bForward = true;
	SetActorTickEnabled(true);
}
void APath::StartTravelBackward()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (!OriginalViewTarget)
		{
			OriginalViewTarget = PC->GetViewTarget();
		}
		BroadCastSubsystem->RequestViewTarget(PC, this, 1.f);
	}
	bForward = false;
	DistanceCm = TravelPath->GetSplineLength();
	SetActorTickEnabled(true);
}
void APath::StopTravel()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (OriginalViewTarget)
		{
			BroadCastSubsystem->RequestViewTarget(PC, OriginalViewTarget, 1.f);
			OriginalViewTarget = nullptr;
		}
	}
	DistanceCm = 0.f;
	FollowRail(SpringArmComponent, TravelPath, DistanceCm);
	SetActorTickEnabled(false);
}

/**
 * @brief Pulse
 */
void APath::StartPulse() const
{
	UWorld* World = GetWorld();
	if (!World) return;

	UMaterialParameterCollection* MPC = PulseMPC.LoadSynchronous();
	if (PulseMPC)
	{
		if (UMaterialParameterCollectionInstance* Inst = GetWorld()->GetParameterCollectionInstance(MPC))
		{
			Inst->SetScalarParameterValue(TEXT("PulseEnabled"), 1.0f);
		}
	}
}
void APath::StopPulse() const
{
	UWorld* World = GetWorld();
	if (!World) return;

	UMaterialParameterCollection* MPC = PulseMPC.LoadSynchronous();
	if (PulseMPC)
	{
		if (UMaterialParameterCollectionInstance* Inst = GetWorld()->GetParameterCollectionInstance(MPC))
		{
			Inst->SetScalarParameterValue(TEXT("PulseEnabled"), 0.0f);
		}
	}
}
void APath::UpdatePulseSpeed(float NewSpeed) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	UMaterialParameterCollection* MPC = PulseMPC.LoadSynchronous();
	if (PulseMPC)
	{
		if (UMaterialParameterCollectionInstance* Inst = GetWorld()->GetParameterCollectionInstance(MPC))
		{
			Inst->SetScalarParameterValue(TEXT("PulseSpeed"), NewSpeed);
		}
	}
}
void APath::UpdatePulseGlow(float NewPulseGlow) const
{
	
	UWorld* World = GetWorld();
	if (!World) return;

	UMaterialParameterCollection* MPC = PulseMPC.LoadSynchronous();
	if (PulseMPC)
	{
		if (UMaterialParameterCollectionInstance* Inst = GetWorld()->GetParameterCollectionInstance(MPC))
		{
			Inst->SetScalarParameterValue(TEXT("PulseIntensity"), NewPulseGlow);
		}
	}
	
}

/**
 * @brief Toggle displays
 * @param bShowPath 
 */
void APath::ChangePathVisibility(bool bShowPath)
{
	if (PathSplineMeshes.Num() > 0)
	{
		for (TObjectPtr<USplineMeshComponent> SplineMeshComponent : PathSplineMeshes)
		{
			SplineMeshComponent->SetHiddenInGame(!bShowPath);
		}
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("No path components have been set"));
	}
}
void APath::ChangeSlopeVisibility(bool bShowPath)
{
	if (PathSplineMeshes.Num() > 0)
	{
		for (TObjectPtr<USplineMeshComponent> SplineMeshComponent : SlopeSplineMeshes)
		{
			SplineMeshComponent->SetHiddenInGame(!bShowPath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No slope components have been set"));
	}
}
void APath::ChangeKmsVisibility(bool bShowKm)
{
	for (auto km : Kms[RaceId])
	{
		km->SetActorHiddenInGame(!bShowKm);
	}
}

/**
 * @brief Updating Glow value
 * @param NewGlow 
 * @param RaceID 
 */
void APath::UpdatePathGlow(float NewGlow, int64 RaceID)
{
	for (TObjectPtr<USplineMeshComponent> SplineMeshComponent : PathSplineMeshes)
	{
		SplineMeshComponent->SetCustomPrimitiveDataFloat(16, NewGlow);
	}
	for (TObjectPtr<USplineMeshComponent> SplineMeshComponent : SlopeSplineMeshes)
	{
		SplineMeshComponent->SetCustomPrimitiveDataFloat(16, NewGlow);
	}
	SettingsSubsystem->SetGlowById(NewGlow, RaceID);
}

void APath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TravelPath) return;

	const float SpeedCmPerSec = SpeedMetersPerSec * 100.f;

	if (bForward)
	{
		DistanceCm += SpeedCmPerSec * DeltaTime;

		const float MaxCm = TravelPath->GetSplineLength();
		DistanceCm = FMath::Clamp(DistanceCm, 0.f, MaxCm);
	} else
	{
		DistanceCm -= SpeedCmPerSec * DeltaTime;
		const float MaxCm = TravelPath->GetSplineLength();
		DistanceCm = FMath::Clamp(DistanceCm, 0.f, MaxCm);
	}

	FollowRail(SpringArmComponent, TravelPath, DistanceCm);
}

// HELPERS
FVector APath::GetClosestSplineLocation(FVector RunnerLocation) const
{
	return SplinePath->GetLocationAtSplineInputKey(SplinePath->FindInputKeyClosestToWorldLocation(RunnerLocation), ESplineCoordinateSpace::World);
	
}
FVector APath::GetLocationAtDistance(float Distance) const
{
	return SplinePath->GetWorldLocationAtDistanceAlongSpline(Distance);
}

float APath::GetDistanceAlongSpline(FVector RunnerLocation) const
{
	return SplinePath->GetDistanceAlongSplineAtLocation(RunnerLocation, ESplineCoordinateSpace::World);
}

void APath::SetRacePath(const FRacePath& NewRacePath)
{
	RacePath = NewRacePath;
}
void APath::HandleCheckpointsDatasGathered(int64 RaceID, FCheckpoints CheckpointsDatas)
{
	NbCheckpoints = CheckpointsDatas.Checkpoints.Num(); 
}

void APath::SetBroadcastCaptureEnabled_Implementation(bool bEnabled, class UTextureRenderTarget2D* SharedRT)
{
	if (bEnabled)
	{
		OwlCapture->SetComponentTickEnabled(true);
		OwlCapture->Activate(true);
		OwlCapture->bPauseRendering = false;
		OwlCapture->TextureTarget = SharedRT;
	} else
	{
		OwlCapture->bPauseRendering = true;            // pause
		OwlCapture->SetComponentTickEnabled(false);    // stop tick
		OwlCapture->Deactivate();

		OwlCapture->TextureTarget = nullptr;
	}
}
