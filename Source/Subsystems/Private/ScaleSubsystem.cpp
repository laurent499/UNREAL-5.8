// ScaleByDistanceSubsystem.cpp

#include "ScaleSubsystem.h"
#include "CineCameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"

void UScaleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GlobalConfig.ScaleMinMax = FMinMax(FVector(50.f), FVector(200.f));
	GlobalConfig.NearDistanceCm = 50000.f; // 500m
	GlobalConfig.FarDistanceCm = 1000000.f; // 10km
	GlobalConfig.bSmoothStep = true;
	GlobalConfig.InterpSpeed = 8.f;
	GlobalConfig.UpdateIntervalSec = 0.015f;
}

void UScaleSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UScaleSubsystem::SetGlobalConfig(const FDistanceScaleConfig& NewConfig)
{
	GlobalConfig = NewConfig;
}

void UScaleSubsystem::SetNearFar(float NewNear, float NewFar)
{
	GlobalConfig.NearDistanceCm = NewNear;
	GlobalConfig.FarDistanceCm = NewFar;
}

float UScaleSubsystem::GetNear() const
{
	return GlobalConfig.NearDistanceCm;
}

float UScaleSubsystem::GetFar() const
{
	return GlobalConfig.FarDistanceCm;
}

void UScaleSubsystem::SetActorScaleMinMax(AActor* Actor, const FMinMax& NewMinMax)
{
	for (FEntry& E : Entries)
	{
		if (E.Actor.Get() == Actor)
		{
			E.CurrentMinMax = NewMinMax;
			return;
		}
	}
}

void UScaleSubsystem::SetGlobalMinMax(float MinValue, float MaxValue)
{
	GlobalConfig.ScaleMinMax = FMinMax(FVector(MinValue), FVector(MaxValue));	
}

void UScaleSubsystem::RegisterScalableActor(AActor* Actor)
{
	if (!IsValid(Actor)) return;
	if (!Actor->GetClass()->ImplementsInterface(UScaleInterface::StaticClass())) return;

	for (const FEntry& E : Entries)
	{
		if (E.Actor.Get() == Actor)
		{
			return;
		}
	}

	FEntry NewEntry;
	NewEntry.Actor = Actor;
	NewEntry.OriginalScale = Actor->GetActorScale3D();

	// Base = config globale
	NewEntry.CurrentMinMax = GlobalConfig.ScaleMinMax;

	Entries.Add(MoveTemp(NewEntry));
	Actor->OnEndPlay.AddDynamic(this, &UScaleSubsystem::HandleActorEndPlay);
}

void UScaleSubsystem::UnregisterScalableActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (Entries[i].Actor.Get() == Actor)
		{
			Entries.RemoveAtSwap(i);
		}
	}
}

void UScaleSubsystem::HandleActorEndPlay(AActor* Actor, EEndPlayReason::Type Reason)
{
	UnregisterScalableActor(Actor);
}

bool UScaleSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return Entries.Num() > 0 && World && World->IsGameWorld();
}

TStatId UScaleSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UScaleSubsystem, STATGROUP_Tickables);
}

bool UScaleSubsystem::GetCameraLocation(FVector& OutCamLoc) const
{
	const UWorld* World = GetWorld();
	if (!World) return false;

	APlayerController* PC = World->GetFirstPlayerController();
	// APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AActor* ViewTarget = PC ? PC->GetViewTarget() : nullptr;
	UCineCameraComponent* CamComp = ViewTarget->GetComponentByClass<UCineCameraComponent>();
	
	if (!PC) return false;

	if (PC->PlayerCameraManager)
	{
		// OutCamLoc = PC->PlayerCameraManager->GetCameraLocation();
		OutCamLoc = CamComp->GetComponentLocation();
		return true;
	}
	
	// Fallback
	OutCamLoc = PC->GetPawn()->GetActorLocation();
	return true;
}

void UScaleSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Nettoyage + update
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		FEntry& E = Entries[i];
		AActor* A = E.Actor.Get();

		if (!IsValid(A))
		{
			Entries.RemoveAtSwap(i);
			continue;
		}

		UWorld* World = A->GetWorld();
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		// const FVector CamLoc =
		// 	(PC && PC->PlayerCameraManager)
		// 	? PC->PlayerCameraManager->GetCameraLocation()
		// 	: FVector::ZeroVector;
		
		const FVector CamLoc = PC->GetViewTarget()->GetComponentByClass<UCineCameraComponent>()->GetComponentLocation();
		
		const FVector ActorLoc = A->GetActorLocation();
		const FVector TargetScale = ComputeTargetScale(E, CamLoc, ActorLoc);

		const FDistanceScaleConfig& Cfg = GlobalConfig;

		// Optionnel : interp
		FVector NewScale = TargetScale;
		if (Cfg.bInterp)
		{
			// DeltaTime réel => beaucoup plus smooth
			NewScale = FMath::VInterpTo(A->GetActorScale3D(), TargetScale, DeltaTime, Cfg.InterpSpeed);
		}

		if (!A->GetActorScale3D().Equals(NewScale, Cfg.ScaleEpsilon))
		{
			A->SetActorScale3D(NewScale);
		}
	}
}

FVector UScaleSubsystem::ComputeTargetScale(const FEntry& Entry, const FVector& CamLoc, const FVector& ActorLoc) const
{
	const FDistanceScaleConfig& Cfg = GlobalConfig;

	const float Near = FMath::Max(0.f, Cfg.NearDistanceCm);
	const float Far  = FMath::Max(Near + KINDA_SMALL_NUMBER, Cfg.FarDistanceCm);

	// Distance
	float Dist = 0.f;
	if (Cfg.bUseXYOnly)
	{
		const FVector2D D = FVector2D(ActorLoc.X - CamLoc.X, ActorLoc.Y - CamLoc.Y);
		Dist = D.Size();
	}
	else
	{
		Dist = FVector::Dist(ActorLoc, CamLoc);
	}

	// Alpha 0=proche (min) -> 1=loin (max)
	float Alpha = (Dist - Near) / (Far - Near);
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	if (Cfg.bSmoothStep)
	{
		Alpha = Alpha * Alpha * (3.f - 2.f * Alpha); // smoothstep
	}

	const FVector MinScale = Entry.CurrentMinMax.Min;
	const FVector MaxScale = Entry.CurrentMinMax.Max;
	return FMath::Lerp(MinScale, MaxScale, Alpha);
	
}
