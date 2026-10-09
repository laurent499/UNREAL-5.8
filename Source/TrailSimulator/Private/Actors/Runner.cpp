// All Rights Reserved


#include "TrailSimulator/Public/Actors/Runner.h"

#include "BroadcastCaptureSubsystem.h"
#include "TrailSimulator/Public/Actors/Path.h"
#include "CesiumFlyToComponent.h"
#include "CesiumGeoreference.h"
#include "CesiumGlobeAnchorComponent.h"
#include "CineCameraComponent.h"
#include "OWLCaptureComponent.h"
#include "PathInterface.h"
#include "PathSubsystem.h"
#include "RaceSubsystem.h"
#include "RunnerStackingSubsystem.h"
#include "RunnerSubsystem.h"
#include "ScaleSubsystem.h"
#include "SettingsSubsystem.h"
#include "SlateNotificationsBFL.h"
#include "TrailInputModeInterface.h"
#include "WorldUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/RawTextMaterial.h"

#define ECC_CesiumChannel ECC_GameTraceChannel1

class USettingsSubsystem;
class UCesiumFlyToComponent;
/**
 * @brief Runner Constructor
 */
ARunner::ARunner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetActorEnableCollision(false);
	
	if (GetGameInstance())
	{
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
	}
	
	StackState = EStackState::ESS_Unstacked;
	
	// Globe anchor
	GlobeAnchorComponent = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchorComponent"));
	GlobeAnchorComponent->SetAdjustOrientationForGlobeWhenMoving(true);
	Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	GlobeAnchorComponent->SetGeoreference(Georeference);
	
	MainRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MainRoot"));
	MainRootComponent->SetComponentTickEnabled(false);
	SetRootComponent(MainRootComponent);
	
	PresentationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationRoot"));
	PresentationRoot->SetupAttachment(MainRootComponent);
	PresentationRoot->SetComponentTickEnabled(false);
	PresentationRoot->SetRelativeLocation(FVector(0.f, 0.f, 645.f));
	
	// Foot	
	FootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FootComponent"));
	FootComponent->SetupAttachment(MainRootComponent);
	FootComponent->SetGenerateOverlapEvents(false);
	FootComponent->SetComponentTickEnabled(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FootMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/Foot.Foot"));
	if (FootMesh.Succeeded())
	{
		FootComponent->SetStaticMesh(FootMesh.Object);
	}
	
	FootHook = CreateDefaultSubobject<USceneComponent>(TEXT("FootHook"));
	FootHook->SetupAttachment(PresentationRoot);
	FootHook->SetRelativeTransform(FTransform::Identity);
	FootHook->SetComponentTickEnabled(false);
	FootHook->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	
	// Orbit
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(PresentationRoot);
	SpringArmComponent->SetRelativeTransform(FTransform::Identity);
	SpringArmComponent->TargetArmLength = 500000.0f;
	SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f));
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
		
	/** Name */
	NameHook = CreateDefaultSubobject<USceneComponent>(TEXT("NameHook"));
	NameHook->SetupAttachment(PresentationRoot);
	NameHook->SetComponentTickEnabled(false);
	NameHook->SetRelativeTransform(FTransform::Identity);
	
	TArray<FName> ActorTags = TArray<FName>();
	ActorTags.Reserve(1);
	ActorTags.Add(TEXT("Stackable"));
	Tags = ActorTags;
	
}

void ARunner::SetBroadcastCaptureEnabled_Implementation(bool bEnabled, class UTextureRenderTarget2D* SharedRT)
{
	if (bEnabled)
	{
		OwlCapture->SetComponentTickEnabled(true);
		OwlCapture->Activate(true);
		OwlCapture->bPauseRendering = false;
		OwlCapture->TextureTarget = SharedRT;
	} else
	{
		OwlCapture->bPauseRendering = true;            // pause
		OwlCapture->SetComponentTickEnabled(false);    // stop tick
		OwlCapture->Deactivate();

		OwlCapture->TextureTarget = nullptr;
	}
}

void ARunner::BeginPlay()
{
	Super::BeginPlay();
	// Les textes (M_RawText) comparent leur profondeur a la CustomDepth des autres elements du
	// runner : ils sont ainsi masques par les cartes et photos translucides placees devant eux.
	TrailRawText::EnableOcclusion(this);
	// Le pied (poteau) est translucide comme les cartes : sans priorite, il passait par-dessus
	// le cartouche INDEX quand la camera est basse. Priorite basse = dessine avant les cartes.
	if (FootComponent)
	{
		FootComponent->SetTranslucentSortPriority(-10);
	}
	DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	UCesiumFlyToComponent* FlyToComponent = DynaPawn->GetComponentByClass<UCesiumFlyToComponent>();
	FlyToComponent->OnFlightComplete.AddDynamic(this, &ARunner::UpdateGlobeAnchor);
	if (GetWorld())
	{
		StackingSubsystem = FWorldUtils::GetWorldSubsystemOrLog<URunnerStackingSubsystem>(this, TEXT(__FUNCTION__), false);
		ScaleSubsystem = FWorldUtils::GetWorldSubsystemOrLog<UScaleSubsystem>(this, TEXT(__FUNCTION__), false);
		BroadCastSubsystem = FWorldUtils::GetWorldSubsystemOrLog<UBroadcastCaptureSubsystem>(this, TEXT(__FUNCTION__), false);
		
		if (ScaleSubsystem)
		{
			ScaleSubsystem->RegisterScalableActor(this);
		}
	}
	
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), false);
	SettingsSubsystem->OnMinMaxSet.AddDynamic(this, &ARunner::SetMinMax);
	
	PathSubsystem = FWorldUtils::GetGISubsystemOrLog<UPathSubsystem>(this, TEXT(__FUNCTION__), false);
	RunnerSubsystem = FWorldUtils::GetGISubsystemOrLog<URunnerSubsystem>(this, TEXT(__FUNCTION__), false);
	
	// LookAt
	GetWorldTimerManager().SetTimer(
		LookAtTimerHandle,
		this,
		&ARunner::OnLookAtTimerTick,
		SettingsSubsystem->GetUpdateIntervalSeconds(),
		true,
		// premier declenchement aleatoire : les acteurs ne tournent pas tous sur la meme image
		FMath::FRandRange(0.f, SettingsSubsystem->GetUpdateIntervalSeconds())
	);
	
	SpringArmComponent->SetUsingAbsoluteRotation(false);
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, SettingsSubsystem->GetZAnchor()));
	SpringArmComponent->TargetArmLength = SettingsSubsystem->GetArmLength();
	SpringArmComponent->SetRelativeRotation(FRotator(SettingsSubsystem->GetCameraPitch(), 0.f, 0.f));
}

void ARunner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LookAtTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ARunner::UpdateGlobeAnchor()
{
	GlobeAnchorComponent->SnapLocalUpToEllipsoidNormal();
}

void ARunner::TriggerUpdateAfterHidden(int64 RunnerID)
{
	
	TObjectPtr<AActor> PathActor = PathSubsystem->GetPathById(RaceSubsystem->GetCurrentRaceId());
	if (TObjectPtr<APath> Path = Cast<APath>(PathActor))
	{
		// Reapparition : placement direct, sans interpolation depuis l'ancienne position
		bHasTrackDist = false;
		UpdateRunnerLocation(RunnerDatas, Path);
	}
}

void ARunner::UpdateRunnerLocation(FRunnerStruct RunnerStruct, TObjectPtr<APath> CurrentPath)
{
	Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	if (IPathInterface* PathInterface = Cast<IPathInterface>(CurrentPath))
	{
		const FVector NewRunnerLocation = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(
				FVector(RunnerStruct.lon, RunnerStruct.lat, RunnerStruct.elevation));
		const FVector TargetOnSpline = PathInterface->GetClosestSplineLocation(NewRunnerLocation);
		const float TargetDist = PathInterface->GetDistanceAlongSpline(TargetOnSpline);

		// Le stacking travaille sur la position cible (vraie position track)
		const FTransform RunnerTransform = FTransform(GetActorRotation(), TargetOnSpline, GetActorScale3D());
		if (!IsHidden())
			StackingSubsystem->UpdateRunnerTrackState(this, RunnerTransform, TargetDist);
		
		if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(this))
		{
			if (SI->IsStacked())
			{
				// Empile : il suit son parent, on note juste ou il en est sur le trace
				bTrackInterpActive = false;
				bHasTrackDist = false;
				return;
			}
		}

		// Premier placement, changement de trace ou saut trop grand : teleportation
		const float Dt = FMath::Max(SettingsSubsystem->GetFetchFrequency(), 0.1f);
		const float MaxInterpCm = FMath::Max(100.f * 100.f, 15.f * 100.f * Dt); // 100 m ou 15 m/s
		if (!bHasTrackDist || TrackInterpPath.Get() != CurrentPath.Get()
			|| FMath::Abs(TargetDist - TrackDisplayedDist) > MaxInterpCm)
		{
			SetActorLocation(TargetOnSpline, false);
			TrackInterpPath = CurrentPath.Get();
			TrackDisplayedDist = TargetDist;
			bHasTrackDist = true;
			bTrackInterpActive = false;
			return;
		}

		// Interpolation le long du trace depuis la position affichee vers la cible.
		// Duree un peu superieure a la periode : le snapshot suivant arrive avant l'arret, le mouvement reste continu.
		TrackFromDist = TrackDisplayedDist;
		TrackToDist = TargetDist;
		TrackInterpElapsed = 0.f;
		TrackInterpDuration = Dt * 1.1f;
		bTrackInterpActive = !FMath::IsNearlyEqual(TrackFromDist, TrackToDist, 1.f);
	} else
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Path Inteface"))), EMessageType::Error);
	}
}

bool ARunner::AdvanceTrackInterp(float DeltaTime)
{
	if (!bTrackInterpActive) return false;

	APath* Path = TrackInterpPath.Get();
	if (!Path || bStackedState || GetAttachParentActor())
	{
		bTrackInterpActive = false;
		bHasTrackDist = false;
		return false;
	}

	TrackInterpElapsed += DeltaTime;
	const float Alpha = FMath::Min(TrackInterpElapsed / TrackInterpDuration, 1.f);
	TrackDisplayedDist = FMath::Lerp(TrackFromDist, TrackToDist, Alpha);
	const bool bDone = Alpha >= 1.f;
	if (bDone) bTrackInterpActive = false;

	// Hors champ ou cache : on ne deplace l'acteur qu'a la fin (economie du thread de jeu)
	if (!bDone && (IsHidden() || !WasRecentlyRendered(0.5f))) return true;

	SetActorLocation(Path->GetLocationAtDistance(TrackDisplayedDist), false);
	return !bDone;
}

/**
 * @brief Updating the Runner datas
 * @param Runner FRunnerStruct
 * @param RaceSetup FRaceSetup
 */
void ARunner::UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup)
{
	RunnerDatas = MoveTemp(Runner);
	// Des composants apparaissent / disparaissent pendant la partie (photo, drapeau, club...) :
	// on repasse sur tous ceux presents a chaque mise a jour (idempotent, sans cout si deja actifs).
	TrailRawText::EnableOcclusion(this);

	ScaleSubsystem->SetActorScaleMinMax(this, SettingsSubsystem->GetMinMaxById(RaceSetup.raceId));
	RaceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<URaceSubsystem>();
	FRaceSetup CurrentSetup = RaceSubsystem->GetRaceSetupByName(RunnerDatas.raceName); 
	FString RaceColor = CurrentSetup.color;
	FColor SRGBColor = FColor::FromHex(RaceColor);
	FLinearColor LinearColor = FLinearColor::FromSRGBColor(SRGBColor);
	UMaterialInterface* CoreMat = FootComponent->GetMaterial(0);
	UMaterialInstanceDynamic* MID = FootComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, CoreMat);
	MID->SetVectorParameterValue(TEXT("BaseColor"), LinearColor);
	
	float CameraPitch = SettingsSubsystem->GetCameraPitch();
	FRotator CameraRot = FRotator(CameraPitch, 180.0f, 0.0f);
	SpringArmComponent->SetRelativeRotation(CameraRot);
}

void ARunner::UpdateDayNight(bool bIsDay)
{
}

void ARunner::AssignRunnerToTeam(int64 AssignedIdTeam)
{
	IdTeam = AssignedIdTeam;
}

void ARunner::StartAnimation(float RotationSpeed)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn && PlayerPawn->GetClass()->ImplementsInterface(UTrailInputModeInterface::StaticClass()))
	{
		ITrailInputModeInterface::Execute_RequestAnimationInput(PlayerPawn, this);
	}
	GetWorldTimerManager().ClearTimer(LookAtTimerHandle);
	LookAtTimerHandle.Invalidate();
	bLookAtEnabled = false;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		OriginalViewTarget = PC->GetViewTarget();
		BroadCastSubsystem->RequestViewTarget(PC, this, 1.f);
	}
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	bOrbitEnabled = true;
	OrbitElapsed = 0.f;
	StartYaw = GetActorRotation().Yaw;
}

void ARunner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bOrbitEnabled) return;
	
	// v2 stacked
	if (AActor* ParentActor = GetAttachParentActor())
	{
		bOrbitEnabled = false;
		bLookAtEnabled = true;
		if (ARunner* ParentRunner = Cast<ARunner>(ParentActor))
		{
			ParentRunner->bOrbitEnabled = true;
			ParentRunner->bLookAtEnabled = false;
			ParentRunner->SetActorTickEnabled(true);
			ParentRunner->PrimaryActorTick.SetTickFunctionEnable(true);
			SetActorTickEnabled(false);
		}
		
	} else	
	{
		AddActorWorldRotation(FRotator(0.f, OrbitSpeedDegPerSec * DeltaTime, 0.f));
	}
}

void ARunner::StopAnimation()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn && PlayerPawn->GetClass()->ImplementsInterface(UTrailInputModeInterface::StaticClass()))
	{
		ITrailInputModeInterface::Execute_ReleaseAnimationInput(PlayerPawn, this);
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (OriginalViewTarget)
		{
			BroadCastSubsystem->RequestViewTarget(PC, OriginalViewTarget, 1.f);
			OriginalViewTarget = nullptr;
		}
	}
	GetWorldTimerManager().ClearTimer(RotHandle);
	RotHandle.Invalidate();
	
	if (AActor* ParentActor = GetAttachParentActor())
	{
		ParentActor->SetActorTickEnabled(false);
		if (ARunner* ParentRunner = Cast<ARunner>(ParentActor))
		{
			ParentRunner->bOrbitEnabled = false;
			ParentRunner->bLookAtEnabled = true;
			ParentRunner->PrimaryActorTick.SetTickFunctionEnable(false);
		}
		
	} else
	{
		bLookAtEnabled = true;
		SetActorTickEnabled(false);
		bOrbitEnabled = false;
		PrimaryActorTick.SetTickFunctionEnable(false);
	}
	
	// LookAt
	GetWorldTimerManager().SetTimer(
		LookAtTimerHandle,
		this,
		&ARunner::OnLookAtTimerTick,
		SettingsSubsystem->GetUpdateIntervalSeconds(),
		true,
		// premier declenchement aleatoire : les acteurs ne tournent pas tous sur la meme image
		FMath::FRandRange(0.f, SettingsSubsystem->GetUpdateIntervalSeconds())
	);
}

void ARunner::MarkDirty()
{
	StackingSubsystem->MakeDirty();
}

/**
 * @brief Updating the MinMax
 * @param MinValue float
 * @param MaxValue float
 * @param RaceID int64
 */
void ARunner::SetMinMax(float MinValue, float MaxValue, int64 RaceID)
{
	if (RunnerSubsystem->DoesRunnerBelongsToRace(RunnerDatas.runnerId, RaceID))
	{
		RunnerDatas.MinMax = FMinMax(FVector(MinValue), FVector(MaxValue));
		MinMax = RunnerDatas.MinMax;
		ScaleSubsystem->RegisterScalableActor(this);
		ScaleSubsystem->SetActorScaleMinMax(this, MinMax);
	}
}

/**
 * @brief Gets the MinMax for the runner
 * @return FMinMax()
 */
FMinMax ARunner::GetRunnerMinMax() const
{
	return RunnerDatas.MinMax;
}

float ARunner::GetRunnerVDelta() const
{
	return RunnerDatas.VDelta;
}

USceneComponent* ARunner::GetStackHookComponent() const
{
	return FootHook;
}

USceneComponent* ARunner::GetStackAttachComponent() const
{
	return GetRootComponent();
}

void ARunner::SetIsStacked(bool bInStacked)
{
	bStackedState = bInStacked;
	if (FootComponent)
	{
		FootComponent->SetHiddenInGame(bInStacked, false);
		FootComponent->SetVisibility(!bInStacked, false);
		FootComponent->SetCollisionEnabled(
			bInStacked ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly
		);
	}
}

bool ARunner::IsStacked() const
{
	return bStackedState;
}

void ARunner::OnLookAtTimerTick()
{
	// Hors champ (ni vue principale ni capture OWL) : inutile de le tourner vers la camera
	if (!WasRecentlyRendered(0.5f)) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	UCineCameraComponent* ActiveCam = nullptr;
	AActor* ViewTarget = PC->GetViewTarget();
	if (AActor* VT = ViewTarget)
	{
		ActiveCam = VT->FindComponentByClass<UCineCameraComponent>();
	}
	if (ActiveCam)
	{
		const FVector MyLoc = GetActorLocation();
		FVector TargetLoc = ActiveCam->GetComponentLocation();
		TargetLoc.Z = MyLoc.Z;
		
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

// Fwd/Bkwd
void ARunner::CameraControl_GetLength_Implementation()
{
	if (!SettingsSubsystem) return;
	ArmLength = SettingsSubsystem->GetArmLength();
	SpringArmComponent->TargetArmLength = ArmLength;
	SpringArmComponent->bDoCollisionTest = false;
}

void ARunner::CameraControl_Fwd_Implementation(float Value)
{
	
	ArmLength += FMath::Clamp(Value, -1.0f, 1.0f) * 1000.f;
	SpringArmComponent->TargetArmLength = ArmLength;
}

void ARunner::UpdateLength(float NewLength)
{
	SpringArmComponent->TargetArmLength += (NewLength * 1000.f);
}

void ARunner::CameraControl_SaveFwd_Implementation()
{
	SettingsSubsystem->SetArmLength(ArmLength);
	SpringArmComponent->bDoCollisionTest = true;
}

// Pitch
void ARunner::CameraControl_GetPitch_Implementation()
{
	if (!SettingsSubsystem) return;
	Pitch = SettingsSubsystem->GetCameraPitch();
	FRotator CameraRot = FRotator(Pitch, 180.0f, 0.0f);
	SpringArmComponent->SetRelativeRotation(CameraRot);
}

void ARunner::CameraControl_Pitch_Implementation(float Value)
{
	Pitch += FMath::Clamp(Value, -1.0f, 1.0f);
	FRotator CameraRot = FRotator(Pitch, 180.0f, 0.0f);
	SpringArmComponent->SetRelativeRotation(CameraRot);
}

void ARunner::UpdatePitch(float NewPitch)
{
	Pitch += NewPitch;
	FRotator CameraRot = FRotator(Pitch, 180.0f, 0.0f);
	SpringArmComponent->SetRelativeRotation(CameraRot);
}
void ARunner::CameraControl_SavePitch_Implementation()
{
	SettingsSubsystem->SetCameraPitch(Pitch);
}

// Height
void ARunner::CameraControl_GetHeight_Implementation()
{
	if (!SettingsSubsystem) return;
	Height = SettingsSubsystem->GetZAnchor();
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, Height));
}

void ARunner::CameraControl_Height_Implementation(float Value)
{
	SpringArmComponent->bDoCollisionTest = false;
	Height += FMath::Clamp(Value, -1.0f, 1.0f) * 10.f;
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, Height));
}
void ARunner::CameraControl_SaveHeight_Implementation()
{
	SpringArmComponent->bDoCollisionTest = true;
	SettingsSubsystem->SetZAnchor(Height);
}

void ARunner::UpdateZAnchor(float NewZ)
{
	SpringArmComponent->bDoCollisionTest = false;
	Height += FMath::Clamp(NewZ, -1.0f, 1.0f) * 10.f;
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, Height));
}

// Reset
void ARunner::CameraControl_Reset_Implementation()
{
	SpringArmComponent->TargetArmLength = 500000.f;
	SpringArmComponent->SetRelativeRotation(FRotator(15.f, 180.f, 0.f));
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, 500.f));
}


void ARunner::ToggleClub(bool bDisplay)
{
	bShowClub = bDisplay;
}
bool ARunner::IsClubVisible()
{
	return bShowClub;
}

void ARunner::TogglePhoto(bool bDisplay)
{
	bShowPhoto = bDisplay;
}

bool ARunner::IsPhotoVisible()
{
	return bShowPhoto;
}

void ARunner::ToggleFlag(bool bDisplay)
{
	bShowFlag = bDisplay;
}

bool ARunner::IsFlagVisible()
{
	return bShowFlag;
}