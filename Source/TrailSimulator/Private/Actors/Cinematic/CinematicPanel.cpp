// All Rights Reserved

#include "Actors/Cinematic/CinematicPanel.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ACinematicPanel::ACinematicPanel()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	// Apres la camera cinema (TG_PostUpdateWork) : le panneau suit sa position de l'image en cours
	PrimaryActorTick.TickGroup = TG_LastDemotable;

	Plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
	SetRootComponent(Plane);
	Plane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Plane->SetCastShadow(false);
	Plane->SetMobility(EComponentMobility::Movable);
	ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (Mesh.Succeeded())
	{
		Plane->SetStaticMesh(Mesh.Object);
	}
	ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/Cinematics/MontBlanc/M_PanneauCinema.M_PanneauCinema"));
	if (Mat.Succeeded())
	{
		Plane->SetMaterial(0, Mat.Object);
	}
}

void ACinematicPanel::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	MID = nullptr;
	UpdatePanel();
}

void ACinematicPanel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdatePanel();
}

void ACinematicPanel::UpdatePanel()
{
	if (!MID && Plane->GetMaterial(0))
	{
		MID = Plane->CreateDynamicMaterialInstance(0, Plane->GetMaterial(0));
	}
	if (MID)
	{
		if (Image) MID->SetTextureParameterValue(TEXT("Image"), Image);
		MID->SetScalarParameterValue(TEXT("Opacite"), Opacity);
	}
	Plane->SetVisibility(Opacity > 0.002f);

	const FVector Base = Anchor + FVector(0, 0, HeightM * 100.f);
	const UCameraComponent* Cam = Camera ? Camera->FindComponentByClass<UCameraComponent>() : nullptr;
	const FVector CamLoc = Cam ? Cam->GetComponentLocation() : Base - FVector(100000.f, 0, 0);

	// Taille : fraction de la hauteur de l'image a la distance du panneau (champ vertical de la camera)
	float SizeCm = SizeM * 100.f;
	if (ScreenSize > 0.f && Cam)
	{
		const float Aspect = Cam->AspectRatio > 0.f ? Cam->AspectRatio : 16.f / 9.f;
		const float VFov = 2.f * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(Cam->FieldOfView) * 0.5f) / Aspect);
		SizeCm = ScreenSize * 2.f * FVector::Dist(CamLoc, Base) * FMath::Tan(VFov * 0.5f);
	}

	// Plan moteur : 100 x 100 cm dans XY, normale +Z. On le dresse face a la camera (axe vertical conserve)
	FVector ToCam = CamLoc - Base;
	ToCam.Z = 0.f;
	if (ToCam.IsNearlyZero()) ToCam = FVector(1, 0, 0);
	const FVector N = ToCam.GetSafeNormal();
	const FVector Right = FVector::CrossProduct(FVector::UpVector, -N);
	// Axes du plan : X (u de la texture) vers la droite de l'ecran, Y (v, vers le bas de l'image) vers le bas, Z vers la camera
	const FQuat Rot = FMatrix(Right, -FVector::UpVector, N, FVector::ZeroVector).ToQuat();
	const float S = SizeCm / 100.f;
	// Ancrage en bas au centre : le plan est centre, on le remonte d'une demi-taille
	SetActorLocationAndRotation(Base + FVector(0, 0, SizeCm * 0.5f), Rot);
	SetActorScale3D(FVector(S, S, 1.f));
}
