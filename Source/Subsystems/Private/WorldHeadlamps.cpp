// Copyright LTV Prod 2026. All Rights Reserved

#include "WorldHeadlamps.h"
#include "WorldAmbienceSubsystem.h"
#include "RunnerInterface.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

namespace WorldHeadlamps
{
	constexpr float HeadHeight = 250.f;      // cm : la lampe est un peu au-dessus de la tete
	constexpr float MinRadius = 1500.f;      // cm : flaque de 15 m vue de pres
	constexpr float RadiusPerDistance = 0.012f; // rayon / distance camera : la lueur reste lisible de loin
	constexpr float BaseCandela = 60.f;      // intensite pour un rayon de 15 m
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
}

void AWorldHeadlamps::RefreshRunners()
{
	Runners.Reset();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->Implements<URunnerInterface>()) Runners.Add(*It);
	}
}

UPointLightComponent* AWorldHeadlamps::GetLamp(int32 Index)
{
	while (Lamps.Num() <= Index)
	{
		UPointLightComponent* Lamp = NewObject<UPointLightComponent>(this);
		Lamp->SetMobility(EComponentMobility::Movable);
		Lamp->SetCastShadows(false);              // pas d'ombre : cout quasi nul, meme avec beaucoup de coureurs
		Lamp->SetIntensityUnits(ELightUnits::Candelas);
		Lamp->SetUseTemperature(true);
		Lamp->SetTemperature(5600.f);              // LED de frontale, blanc neutre
		Lamp->SetIndirectLightingIntensity(0.3f);  // peu de rebond Lumen
		Lamp->SetSourceRadius(5.f);
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
	if (!World) return;

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
	const float Size = FMath::Max(Settings.HeadlampSize, 0.1f);

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
			const float Radius = FMath::Max(WorldHeadlamps::MinRadius, static_cast<float>(FVector::Dist(Head, CamPos)) * WorldHeadlamps::RadiusPerDistance) * Size;
			// Intensite proportionnelle au carre du rayon : meme eclat au centre quelle que soit la taille
			const float Candela = WorldHeadlamps::BaseCandela * FMath::Square(Radius / WorldHeadlamps::MinRadius) * Glow;

			UPointLightComponent* Lamp = GetLamp(LitCount++);
			Lamp->SetWorldLocation(Head);
			Lamp->SetAttenuationRadius(Radius);
			Lamp->SetIntensity(Candela);
			Lamp->SetVisibility(true);
		}
	}
	for (int32 i = LitCount; i < Lamps.Num(); ++i)
	{
		if (Lamps[i]->IsVisible()) Lamps[i]->SetVisibility(false);
	}
}
