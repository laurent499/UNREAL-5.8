// Copyright LTV Prod 2026. All Rights Reserved

#include "WorldFauna.h"
#include "WorldAmbienceSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"

namespace WorldFauna
{
	const TCHAR* MaterialPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");

	constexpr float RespawnDistance = 80000.f;            // 800 m : les oiseaux ne restent pas loin derriere
	constexpr float AnchorDistance = 8000.f;              // 80 m devant la camera

	/** Hauteur maximale des oiseaux au-dessus du sol (cm), reglee par la regie (0 a 4 000 m) */
	float MaxBirdHeight(const UWorldAmbienceSubsystem* Ambience)
	{
		return Ambience ? FMath::Max(Ambience->GetSettings().FaunaMaxHeightM * 100.f, 1500.f) : 150000.f;
	}

	/** Mesh statique construit au lancement a partir de triangles (chaque face doublee : visible des deux cotes) */
	UStaticMesh* BuildMesh(UObject* Outer, const TCHAR* Name, const TArray<FVector3f>& Points, const TArray<int32>& Triangles, UMaterialInterface* Material)
	{
		FMeshDescription Desc;
		FStaticMeshAttributes Attributes(Desc);
		Attributes.Register();

		const FPolygonGroupID Group = Desc.CreatePolygonGroup();
		Attributes.GetPolygonGroupMaterialSlotNames()[Group] = TEXT("Bird");

		TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();
		TVertexInstanceAttributesRef<FVector3f> Normals = Attributes.GetVertexInstanceNormals();
		TVertexInstanceAttributesRef<FVector2f> UVs = Attributes.GetVertexInstanceUVs();
		UVs.SetNumChannels(1);

		TArray<FVertexID> Vertices;
		for (const FVector3f& P : Points)
		{
			const FVertexID V = Desc.CreateVertex();
			Positions[V] = P;
			Vertices.Add(V);
		}

		auto AddTriangle = [&](int32 A, int32 B, int32 C)
		{
			const FVector3f N = FVector3f::CrossProduct(Points[C] - Points[A], Points[B] - Points[A]).GetSafeNormal();
			TArray<FVertexInstanceID, TFixedAllocator<3>> Corners;
			for (const int32 Index : { A, B, C })
			{
				const FVertexInstanceID I = Desc.CreateVertexInstance(Vertices[Index]);
				Normals[I] = N;
				UVs.Set(I, 0, FVector2f(Points[Index].X / 100.f, Points[Index].Y / 100.f));
				Corners.Add(I);
			}
			Desc.CreateTriangle(Group, Corners);
		};
		for (int32 i = 0; i + 2 < Triangles.Num(); i += 3)
		{
			AddTriangle(Triangles[i], Triangles[i + 1], Triangles[i + 2]);
			AddTriangle(Triangles[i], Triangles[i + 2], Triangles[i + 1]);
		}

		UStaticMesh* Mesh = NewObject<UStaticMesh>(Outer, Name);
		Mesh->GetStaticMaterials().Add(FStaticMaterial(Material, TEXT("Bird")));
		UStaticMesh::FBuildMeshDescriptionsParams Params;
		Params.bFastBuild = true;
		Params.bBuildSimpleCollision = false;
		Mesh->BuildFromMeshDescriptions({ &Desc }, Params);
		return Mesh;
	}
}

AWorldFauna::AWorldFauna()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork; // apres la camera

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	for (TObjectPtr<UInstancedStaticMeshComponent>* Component : { &Bodies, &Wings })
	{
		*Component = CreateDefaultSubobject<UInstancedStaticMeshComponent>(Component == &Bodies ? TEXT("Bodies") : TEXT("Wings"));
		(*Component)->SetupAttachment(Root);
		(*Component)->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		(*Component)->SetCastShadow(false);
		(*Component)->SetMobility(EComponentMobility::Movable);
		(*Component)->SetVisibility(false);
	}
}

void AWorldFauna::BeginPlay()
{
	Super::BeginPlay();
	Ambience = GetWorld()->GetSubsystem<UWorldAmbienceSubsystem>();
	BuildMeshes();
}

void AWorldFauna::BuildMeshes()
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, WorldFauna::MaterialPath);
	BirdMaterial = UMaterialInstanceDynamic::Create(Base, this);
	if (BirdMaterial)
	{
		BirdMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.045f, 0.038f, 0.03f));
	}

	// Corps de rapace (cm) : fuseau de 90 cm et queue en eventail
	const TArray<FVector3f> BodyPoints = {
		{ 40, 0, 0 }, { 0, 7, 0 }, { 0, -7, 0 }, { 0, 0, 6 }, { 0, 0, -6 }, { -45, 0, 0 },
		{ -40, 0, 0 }, { -62, 16, 0 }, { -62, -16, 0 } };
	const TArray<int32> BodyTris = {
		0, 1, 3, 0, 3, 2, 0, 4, 1, 0, 2, 4,
		5, 3, 1, 5, 2, 3, 5, 1, 4, 5, 4, 2,
		6, 7, 8 };
	// Aile droite (le long de +Y), envergure totale 2,2 m ; l'aile gauche est la meme en miroir
	const TArray<FVector3f> WingPoints = {
		{ 12, 0, 0 }, { -20, 0, 0 }, { -26, 60, 0 }, { -16, 110, 0 }, { 2, 104, 0 }, { 10, 58, 0 } };
	const TArray<int32> WingTris = { 0, 1, 2, 0, 2, 5, 5, 2, 3, 5, 3, 4 };

	UMaterialInterface* Material = BirdMaterial ? static_cast<UMaterialInterface*>(BirdMaterial) : Base;
	BodyMesh = WorldFauna::BuildMesh(this, TEXT("BirdBody"), BodyPoints, BodyTris, Material);
	WingMesh = WorldFauna::BuildMesh(this, TEXT("BirdWing"), WingPoints, WingTris, Material);
	Bodies->SetStaticMesh(BodyMesh);
	Wings->SetStaticMesh(WingMesh);
}

float AWorldFauna::GroundZAt(const FVector& Location) const
{
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WorldFaunaGround), false, this);
	// La camera du joueur a sa propre collision : sans cela le rayon s'arrete sur elle
	if (const APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) Params.AddIgnoredActor(Pawn);
	const FVector Start(Location.X, Location.Y, Location.Z + 500000.0);
	const FVector End(Location.X, Location.Y, Location.Z - 2000000.0);
	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) ? static_cast<float>(Hit.ImpactPoint.Z) : TNumericLimits<float>::Lowest();
}

bool AWorldFauna::IsFaunaAllowed()
{
	const UWorldAmbienceSubsystem* World = Ambience.Get();
	if (!World) { Status = TEXT("Sous-systeme absent"); return false; }
	if (World->GetEffectiveFaunaDensity() <= 0.f) { Status = TEXT("Faune coupee (reglage ou gardien)"); return false; }
	if (!BodyMesh || !WingMesh) { Status = TEXT("Meshes non construits"); return false; }
	const FWorldAmbienceState& S = World->GetState();
	// Pas d'oiseaux la nuit, sous la pluie ou la neige forte, ni par grand vent
	if (S.Night >= 0.5f) { Status = TEXT("Nuit"); return false; }
	if (S.Rain >= 5.f || S.Snow >= 5.f || S.Wind >= 8.f) { Status = TEXT("Meteo trop forte"); return false; }
	if (!bHasAnchor) { Status = TEXT("Sol introuvable sous la camera"); return false; }
	// Oiseaux confines entre le sol et FaunaMaxHeightM ; au-dela de 3 km au-dessus de cette tranche ils ne seraient plus visibles
	if (CameraHeightAboveGround >= WorldFauna::MaxBirdHeight(World) + 300000.f)
	{
		Status = FString::Printf(TEXT("Camera trop haute (%.0f m du sol)"), CameraHeightAboveGround / 100.f);
		return false;
	}
	Status = FString::Printf(TEXT("En vol (camera a %.0f m du sol)"), CameraHeightAboveGround / 100.f);
	return true;
}

bool AWorldFauna::UpdateAnchor(float DeltaSeconds)
{
	// Le sol est sonde une fois par seconde : point a 80 m devant la camera, a l'horizontale
	AnchorTimer -= DeltaSeconds;
	if (AnchorTimer > 0.f) return false;
	AnchorTimer = 1.f;

	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!Camera) return false;
	const FVector CamPos = Camera->GetCameraLocation();
	FVector Forward = Camera->GetCameraRotation().Vector();
	Forward.Z = 0.0;
	Forward = Forward.GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector);

	// Plus la camera est haute, plus les oiseaux sont places loin devant et haut, entre le sol et elle
	const FVector Candidate = CamPos + Forward * FMath::Max(WorldFauna::AnchorDistance, CameraHeightAboveGround * 0.8f);
	const float GroundZ = GroundZAt(Candidate);
	if (GroundZ == TNumericLimits<float>::Lowest()) return false;
	const float CamGroundZ = GroundZAt(CamPos);
	CameraHeightAboveGround = static_cast<float>(CamPos.Z) - (CamGroundZ == TNumericLimits<float>::Lowest() ? GroundZ : CamGroundZ);

	const bool bMoved = !bHasAnchor || FVector::Dist2D(Candidate, Anchor) > WorldFauna::RespawnDistance;
	if (bMoved)
	{
		Anchor = FVector(Candidate.X, Candidate.Y, GroundZ);
		AnchorGroundZ = GroundZ;
		bHasAnchor = true;
	}
	return bMoved;
}

void AWorldFauna::Respawn(const FVector& Center, float GroundZ)
{
	const float Density = Ambience.IsValid() ? Ambience->GetEffectiveFaunaDensity() : 0.f;
	AppliedDensity = Density;
	// Hauteur de camera au moment du placement : les orbites s'elargissent avec elle
	SpawnCameraHeight = CameraHeightAboveGround;
	// Les oiseaux volent pres de la hauteur de la camera, sans depasser la tranche reglee
	const float Top = WorldFauna::MaxBirdHeight(Ambience.Get());
	const float RefHeight = FMath::Min(SpawnCameraHeight, Top);
	const float Spread = FMath::Max(1.f, SpawnCameraHeight / 10000.f);
	// Camera haute : oiseaux grossis pour rester lisibles a plusieurs centaines de metres
	const float Readable = FMath::Clamp(SpawnCameraHeight / 30000.f, 1.f, 3.f);
	Birds.Reset();

	const int32 Raptors = FMath::Clamp(FMath::RoundToInt(2.f * Density), 0, 6);
	for (int32 i = 0; i < Raptors; ++i)
	{
		FBird B;
		B.Type = EBirdType::Raptor;
		const float A = FMath::FRandRange(0.f, UE_TWO_PI);
		const float Dist = FMath::FRandRange(2000.f, 6000.f) * Spread;
		B.OrbitCenter = Center + FVector(FMath::Cos(A) * Dist, FMath::Sin(A) * Dist, 0.f);
		B.OrbitCenter.Z = GroundZ;
		B.OrbitRadius = FMath::FRandRange(2500.f, 5000.f) * Spread;
		B.OrbitSpeed = (FMath::RandBool() ? 1.f : -1.f) * 1000.f / B.OrbitRadius; // ~10 m/s
		B.OrbitAngle = FMath::FRandRange(0.f, UE_TWO_PI);
		B.HeightAboveGround = FMath::Min(FMath::FRandRange(FMath::Max(2500.f, RefHeight * 0.4f), FMath::Max(7000.f, RefHeight * 1.0f)), Top - 800.f);
		B.Scale = FMath::FRandRange(1.2f, 1.6f) * Readable; // envergure 2,6 a 3,5 m : gypaete, aigle royal
		B.FlapSpeed = FMath::FRandRange(5.f, 7.f);
		B.FlapTimer = FMath::FRandRange(2.f, 15.f);
		B.Position = B.OrbitCenter + FVector(FMath::Cos(B.OrbitAngle) * B.OrbitRadius, FMath::Sin(B.OrbitAngle) * B.OrbitRadius, B.HeightAboveGround);
		Birds.Add(B);
	}

	const int32 Flock = FMath::Clamp(FMath::RoundToInt(10.f * Density), 0, 30);
	FlockLeader = Center + FVector(FMath::FRandRange(-8000.f, 8000.f), FMath::FRandRange(-8000.f, 8000.f), 0.f);
	FlockHeading = FMath::FRandRange(0.f, UE_TWO_PI);
	FlockHeight = FMath::Min(FMath::FRandRange(FMath::Max(1500.f, RefHeight * 0.2f), FMath::Max(3500.f, RefHeight * 0.7f)), Top - 300.f);
	FlockLeader.Z = GroundZ + FlockHeight;
	for (int32 i = 0; i < Flock; ++i)
	{
		FBird B;
		B.Type = EBirdType::Flock;
		B.FormationOffset = FVector(FMath::FRandRange(-600.f, 600.f), FMath::FRandRange(-500.f, 500.f), FMath::FRandRange(-200.f, 200.f));
		B.Scale = FMath::FRandRange(0.13f, 0.18f) * Readable;
		B.FlapSpeed = FMath::FRandRange(45.f, 60.f);
		B.FlapPhase = FMath::FRandRange(0.f, UE_TWO_PI);
		B.FlapAmount = 1.f;
		Birds.Add(B);
	}

	Bodies->ClearInstances();
	Wings->ClearInstances();
	TArray<FTransform> Identity;
	Identity.Init(FTransform::Identity, Birds.Num());
	Bodies->AddInstances(Identity, false);
	Identity.Init(FTransform::Identity, Birds.Num() * 2);
	Wings->AddInstances(Identity, false);
}

void AWorldFauna::Simulate(float DeltaSeconds)
{
	FlockTime += DeltaSeconds;

	// Chef du vol groupe : cap qui ondule, ramene vers le point d'ancrage s'il s'eloigne de plus de 120 m
	const FVector ToAnchor = Anchor - FlockLeader;
	float Turn = FMath::Sin(FlockTime * 0.13f) * 0.25f;
	if (ToAnchor.Size2D() > 12000.f * FMath::Max(1.f, SpawnCameraHeight / 10000.f))
	{
		const float Wanted = FMath::Atan2(ToAnchor.Y, ToAnchor.X);
		Turn += FMath::FindDeltaAngleRadians(FlockHeading, Wanted) * 0.6f;
	}
	FlockHeading += Turn * DeltaSeconds;
	const FVector LeaderDir(FMath::Cos(FlockHeading), FMath::Sin(FlockHeading), 0.f);
	FlockLeader += LeaderDir * 1200.f * DeltaSeconds;
	FlockLeader.Z = FMath::FInterpTo(FlockLeader.Z, AnchorGroundZ + FlockHeight, DeltaSeconds, 0.3f);

	int32 Index = 0;
	for (FBird& B : Birds)
	{
		if (B.Type == EBirdType::Raptor)
		{
			B.OrbitAngle += B.OrbitSpeed * DeltaSeconds;
			const float Lift = FMath::Sin(FlockTime * 0.07f + Index) * 800.f; // monte et descend dans l'ascendance
			const FVector Offset(FMath::Cos(B.OrbitAngle) * B.OrbitRadius, FMath::Sin(B.OrbitAngle) * B.OrbitRadius, B.HeightAboveGround + Lift);
			const FVector NewPos = B.OrbitCenter + Offset;
			B.Velocity = (NewPos - B.Position) / FMath::Max(DeltaSeconds, 1e-3f);
			B.Position = NewPos;

			// Quelques battements de temps en temps, sinon vol plane
			B.FlapTimer -= DeltaSeconds;
			if (B.FlapTimer < -2.5f) B.FlapTimer = FMath::FRandRange(8.f, 20.f);
			B.FlapAmount = FMath::FInterpTo(B.FlapAmount, B.FlapTimer < 0.f ? 1.f : 0.f, DeltaSeconds, 3.f);
		}
		else
		{
			const FVector Local = FRotator(0.f, FMath::RadiansToDegrees(FlockHeading), 0.f).RotateVector(B.FormationOffset);
			const FVector Wobble(FMath::Sin(FlockTime * 0.9f + Index) * 150.f, FMath::Cos(FlockTime * 0.7f + Index * 1.3f) * 150.f, FMath::Sin(FlockTime * 1.1f + Index * 0.7f) * 80.f);
			B.Position = FlockLeader + Local + Wobble;
			B.Velocity = LeaderDir * 1200.f;
			// Vol ondule des passereaux : battements puis courtes glissades
			B.FlapAmount = FMath::FInterpTo(B.FlapAmount, FMath::Sin(FlockTime * 1.7f + Index) > -0.4f ? 1.f : 0.f, DeltaSeconds, 6.f);
		}
		B.FlapPhase += B.FlapSpeed * DeltaSeconds;
		++Index;
	}
}

void AWorldFauna::PushInstances()
{
	TArray<FTransform> BodyTransforms;
	TArray<FTransform> WingTransforms;
	BodyTransforms.Reserve(Birds.Num());
	WingTransforms.Reserve(Birds.Num() * 2);

	const float FlockTurn = FMath::Sin(FlockTime * 0.13f) * 0.25f;
	for (const FBird& B : Birds)
	{
		FRotator Rot = B.Velocity.IsNearlyZero() ? FRotator::ZeroRotator : B.Velocity.Rotation();
		// Inclinaison dans les virages : vers l'interieur de l'orbite pour les rapaces
		Rot.Roll = B.Type == EBirdType::Raptor ? (B.OrbitSpeed > 0.f ? 22.f : -22.f) : FMath::Clamp(FlockTurn * 60.f, -30.f, 30.f);
		const FTransform BodyXf(Rot, B.Position, FVector(B.Scale));
		BodyTransforms.Add(BodyXf);

		// Battement : rotation des ailes autour de l'axe du corps, diedre releve en vol plane
		const float FlapDeg = FMath::Sin(B.FlapPhase) * B.FlapAmount * 40.f + (1.f - B.FlapAmount) * 8.f;
		const FTransform Right(FRotator(0.f, 0.f, FlapDeg), FVector::ZeroVector, FVector::OneVector);
		const FTransform Left(FRotator(0.f, 0.f, -FlapDeg), FVector::ZeroVector, FVector(1.f, -1.f, 1.f));
		WingTransforms.Add(Right * BodyXf);
		WingTransforms.Add(Left * BodyXf);
	}

	Bodies->BatchUpdateInstancesTransforms(0, BodyTransforms, true, true, true);
	Wings->BatchUpdateInstancesTransforms(0, WingTransforms, true, true, true);
}

void AWorldFauna::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Ambience.IsValid() || !BodyMesh) return;

	const bool bAnchorMoved = UpdateAnchor(DeltaSeconds);
	const bool bAllowed = IsFaunaAllowed();
	bVisible = bAllowed;
	Bodies->SetVisibility(bAllowed);
	Wings->SetVisibility(bAllowed);
	if (!bAllowed) return;

	const float Density = Ambience->GetEffectiveFaunaDensity();
	// La camera a beaucoup monte ou descendu depuis le placement : on replace les oiseaux a sa hauteur
	const float HeightRatio = (CameraHeightAboveGround + 5000.f) / (SpawnCameraHeight + 5000.f);
	const bool bHeightChanged = HeightRatio > 1.6f || HeightRatio < 0.6f;
	if (bAnchorMoved || bHeightChanged || !FMath::IsNearlyEqual(Density, AppliedDensity, 0.05f) || Birds.Num() == 0)
	{
		Respawn(Anchor, AnchorGroundZ);
		if (Birds.Num() == 0) return;
	}

	Simulate(DeltaSeconds);
	PushInstances();
}
