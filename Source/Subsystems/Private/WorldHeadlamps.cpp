// Copyright LTV Prod 2026. All Rights Reserved

#include "WorldHeadlamps.h"
#include "WorldAmbienceSubsystem.h"
#include "RunnerInterface.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace WorldHeadlamps
{
	const TCHAR* MeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");          // diametre 100 cm
	const TCHAR* MaterialPath = TEXT("/Game/LTVContent/Materials/Masters/M_MasterIllum.M_MasterIllum"); // unlit, BaseColor x IllumFixed
	constexpr float HeadHeight = 170.f;      // cm au-dessus du coureur
	constexpr float MinDiameter = 12.f;      // cm, vu de pres
	constexpr float ScreenRatio = 0.0025f;   // diametre / distance : ~2 px a 1080p quelle que soit la distance
}

AWorldHeadlamps::AWorldHeadlamps()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork; // apres le deplacement des coureurs et de la camera
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AWorldHeadlamps::BeginPlay()
{
	Super::BeginPlay();
	Ambience = GetWorld()->GetSubsystem<UWorldAmbienceSubsystem>();
	LampMesh = LoadObject<UStaticMesh>(nullptr, WorldHeadlamps::MeshPath);
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, WorldHeadlamps::MaterialPath))
	{
		LampMaterial = UMaterialInstanceDynamic::Create(Base, this);
		LampMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.88f, 0.7f)); // LED legerement chaude
	}
}

void AWorldHeadlamps::RefreshRunners()
{
	Runners.Reset();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->Implements<URunnerInterface>()) Runners.Add(*It);
	}
}

UStaticMeshComponent* AWorldHeadlamps::GetLamp(int32 Index)
{
	while (Lamps.Num() <= Index)
	{
		UStaticMeshComponent* Lamp = NewObject<UStaticMeshComponent>(this);
		Lamp->SetStaticMesh(LampMesh);
		Lamp->SetMaterial(0, LampMaterial);
		Lamp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Lamp->SetCastShadow(false);
		Lamp->SetAffectDistanceFieldLighting(false);
		Lamp->SetMobility(EComponentMobility::Movable);
		Lamp->SetupAttachment(GetRootComponent());
		Lamp->RegisterComponent();
		Lamps.Add(Lamp);
	}
	return Lamps[Index];
}

void AWorldHeadlamps::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const UWorldAmbienceSubsystem* World = Ambience.Get();
	if (!World || !LampMesh || !LampMaterial) return;

	// Les coureurs apparaissent et disparaissent au fil des courses : liste relue toutes les 2 s
	RefreshTimer -= DeltaSeconds;
	if (RefreshTimer <= 0.f)
	{
		RefreshTimer = 2.f;
		RefreshRunners();
	}

	const FWorldAmbienceSettings& Settings = World->GetSettings();
	// Allumage progressif entre la fin du jour et la nuit noire
	const float Night = FMath::SmoothStep(0.3f, 0.7f, World->GetState().Night);
	const float Glow = Settings.bHeadlamps ? Settings.HeadlampIntensity * Night : 0.f;
	if (!FMath::IsNearlyEqual(Glow, AppliedGlow, 0.01f))
	{
		AppliedGlow = Glow;
		LampMaterial->SetScalarParameterValue(TEXT("IllumFixed"), Glow * 25.f);
	}

	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	const FVector CamPos = Camera ? Camera->GetCameraLocation() : FVector::ZeroVector;

	LitCount = 0;
	if (Glow > 0.f)
	{
		for (const TWeakObjectPtr<AActor>& Weak : Runners)
		{
			const AActor* Runner = Weak.Get();
			const IRunnerInterface* Interface = Runner ? Cast<IRunnerInterface>(Runner) : nullptr;
			const UStaticMeshComponent* Foot = Interface ? Interface->GetFootMesh() : nullptr;
			// Seulement les coureurs affiches a l'antenne
			if (!Interface || Runner->IsHidden() || (Foot && !Foot->IsVisible())) continue;

			const FVector Head = Runner->GetActorLocation() + FVector(0.0, 0.0, WorldHeadlamps::HeadHeight);
			const float Diameter = FMath::Max(WorldHeadlamps::MinDiameter, static_cast<float>(FVector::Dist(Head, CamPos)) * WorldHeadlamps::ScreenRatio);
			UStaticMeshComponent* Lamp = GetLamp(LitCount++);
			Lamp->SetWorldLocationAndRotation(Head, FQuat::Identity, false, nullptr, ETeleportType::TeleportPhysics);
			Lamp->SetWorldScale3D(FVector(Diameter / 100.f));
			Lamp->SetVisibility(true);
		}
	}
	for (int32 i = LitCount; i < Lamps.Num(); ++i)
	{
		if (Lamps[i]->IsVisible()) Lamps[i]->SetVisibility(false);
	}
}
