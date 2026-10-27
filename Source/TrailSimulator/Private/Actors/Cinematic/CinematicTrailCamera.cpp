// All Rights Reserved

#include "Actors/Cinematic/CinematicTrailCamera.h"
#include "Actors/Cinematic/CinematicTrail.h"
#include "Components/SplineComponent.h"

namespace CinematicTrailCamera
{
	constexpr int32 AheadSamples = 4;
}

ACinematicTrailCamera::ACinematicTrailCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	// Apres l'evaluation de Sequencer (et de chaque sous-image du rendu), avant la mise a jour des cameras
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void ACinematicTrailCamera::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateFromTrail();
}

void ACinematicTrailCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFromTrail();
}

void ACinematicTrailCamera::UpdateFromTrail()
{
	const USplineComponent* Rail = IsValid(Trail) && Trail->HasTrail() ? Trail->GetRail() : nullptr;
	if (!Rail || Rail->GetNumberOfSplinePoints() < 2) return;

	const float Len = Rail->GetSplineLength();
	const float DistanceCm = FMath::Clamp(Progress, 0.f, 1.f) * Len;
	const float LeadCm = FMath::Max(1000.f, LeadM * 100.f);
	auto At = [&](float Offset)
	{
		return Rail->GetLocationAtDistanceAlongSpline(FMath::Clamp(DistanceCm + Offset, 0.f, Len), ESplineCoordinateSpace::World);
	};
	auto Flat = [](const FVector& V) { return FVector(V.X, V.Y, 0.f); };

	// Cap : point moyen en avance sur le rail, comme le survol live (sans son lissage temporel)
	const FVector Pivot = At(0.f);
	FVector Ahead = FVector::ZeroVector;
	for (int32 i = 1; i <= CinematicTrailCamera::AheadSamples; ++i)
	{
		Ahead += At(LeadCm * i / CinematicTrailCamera::AheadSamples);
	}
	Ahead /= CinematicTrailCamera::AheadSamples;

	FVector Heading = Flat(Ahead - Pivot);
	if (Heading.Size() < 5000.f)
	{
		// Fin du rail : tangente
		Heading += Flat(Rail->GetDirectionAtDistanceAlongSpline(DistanceCm, ESplineCoordinateSpace::World)) * 5000.f;
	}
	Heading = Heading.GetSafeNormal();
	if (Heading.IsNearlyZero()) return;

	const FVector Orbit = Heading.RotateAngleAxis(OrbitDeg, FVector::UpVector);
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Heading);
	const FVector CamPos = Pivot + (-Orbit * BackM + Right * SideM + FVector::UpVector * HeightM) * 100.f;

	const FVector Aim = FMath::Lerp(Ahead, Pivot, FMath::Clamp(AimAtPivot, 0.f, 1.f));
	FRotator CamRot = FRotationMatrix::MakeFromXZ(Aim - CamPos, FVector::UpVector).Rotator();
	CamRot.Pitch = FMath::Clamp(CamRot.Pitch + PitchOffsetDeg, -89.f, 89.f);
	CamRot.Roll = 0.f;

	SetActorLocationAndRotation(CamPos, CamRot);
}
