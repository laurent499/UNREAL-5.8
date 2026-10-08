// Copyright LTV Prod 2026. All Rights Reserved

#include "WorldFlora.h"
#include "WorldAmbienceSubsystem.h"
#include "WaterMaskSubsystem.h"
#include "PathSubsystem.h"
#include "Engine/GameInstance.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/PlayerCameraManager.h"
#include "Cesium3DTileset.h"
#include "CesiumGeoreference.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

namespace WorldFlora
{
	const TCHAR* Folders[] = { TEXT("/Game/LTVContent/Flora/Grass"), TEXT("/Game/LTVContent/Flora/Shrubs"), TEXT("/Game/LTVContent/Flora/Rocks") };
	constexpr float CellSize = 1000.f;                    // 10 m
	constexpr float MaxCameraHeightAboveGround = 8000.f;  // 80 m
	constexpr int32 TracesPerFrame = 300;                 // budget de lancers de rayon par image
	// Nombre de tentatives par cellule de 10 m a densite 1 (touffes d'herbe, buissons, rochers)
	constexpr float SamplesPerCell[] = { 40.f, 1.2f, 0.35f };
	constexpr float TreelineM = 2300.f;

	uint32 Hash(int32 X, int32 Y, int32 Salt)
	{
		uint32 H = static_cast<uint32>(X) * 73856093u ^ static_cast<uint32>(Y) * 19349663u ^ static_cast<uint32>(Salt) * 83492791u;
		H ^= H >> 13; H *= 0x5bd1e995u; H ^= H >> 15;
		return H;
	}
}

AWorldFlora::AWorldFlora()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AWorldFlora::BeginPlay()
{
	Super::BeginPlay();
	Ambience = GetWorld()->GetSubsystem<UWorldAmbienceSubsystem>();
	LoadMeshes();

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UPathSubsystem* Paths = GI->GetSubsystem<UPathSubsystem>())
		{
			Paths->OnPathDatasGathered.AddUniqueDynamic(this, &AWorldFlora::HandlePathGathered);
			HandlePathGathered(-1, Paths->GetRacePath()); // trace deja charge avant nous
		}
	}
}

void AWorldFlora::HandlePathGathered(int64 RaceID, FRacePath RacePath)
{
	TArray<FVector2D>& Points = RacePathsLonLat.FindOrAdd(RaceID);
	Points.Reset(RacePath.Points.Num());
	for (const FRacePathPoint& P : RacePath.Points) Points.Emplace(P.lon, P.lat);
	BuiltDensity = -1.f; // force une reconstruction avec le nouveau trace
}

bool AWorldFlora::IsOnTrail(const FVector& Location) const
{
	// Couloir de 3 m de part et d'autre du trace : le chemin reste degage
	constexpr double HalfWidth = 300.0;
	const FVector2D P(Location);
	for (const TPair<FVector2D, FVector2D>& S : TrailSegments)
	{
		const FVector2D AB = S.Value - S.Key;
		const double T = FMath::Clamp(FVector2D::DotProduct(P - S.Key, AB) / FMath::Max(AB.SizeSquared(), 1.0), 0.0, 1.0);
		if (FVector2D::DistSquared(P, S.Key + AB * T) < HalfWidth * HalfWidth) return true;
	}
	return false;
}

void AWorldFlora::LoadMeshes()
{
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	// Liste explicite dans RemoteControl/Flore.json (cles grass, shrubs, rocks), sinon les dossiers Flora
	TSharedPtr<FJsonObject> Config;
	FString Json;
	const FString ConfigFile = FPaths::Combine(FPaths::ProjectDir(), TEXT("RemoteControl"), TEXT("Flore.json"));
	if (FFileHelper::LoadFileToString(Json, *ConfigFile)) FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Config);
	const TCHAR* ConfigKeys[] = { TEXT("grass"), TEXT("shrubs"), TEXT("rocks") };

	for (int32 Kind = 0; Kind < static_cast<int32>(EKind::Count); ++Kind)
	{
		TArray<UStaticMesh*> Meshes;
		const TArray<TSharedPtr<FJsonValue>>* Paths = nullptr;
		if (Config.IsValid() && Config->TryGetArrayField(ConfigKeys[Kind], Paths))
		{
			for (const TSharedPtr<FJsonValue>& Path : *Paths)
			{
				if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path->AsString())) Meshes.Add(Mesh);
				else UE_LOG(LogTemp, Warning, TEXT("[Flore] Mesh introuvable : %s"), *Path->AsString());
			}
		}
		else
		{
			TArray<FAssetData> Assets;
			Registry.GetAssetsByPath(FName(WorldFlora::Folders[Kind]), Assets, /*bRecursive=*/true);
			for (const FAssetData& Asset : Assets)
			{
				if (Asset.AssetClassPath != UStaticMesh::StaticClass()->GetClassPathName()) continue;
				if (UStaticMesh* Mesh = Cast<UStaticMesh>(Asset.GetAsset())) Meshes.Add(Mesh);
			}
		}

		for (UStaticMesh* Mesh : Meshes)
		{
			UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
			HISM->SetStaticMesh(Mesh);
			HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HISM->SetMobility(EComponentMobility::Movable);
			// L'herbe ne projette pas d'ombre (cout VSM) ; buissons et rochers oui, ils ancrent l'image
			HISM->SetCastShadow(Kind != static_cast<int32>(EKind::Grass));
			HISM->bAffectDistanceFieldLighting = false;
			HISM->SetupAttachment(GetRootComponent());
			HISM->RegisterComponent();
			HISM->SetVisibility(false);
			Kinds[Kind].Components.Add(HISM);
			AllComponents.Add(HISM);
		}
	}
	Status = AllComponents.Num() == 0
		? TEXT("Aucun mesh (RemoteControl/Flore.json ou /Game/LTVContent/Flora)")
		: FString::Printf(TEXT("%d meshes charges"), AllComponents.Num());
}

bool AWorldFlora::TraceGround(const FVector& XY, FHitResult& OutHit) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WorldFloraGround), false, this);
	const FVector Start(XY.X, XY.Y, XY.Z + 30000.0);
	const FVector End(XY.X, XY.Y, XY.Z - 60000.0);
	if (!GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params)) return false;
	// Uniquement sur le terrain Cesium (pas sur les coureurs, panneaux ou trace)
	return OutHit.GetActor() && OutHit.GetActor()->IsA<ACesium3DTileset>();
}

void AWorldFlora::HideAll()
{
	if (!bShown) return;
	for (UHierarchicalInstancedStaticMeshComponent* HISM : AllComponents) HISM->SetVisibility(false);
	bShown = false;
	ShownInstances = 0;
}

void AWorldFlora::StartBuild(const FVector& Center, float RadiusCm, float Density)
{
	bBuilding = true;
	BuildCenter = Center;
	BuildRadius = RadiusCm;
	BuildDensity = Density;
	PendingCells.Reset();
	PendingInstances.Reset();

	const int32 CX = FMath::FloorToInt(Center.X / WorldFlora::CellSize);
	const int32 CY = FMath::FloorToInt(Center.Y / WorldFlora::CellSize);
	const int32 R = FMath::CeilToInt(RadiusCm / WorldFlora::CellSize);
	for (int32 Y = -R; Y <= R; ++Y)
	{
		for (int32 X = -R; X <= R; ++X)
		{
			if (X * X + Y * Y <= R * R) PendingCells.Add({ CX + X, CY + Y });
		}
	}
	// Cellules proches d'abord : le premier plan se remplit avant le fond
	PendingCells.Sort([CX, CY](const FPendingCell& A, const FPendingCell& B)
	{
		return FMath::Square(A.X - CX) + FMath::Square(A.Y - CY) < FMath::Square(B.X - CX) + FMath::Square(B.Y - CY);
	});

	Georeference = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(GetWorld(), ACesiumGeoreference::StaticClass()));
	UGameInstance* GI = GetWorld()->GetGameInstance();
	WaterMask = GI ? GI->GetSubsystem<UWaterMaskSubsystem>() : nullptr;

	// Segments des traces de course qui passent dans la zone (avec une marge)
	TrailSegments.Reset();
	if (const ACesiumGeoreference* Geo = Georeference.Get())
	{
		const double Reach = FMath::Square(RadiusCm + 5000.0);
		for (const TPair<int64, TArray<FVector2D>>& Pair : RacePathsLonLat)
		{
			FVector2D Prev;
			bool bHasPrev = false;
			for (const FVector2D& LonLat : Pair.Value)
			{
				const FVector World = Geo->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(LonLat.X, LonLat.Y, 0.0));
				const FVector2D Cur(World);
				if (bHasPrev && (FVector2D::DistSquared(Cur, FVector2D(Center)) < Reach || FVector2D::DistSquared(Prev, FVector2D(Center)) < Reach))
				{
					TrailSegments.Emplace(Prev, Cur);
				}
				Prev = Cur;
				bHasPrev = true;
			}
		}
	}
}

void AWorldFlora::ContinueBuild()
{
	int32 Budget = WorldFlora::TracesPerFrame;
	while (Budget > 0 && PendingCells.Num() > 0)
	{
		const FPendingCell Cell = PendingCells.Pop(EAllowShrinking::No); // la liste est triee : on prend par la fin
		for (int32 Kind = 0; Kind < static_cast<int32>(EKind::Count); ++Kind)
		{
			const TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>>& Components = Kinds[Kind].Components;
			if (Components.Num() == 0) continue;

			// Nombre de tentatives deterministe pour cette cellule : la flore ne "saute" pas d'une reconstruction a l'autre
			const float Expected = WorldFlora::SamplesPerCell[Kind] * BuildDensity;
			FRandomStream Rand(static_cast<int32>(WorldFlora::Hash(Cell.X, Cell.Y, Kind)));
			int32 Count = FMath::FloorToInt(Expected);
			if (Rand.FRand() < Expected - Count) ++Count;

			// Herbe : beaucoup de petites touffes, posees sur le plan du sol sonde une seule fois au centre de la cellule
			const bool bGrass = static_cast<EKind>(Kind) == EKind::Grass;
			FHitResult CellHit;
			if (bGrass && Count > 0)
			{
				const FVector2D CellCenter2D((Cell.X + 0.5f) * WorldFlora::CellSize, (Cell.Y + 0.5f) * WorldFlora::CellSize);
				if (FVector2D::Distance(CellCenter2D, FVector2D(BuildCenter)) > 6000.f) continue;
				--Budget;
				const FVector CellCenter((Cell.X + 0.5f) * WorldFlora::CellSize, (Cell.Y + 0.5f) * WorldFlora::CellSize, BuildCenter.Z);
				if (!TraceGround(CellCenter, CellHit) || CellHit.ImpactNormal.Z < 0.5) continue;
			}

			for (int32 i = 0; i < Count; ++i)
			{
				const FVector XY((Cell.X + Rand.FRand()) * WorldFlora::CellSize, (Cell.Y + Rand.FRand()) * WorldFlora::CellSize, BuildCenter.Z);
				const float Yaw = Rand.FRandRange(0.f, 360.f);
				const float Scale = bGrass ? Rand.FRandRange(0.8f, 1.7f) : Rand.FRandRange(0.75f, 1.3f);
				const int32 MeshIndex = Rand.RandRange(0, Components.Num() - 1);
				const float Keep = Rand.FRand();

				FHitResult Hit;
				if (bGrass)
				{
					// Hauteur sur le plan tangent : z = z0 - (n.x * dx + n.y * dy) / n.z
					Hit = CellHit;
					const FVector N = CellHit.ImpactNormal;
					const double DZ = -(N.X * (XY.X - CellHit.ImpactPoint.X) + N.Y * (XY.Y - CellHit.ImpactPoint.Y)) / N.Z;
					Hit.ImpactPoint = FVector(XY.X, XY.Y, CellHit.ImpactPoint.Z + DZ);
				}
				else
				{
					--Budget;
					if (!TraceGround(XY, Hit)) continue;
				}
				if (IsOnTrail(Hit.ImpactPoint)) continue;
				const float Slope = static_cast<float>(Hit.ImpactNormal.Z); // 1 = plat
				float AltitudeM = static_cast<float>(Hit.ImpactPoint.Z) / 100.f;
				if (const ACesiumGeoreference* Geo = Georeference.Get())
				{
					const FVector LLH = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(Hit.ImpactPoint);
					AltitudeM = static_cast<float>(LLH.Z);
					// Rien sur les lacs et rivieres connus d'OpenStreetMap
					if (UWaterMaskSubsystem* Water = WaterMask.Get(); Water && Water->IsWaterAt(LLH.X, LLH.Y)) continue;
				}

				// Regles de milieu : herbe sur les pentes douces, buissons sous la limite des arbres, rochers dans la pente
				bool bPlace = false;
				switch (static_cast<EKind>(Kind))
				{
				case EKind::Grass: bPlace = Slope > 0.85f && Keep < FMath::GetMappedRangeValueClamped(FVector2f(2400.f, 3000.f), FVector2f(1.f, 0.2f), AltitudeM); break;
				case EKind::Shrub: bPlace = Slope > 0.8f && Keep < (AltitudeM < WorldFlora::TreelineM ? 1.f : 0.3f); break;
				case EKind::Rock:  bPlace = Slope < 0.95f || Keep < 0.3f; break;
				default: break;
				}
				if (!bPlace) continue;

				// Rochers inclines avec le terrain, vegetation verticale
				FQuat Rot = FQuat(FRotator(0.f, Yaw, 0.f));
				if (static_cast<EKind>(Kind) == EKind::Rock)
				{
					Rot = FQuat::FindBetweenNormals(FVector::UpVector, Hit.ImpactNormal) * Rot;
				}
				const FVector Location = Hit.ImpactPoint - FVector(0.0, 0.0, static_cast<EKind>(Kind) == EKind::Rock ? 20.0 : 2.0);
				PendingInstances.FindOrAdd(Components[MeshIndex]).Add(FTransform(Rot, Location, FVector(Scale)));
			}
		}
	}

	if (PendingCells.Num() == 0) ApplyBuild();
}

void AWorldFlora::ApplyBuild()
{
	bBuilding = false;
	ShownInstances = 0;
	for (UHierarchicalInstancedStaticMeshComponent* HISM : AllComponents)
	{
		HISM->ClearInstances();
		if (const TArray<FTransform>* Instances = PendingInstances.Find(HISM))
		{
			HISM->AddInstances(*Instances, false, /*bWorldSpace=*/true);
			ShownInstances += Instances->Num();
		}
		// L'herbe (petites touffes) disparait plus tot : au-dela de 60 m elle ne fait que couter
		const bool bGrass = Kinds[static_cast<int32>(EKind::Grass)].Components.Contains(HISM);
		HISM->SetCullDistances(0, static_cast<int32>(bGrass ? FMath::Min(BuildRadius, 6000.f) : BuildRadius));
		HISM->SetVisibility(true);
	}
	bShown = true;
	PendingInstances.Reset();
	BuiltCenter = BuildCenter;
	BuiltRadius = BuildRadius;
	BuiltDensity = BuildDensity;
	Status = FString::Printf(TEXT("%d elements sur %.0f m autour de la camera"), ShownInstances, BuildRadius / 100.f);
}

void AWorldFlora::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const UWorldAmbienceSubsystem* World = Ambience.Get();
	if (!World || AllComponents.Num() == 0) return;

	if (bBuilding)
	{
		ContinueBuild();
		return;
	}

	CheckTimer -= DeltaSeconds;
	if (CheckTimer > 0.f) return;
	CheckTimer = 0.5f;

	const float Density = World->GetEffectiveFloraDensity();
	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (Density <= 0.f || !Camera)
	{
		Status = TEXT("Flore coupee (reglage ou gardien)");
		HideAll();
		return;
	}

	const FVector CamPos = Camera->GetCameraLocation();
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WorldFloraCamera), false, this);
	if (const APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) Params.AddIgnoredActor(Pawn);
	const bool bGround = GetWorld()->LineTraceSingleByChannel(Hit, CamPos, CamPos - FVector(0.0, 0.0, 2000000.0), ECC_Visibility, Params);
	if (!bGround || CamPos.Z - Hit.ImpactPoint.Z > WorldFlora::MaxCameraHeightAboveGround)
	{
		Status = bGround ? FString::Printf(TEXT("Camera trop haute (%.0f m du sol)"), (CamPos.Z - Hit.ImpactPoint.Z) / 100.f) : TEXT("Sol introuvable sous la camera");
		HideAll();
		return;
	}

	// Reconstruction quand la camera a parcouru un tiers du rayon, ou si la densite ou le rayon ont change
	const float Radius = FMath::Clamp(World->GetSettings().FloraRadiusM, 30.f, 400.f) * 100.f;
	const FVector Center(CamPos.X, CamPos.Y, Hit.ImpactPoint.Z);
	if (!bShown || FVector::Dist2D(Center, BuiltCenter) > Radius / 3.f
		|| !FMath::IsNearlyEqual(Density, BuiltDensity, 0.05f) || !FMath::IsNearlyEqual(Radius, BuiltRadius, 100.f))
	{
		StartBuild(Center, Radius, Density);
	}
}
