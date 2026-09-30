// All Rights Reserved

#include "TrailSimulator/Public/Actors/Path.h"
#include "BroadcastCaptureSubsystem.h"
#include "CesiumFlyToComponent.h"
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
	
	if (Georeference)
	{
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

		SplinePath->ClearSplinePoints(false);
		SlopePath->ClearSplinePoints(false);
		TravelPath->ClearSplinePoints(false);
		int32 cpt = 0;
		const int32 Total = RacePath.Points.Num();
		for (FRacePathPoint Point  : RacePath.Points)
		{
			FVector UE = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Point.lon, Point.lat, Point.ele));
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
			int32 ChkIndex = 0;
			if (!RacePathDatas.Points[cpt].datas.name.IsEmpty())
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
				SpawnedActor = CheckpointSubsystem->SpawnCheckpointActor(RaceID, RacePathDatas.Points[cpt].datas.checkpointId, World, CheckpointClass, SpawnTransform);
				
				if (ICheckpointInterface* CheckpointInterface = Cast<ICheckpointInterface>(SpawnedActor))
				{
					CheckpointInterface->UpdateCheckpoint(RacePathDatas.Points[cpt].datas, RaceSetup);
					SpawnedActor->SetActorHiddenInGame(true);
					SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
					LoadingSubsystem->Update(IdCheckpoints, 1.f, FText::FromString(FString::Printf(TEXT("Checkpoint %s spawned"), *RacePathDatas.Points[cpt].datas.name)));
				}
				ChkIndex++;
			}
			cpt++;
		} 
		LoadingSubsystem->Complete(IdCheckpoints, FText::FromString(FString::Printf(TEXT("Checkpoints spawned for race %lld"), RaceID)));
	}
	
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
	Root->SetMobility(EComponentMobility::Static);
	for (const FPointIndexSegment& Seg : OutSegments)
	{
		const int32 I0 = Seg.Indices[0];
		const int32 I1 = Seg.Indices.Last();
		
		float Slope = RacePath.Points[I1].ele - RacePath.Points[I0].ele;
		
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
			
			SMC->SetCustomPrimitiveDataFloat(10, Slope);	// SlopeMeters (signé)
			SMC->SetCustomPrimitiveDataFloat(11, 1.f);	// UseSlope = 1
			SMC->SetCustomPrimitiveDataFloat(16, SettingsSubsystem->GetGlowById(RaceID));
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
