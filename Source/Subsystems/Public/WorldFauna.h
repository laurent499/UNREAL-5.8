// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldFauna.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInstanceDynamic;
class UWorldAmbienceSubsystem;

/**
 * @brief Faune legere autour de la camera : rapaces qui tournent en vol plane (25 a 70 m du sol) et
 * petits vols groupes (15 a 35 m), autour d'un point a 80 m devant la camera.
 *
 * Tout tient dans deux composants instancies (corps et ailes), avec des meshes construits au lancement :
 * quelques dizaines d'instances, sans ombre, mises a jour sur le CPU. Les oiseaux n'apparaissent que
 * de jour, sans forte pluie ni grand vent, quand la camera est sous l'altitude de visibilite reglee (FaunaMaxHeightM) ; leur nombre
 * suit la densite de faune effective (reglage regie et gardien de performance).
 */
UCLASS(NotPlaceable, Transient)
class SUBSYSTEMS_API AWorldFauna : public AActor
{
	GENERATED_BODY()

public:
	AWorldFauna();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Etat pour la regie : oiseaux affiches, et sinon pourquoi */
	int32 GetVisibleBirdCount() const { return bVisible ? Birds.Num() : 0; }
	const FString& GetStatus() const { return Status; }

private:
	enum class EBirdType : uint8 { Raptor, Flock };

	struct FBird
	{
		EBirdType Type = EBirdType::Raptor;
		FVector Position = FVector::ZeroVector;
		FVector Velocity = FVector::ForwardVector;
		float Scale = 1.f;
		float FlapPhase = 0.f;
		float FlapSpeed = 10.f;   // rad/s
		float FlapAmount = 0.f;   // 0 vol plane, 1 battement complet
		float FlapTimer = 0.f;    // rapaces : prochaine serie de battements
		// Rapaces : orbite dans une ascendance
		FVector OrbitCenter = FVector::ZeroVector;
		float OrbitAngle = 0.f;
		float OrbitRadius = 10000.f;
		float OrbitSpeed = 0.2f;  // rad/s, signe = sens de rotation
		float HeightAboveGround = 12000.f;
		// Vol groupe : decalage dans la formation
		FVector FormationOffset = FVector::ZeroVector;
	};

	void BuildMeshes();
	void Respawn(const FVector& Anchor, float GroundZ);
	bool UpdateAnchor(float DeltaSeconds);
	bool IsFaunaAllowed();
	void Simulate(float DeltaSeconds);
	void PushInstances();
	float GroundZAt(const FVector& Location) const;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Bodies;
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Wings;
	UPROPERTY()
	TObjectPtr<UStaticMesh> BodyMesh;
	UPROPERTY()
	TObjectPtr<UStaticMesh> WingMesh;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BirdMaterial;

	TWeakObjectPtr<UWorldAmbienceSubsystem> Ambience;
	TArray<FBird> Birds;

	FVector Anchor = FVector::ZeroVector;
	float AnchorGroundZ = 0.f;
	float CameraHeightAboveGround = 0.f;
	float AnchorTimer = 0.f;
	float AppliedDensity = -1.f;
	bool bHasAnchor = false;
	bool bVisible = false;
	float SpawnCameraHeight = 0.f;
	FString Status;

	// Chef du vol groupe : cap qui derive lentement autour du point d'ancrage
	FVector FlockLeader = FVector::ZeroVector;
	float FlockHeading = 0.f;
	float FlockHeight = 5000.f;
	float FlockTime = 0.f;
};
