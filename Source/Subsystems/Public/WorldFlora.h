// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldFlora.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class UWorldAmbienceSubsystem;

/**
 * @brief Flore au ras du sol autour de la camera : herbe, buissons et rochers poses sur les tuiles Cesium.
 *
 * Les meshes sont pris dans /Game/LTVContent/Flora/Grass, /Shrubs et /Rocks (par exemple des Megascans
 * importes depuis Fab), jamais sur les plans d'eau OSM (UWaterMaskSubsystem). La dispersion est deterministe par cellule de 10 m, recalculee quand la camera
 * s'est deplacee, par petits lots de lancers de rayon pour ne pas creer de pic d'image. Herbe sur les
 * pentes douces, rochers sur les pentes fortes, moins de buissons au-dessus de 2300 m. Active seulement
 * quand la camera est a moins de 80 m du sol ; densite et rayon suivent la regie et le gardien.
 */
UCLASS(NotPlaceable, Transient)
class SUBSYSTEMS_API AWorldFlora : public AActor
{
	GENERATED_BODY()

public:
	AWorldFlora();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	int32 GetInstanceCount() const { return ShownInstances; }
	const FString& GetStatus() const { return Status; }

private:
	enum class EKind : uint8 { Grass, Shrub, Rock, Count };

	struct FKindSet
	{
		TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> Components;
	};

	void LoadMeshes();
	void StartBuild(const FVector& Center, float RadiusCm, float Density);
	void ContinueBuild();
	void ApplyBuild();
	void HideAll();
	bool TraceGround(const FVector& XY, FHitResult& OutHit) const;

	UPROPERTY()
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> AllComponents;

	FKindSet Kinds[static_cast<int32>(EKind::Count)];
	TWeakObjectPtr<UWorldAmbienceSubsystem> Ambience;

	// Construction en cours : cellules restant a traiter et instances obtenues par composant
	struct FPendingCell { int32 X; int32 Y; };
	TArray<FPendingCell> PendingCells;
	TMap<UHierarchicalInstancedStaticMeshComponent*, TArray<FTransform>> PendingInstances;
	bool bBuilding = false;
	FVector BuildCenter = FVector::ZeroVector;
	float BuildDensity = 0.f;
	float BuildRadius = 0.f;

	FVector BuiltCenter = FVector(TNumericLimits<float>::Max());
	float BuiltDensity = -1.f;
	float BuiltRadius = -1.f;
	float CheckTimer = 0.f;
	TWeakObjectPtr<class ACesiumGeoreference> Georeference;
	TWeakObjectPtr<class UWaterMaskSubsystem> WaterMask;
	int32 ShownInstances = 0;
	bool bShown = false;
	FString Status;
};
