// Copyright LTV Prod 2026. All Rights Reserved


#include "Components/CameraControlComponent.h"

#include "CineCameraComponent.h"
#include "CineCameraSettings.h"
#include "OWLCaptureComponent.h"
#include "GameFramework/SpringArmComponent.h"

#define ECC_CesiumChannel ECC_GameTraceChannel1

UCameraControlComponent::UCameraControlComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

   SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	// SpringArmComponent->SetupAttachment(GetOwner()->GetRootComponent());
	SpringArmComponent->SetRelativeTransform(FTransform::Identity);
	SpringArmComponent->TargetArmLength = 500000.0f;
	SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -500.0f));
	SpringArmComponent->SocketOffset = FVector(0.0f, 0.0f, 9900.0f);
	SpringArmComponent->bUsePawnControlRotation = false;
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritRoll = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->ProbeSize = 2000.f;
	SpringArmComponent->ProbeChannel = ECC_CesiumChannel;
	SpringArmComponent->SetRelativeRotation(FRotator(-15.f, 180.f, 0.f));
	
	CameraComponent = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	CameraComponent->SetCurrentFocalLength(24.f);
	CameraComponent->Overscan = 1.0f;
	FCameraFilmbackSettings CameraFilmbackSettings = FCameraFilmbackSettings();
	CameraFilmbackSettings.SensorWidth = 23.76f;
	CameraFilmbackSettings.SensorHeight = 13.365f;
	CameraComponent->Filmback = CameraFilmbackSettings;
	
	FPlateCropSettings PlateCropSettings = FPlateCropSettings();
	PlateCropSettings.AspectRatio = 1.77f;
	CameraComponent->CropSettings = PlateCropSettings;
	
	FPostProcessSettings PostProcessSettings = FPostProcessSettings();
	PostProcessSettings.bOverride_AutoExposureMethod = 1;
	PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
	PostProcessSettings.bOverride_AutoExposureBias = 1;
	PostProcessSettings.AutoExposureBias = 0.f;
	PostProcessSettings.bOverride_AutoExposureSpeedDown = 1;
	PostProcessSettings.AutoExposureSpeedDown = 100.f;
	PostProcessSettings.bOverride_AutoExposureSpeedUp = 1;
	PostProcessSettings.AutoExposureSpeedUp = 100.f;
	CameraComponent->PostProcessSettings = PostProcessSettings;
	
	OwlCapture = CreateDefaultSubobject<UOWLCaptureComponent>(TEXT("OwlCapture"));
	OwlCapture->SetupAttachment(CameraComponent);
	OwlCapture->SetRelativeTransform(FTransform::Identity);
	OwlCapture->bPauseRendering = true;
	OwlCapture->PrimaryComponentTick.bStartWithTickEnabled = false;
	OwlCapture->SetComponentTickEnabled(false);
}

void UCameraControlComponent::BeginPlay()
{
    Super::BeginPlay();
}


void UCameraControlComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCameraControlComponent::Pitch(float NewAngle)
{
}

void UCameraControlComponent::Pan(float NewDirection)
{
}

void UCameraControlComponent::Zoom(float NewDirection)
{
}

void UCameraControlComponent::Reset()
{
}

