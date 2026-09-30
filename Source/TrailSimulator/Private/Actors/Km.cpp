// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Km.h"
#include "CesiumFlyToComponent.h"
#include "CesiumGlobeAnchorComponent.h"
#include "RaceManager.h"
#include "RaceSubsystem.h"
#include "SettingsSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

AKm::AKm()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);
	
	MainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MainRoot"));
	SetRootComponent(MainRoot);
	MainRoot->SetRelativeScale3D(FVector(10.f));
	MainRoot->SetComponentTickEnabled(false);
	
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComp->SetupAttachment(MainRoot);
	StaticMeshComp->SetComponentTickEnabled(false);
	StaticMeshComp->SetGenerateOverlapEvents(false);
	StaticMeshComp->SetRelativeTransform(FTransform::Identity);
	StaticMeshComp->SetRelativeRotation(FRotator(0.0f,90.0f,0.0f));
	
	LabelComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LabelComponent"));
	LabelComp->SetupAttachment(MainRoot);
	LabelComp->SetComponentTickEnabled(false);
	LabelComp->SetGenerateOverlapEvents(false);
	LabelComp->SetRelativeTransform(FTransform::Identity);
	LabelComp->SetTextRenderColor(FColor(255,255,255,255));
	LabelComp->SetWorldSize(55.f);
	LabelComp->SetHorizontalAlignment(EHTA_Center);
	LabelComp->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextBottom);
	LabelComp->SetRelativeLocation(FVector(0.0f,0.0f,70.0f));
	
	// Globe anchor
	GlobeAnchorComponent = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchorComponent"));
	GlobeAnchorComponent->SetAdjustOrientationForGlobeWhenMoving(true);
	Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	GlobeAnchorComponent->SetGeoreference(Georeference);
	
}

void AKm::BeginPlay()
{
	Super::BeginPlay();
	
	DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	UCesiumFlyToComponent* FlyToComponent = DynaPawn->GetComponentByClass<UCesiumFlyToComponent>();
	FlyToComponent->OnFlightComplete.AddDynamic(this, &AKm::UpdateGlobeAnchor);
	
	if (GetGameInstance())
	{
		SettingsSubsystem = GetGameInstance()->GetSubsystem<USettingsSubsystem>();
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
	}
	
	// LookAt
	GetWorldTimerManager().SetTimer(
		LookAtTimerHandle,
		this,
		&AKm::OnLookAtTimerTick,
		SettingsSubsystem->GetUpdateIntervalSeconds(),
		true // looping
	);
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &AKm::UpdateDayNight);
	}
}

/**
 * @brief Updating Label and Color
 * @param RaceID 
 * @param NewLabel 
 */
void AKm::UpdateKm(int64 RaceID, const FText& NewLabel) const
{
	LabelComp->SetText(NewLabel);
	
	const FRaceSetup& CurrentSetup = RaceSubsystem->GetRaceSetupById(RaceID);
	FString RaceColor= CurrentSetup.color;
	
	FLinearColor LinearColor = FLinearColor::FromSRGBColor(FColor::FromHex(RaceColor));
	StaticMeshComp->SetCustomPrimitiveDataFloat(11, 0.f); // UseSlope = 0
	StaticMeshComp->SetCustomPrimitiveDataFloat(10, 0.f); // Slope value
	StaticMeshComp->SetCustomPrimitiveDataVector4(12, LinearColor); // Color (12->15)
	StaticMeshComp->SetCustomPrimitiveDataFloat(16, 10.f); // Glow
	LabelComp->SetTextRenderColor(FColor::FromHex(RaceColor));
	LabelComp->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
}

void AKm::UpdateGlobeAnchor()
{
	GlobeAnchorComponent->SnapLocalUpToEllipsoidNormal();
}

void AKm::UpdateDayNight(bool bIsDay)
{
	if (bIsDay)
	{
		LabelComp->SetCustomPrimitiveDataFloat(0, 0.125);
	} else
	{
		LabelComp->SetCustomPrimitiveDataFloat(0, 0.25);
	}
}

/**
 * @brief LookAt
 */
void AKm::OnLookAtTimerTick()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	UCameraComponent* ActiveCam = nullptr;
	AActor* ViewTarget = PC->GetViewTarget();
	if (AActor* VT = ViewTarget)
	{
		ActiveCam = VT->FindComponentByClass<UCameraComponent>();
	}
	if (ActiveCam)
	{
		const FVector MyLoc = GetActorLocation();
		FVector TargetLoc = ActiveCam->GetComponentLocation();
		TargetLoc.Z = MyLoc.Z;	// only Yaw
		
		FVector ToTarget = (TargetLoc - MyLoc);
		if (ToTarget.IsNearlyZero())
		{
			return;
		}
		const FRotator DesiredRot = ToTarget.Rotation();
		FRotator FinalRot = DesiredRot;
		FinalRot.Pitch = 0.f;
		FinalRot.Roll  = 0.f;
		const FRotator NewRot = FMath::RInterpTo(
			GetActorRotation(),
			FinalRot,
			GetWorld()->GetDeltaSeconds(),
			SettingsSubsystem->GetInterpSpeed());
		SetActorRotation(NewRot);
	}
}