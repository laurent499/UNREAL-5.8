// All Rights Reserved


#include "TrailSimulator/Public/Actors/Poi.h"

#include "BroadcastCaptureSubsystem.h"
#include "PoiSubsystem.h"
#include "CesiumFlyToComponent.h"
#include "CesiumGeoreference.h"
#include "CesiumGlobeAnchorComponent.h"
#include "CineCameraComponent.h"
#include "OWLCaptureComponent.h"
#include "Components/RawTextMaterial.h"
#include "SettingsSubsystem.h"
#include "WeatherSubsystem.h"
#include "RaceSubsystem.h"
#include "ScaleSubsystem.h"
#include "TrailInputModeInterface.h"
#include "WorldUtils.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

#define ECC_CesiumChannel ECC_GameTraceChannel1

void APoi::GetBoundsLocation(USceneComponent* Comp, USplineComponent* SplineSupport, FVector& StartLocation, FVector& EndLocation)
{
	Comp->UpdateBounds();
		
	const FBoxSphereBounds LB = Comp->GetLocalBounds();
	const FVector LocalMin = LB.Origin - LB.BoxExtent;
	const FVector LocalMax = LB.Origin + LB.BoxExtent;

	// Largeur locale sur l’axe Y (gauche/droite chez toi)
	float MinY = LocalMin.Y - 20.f;
	float MaxY = LocalMax.Y + 20.f;
		
	const float ScaleY = FMath::Abs(Comp->GetComponentScale().Y);
	float WidthWorld = (MaxY - MinY) * ScaleY;

	constexpr float MinWidthWorld = 30000.f;
	// constexpr float FallbackWidthWorld = 30000.f;

	if (WidthWorld < MinWidthWorld)
	{
		const float HalfFallbackLocal = (0.5f * MinWidthWorld) / FMath::Max(ScaleY, KINDA_SMALL_NUMBER);
		MinY = -HalfFallbackLocal;
		MaxY = +HalfFallbackLocal;
	}

	const FVector Start_TextLocal(0.f, MinY, 0.f);
	const FVector End_TextLocal  (0.f, MaxY, 0.f);

	// Text local -> World
	const FTransform TextXf = Comp->GetComponentTransform();
	const FVector Start_World = TextXf.TransformPosition(Start_TextLocal);
	const FVector End_World   = TextXf.TransformPosition(End_TextLocal);

	// World -> Spline local (car SplineHook != Text)
	const FTransform SplineXf = SplineSupport->GetComponentTransform();
	const FVector Start_SplineLocal = SplineXf.InverseTransformPosition(Start_World);
	const FVector End_SplineLocal   = SplineXf.InverseTransformPosition(End_World);
		
	StartLocation = FVector(0.f, Start_SplineLocal.Y, 0.f);
	EndLocation = FVector(0.f, End_SplineLocal.Y, 0.f);
}

/**
 * @brief Poi Constructor
 */
APoi::APoi()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetActorEnableCollision(false);
	
	if (GetGameInstance())
	{
		PoiSubsystem = GetGameInstance()->GetSubsystem<UPoiSubsystem>();
		WeatherSubsystem = GetGameInstance()->GetSubsystem<UWeatherSubsystem>();
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
		SettingsSubsystem = GetGameInstance()->GetSubsystem<USettingsSubsystem>();
	}
	
	MainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MainRoot"));
	MainRoot->SetComponentTickEnabled(false);
	SetRootComponent(MainRoot);
	
	// Foot	
	FootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FootComponent"));
	FootComponent->SetupAttachment(MainRoot);
	FootComponent->SetComponentTickEnabled(false);
	FootComponent->SetGenerateOverlapEvents(false);
	FootComponent->SetRelativeTransform(FTransform::Identity);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FootMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/Foot.Foot"));
	if (FootMesh.Succeeded())
	{
		FootComponent->SetStaticMesh(FootMesh.Object);
	}
	
	FootHook = CreateDefaultSubobject<USceneComponent>(TEXT("FootHook"));
	FootHook->SetupAttachment(FootComponent);
	FootHook->SetComponentTickEnabled(false);
	FootHook->SetRelativeLocation(FVector(0.f, 0.f, 753.f));
	
	// Orbit
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(FootHook);
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
	
	// Main picto
	MainPicto = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainPicto"));
	MainPicto->SetupAttachment(FootHook);
	MainPicto->SetComponentTickEnabled(false);
	MainPicto->SetGenerateOverlapEvents(false);
	MainPicto->SetRelativeTransform(FTransform::Identity);
	MainPicto->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	MainPicto->SetRelativeScale3D(FVector(0.2f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MainPictoMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newChk/MainPicto.MainPicto"));
	if (MainPictoMesh.Succeeded())
	{
		MainPicto->SetStaticMesh(MainPictoMesh.Object);
	}
	
	// Name
	NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameTextComponent"));
	NameText->SetupAttachment(FootHook);
	NameText->SetComponentTickEnabled(false);
	NameText->SetGenerateOverlapEvents(false);
	NameText->SetRelativeTransform(FTransform::Identity);
	NameText->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	NameText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	NameText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	NameText->SetWorldSize(90.f);
	NameText->SetCastShadow(false);
	NameText->bReceivesDecals = false;
	NameText->bRenderInMainPass = true;
	NameText->bRenderCustomDepth = false;
	NameText->bRenderInDepthPass = false;
	
	SplineHook = CreateDefaultSubobject<USceneComponent>(TEXT("SplineHook"));
	SplineHook->SetupAttachment(FootHook);
	SplineHook->SetComponentTickEnabled(false);
	SplineHook->SetRelativeTransform(FTransform::Identity);
	SplineHook->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(SplineHook);
	SplineComponent->SetComponentTickEnabled(false);
	SplineComponent->SetGenerateOverlapEvents(false);
	SplineComponent->SetRelativeTransform(FTransform::Identity);
	
	NameBkgComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("NameBkgComponent"));
	NameBkgComponent->SetupAttachment(SplineComponent);
	NameBkgComponent->SetComponentTickEnabled(false);
	NameBkgComponent->SetGenerateOverlapEvents(false);
	NameBkgComponent->SetMobility(EComponentMobility::Movable);
	
	// Infos
	InfosBkgComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("InfosBkgComponent"));
	InfosBkgComponent->SetupAttachment(SplineComponent);
	InfosBkgComponent->SetComponentTickEnabled(false);
	InfosBkgComponent->SetGenerateOverlapEvents(false);
	InfosBkgComponent->SetMobility(EComponentMobility::Movable);
	InfosBkgComponent->SetRelativeScale3D(FVector(0.2f, 1.f, 1.f));
	
	AltText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("AltTextComponent"));
	AltText->SetupAttachment(FootHook);
	AltText->SetComponentTickEnabled(false);
	AltText->SetGenerateOverlapEvents(false);
	AltText->SetMobility(EComponentMobility::Movable);
	AltText->SetRelativeTransform(FTransform::Identity);
	AltText->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	AltText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	AltText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	AltText->SetWorldSize(55.f);
	AltText->SetCastShadow(false);
	AltText->bReceivesDecals = false;
	AltText->bRenderInMainPass = true;
	AltText->bRenderCustomDepth = false;
	AltText->bRenderInDepthPass = false;
	
	AltPicto = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltPicto"));
	AltPicto->SetupAttachment(FootHook);
	AltPicto->SetComponentTickEnabled(false);
	AltPicto->SetGenerateOverlapEvents(false);
	AltPicto->SetRelativeTransform(FTransform::Identity);
	AltPicto->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	AltPicto->SetRelativeScale3D(FVector(0.2f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> AltPictoMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newChk/SmallPicto.SmallPicto"));
	if (AltPictoMesh.Succeeded())
	{
		AltPicto->SetStaticMesh(AltPictoMesh.Object);
	}
		
	// Line1
	Line1Component = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Line1Component"));
	Line1Component->SetupAttachment(SplineComponent);
	Line1Component->SetComponentTickEnabled(false);
	Line1Component->SetGenerateOverlapEvents(false);
	Line1Component->SetMobility(EComponentMobility::Movable);
	
	// WeatherBkg
	WeatherBkgComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("WeatherBkgComponent"));
	WeatherBkgComponent->SetupAttachment(SplineComponent);
	WeatherBkgComponent->SetComponentTickEnabled(false);
	WeatherBkgComponent->SetGenerateOverlapEvents(false);
	WeatherBkgComponent->SetMobility(EComponentMobility::Movable);
	WeatherBkgComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	WeatherBkgComponent->SetRelativeScale3D(FVector(0.2f, 1.f, 1.f));
	WeatherBkgComponent->SetHiddenInGame(true);
	
	WeatherPicto = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeatherPicto"));
	WeatherPicto->SetupAttachment(FootHook);
	WeatherPicto->SetComponentTickEnabled(false);
	WeatherPicto->SetGenerateOverlapEvents(false);
	WeatherPicto->SetMobility(EComponentMobility::Movable);
	WeatherPicto->SetRelativeTransform(FTransform::Identity);
	WeatherPicto->SetRelativeLocation(FVector(6.f, 289.f, -187.f));
	WeatherPicto->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	WeatherPicto->SetRelativeScale3D(FVector(0.15f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WeatherMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newChk/SmallPicto.SmallPicto"));
	if (WeatherMesh.Succeeded())
	{
		WeatherPicto->SetStaticMesh(WeatherMesh.Object);
	}
	WeatherPicto->SetHiddenInGame(true);;
	
	TempText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TempTextComponent"));
	TempText->SetupAttachment(FootHook);
	TempText->SetComponentTickEnabled(false);
	TempText->SetGenerateOverlapEvents(false);
	TempText->SetMobility(EComponentMobility::Movable);
	TempText->SetRelativeTransform(FTransform::Identity);
	TempText->SetRelativeLocation(FVector(6.f, 45.f, -190.f));
	TempText->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	TempText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TempText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	TempText->SetWorldSize(55.f);
	TempText->SetHorizontalAlignment(EHTA_Right);
	TempText->SetCastShadow(false);
	TempText->bReceivesDecals = false;
	TempText->bRenderInMainPass = true;
	TempText->bRenderCustomDepth = false;
	TempText->bRenderInDepthPass = false;
	TempText->SetHiddenInGame(true);
	
	Weather1Text = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Weather1TextComponent"));
	Weather1Text->SetupAttachment(FootHook);
	Weather1Text->SetComponentTickEnabled(false);
	Weather1Text->SetGenerateOverlapEvents(false);
	Weather1Text->SetMobility(EComponentMobility::Movable);
	Weather1Text->SetRelativeTransform(FTransform::Identity);
	Weather1Text->SetRelativeLocation(FVector(6.f, -90.f, -150.f));
	Weather1Text->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	Weather1Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	Weather1Text->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	Weather1Text->SetWorldSize(35.f);
	Weather1Text->SetHorizontalAlignment(EHTA_Right);
	Weather1Text->SetCastShadow(false);
	Weather1Text->bReceivesDecals = false;
	Weather1Text->bRenderInMainPass = true;
	Weather1Text->bRenderCustomDepth = false;
	Weather1Text->bRenderInDepthPass = false;
	Weather1Text->SetHiddenInGame(true);
	
	Weather2Text = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Weather2TextComponent"));
	Weather2Text->SetupAttachment(FootHook);
	Weather2Text->SetComponentTickEnabled(false);
	Weather2Text->SetGenerateOverlapEvents(false);
	Weather2Text->SetMobility(EComponentMobility::Movable);
	Weather2Text->SetRelativeTransform(FTransform::Identity);
	Weather2Text->SetRelativeLocation(FVector(6.f, -90.f, -185.f));
	Weather2Text->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	Weather2Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	Weather2Text->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	Weather2Text->SetWorldSize(35.f);
	Weather2Text->SetHorizontalAlignment(EHTA_Right);
	Weather2Text->SetCastShadow(false);
	Weather2Text->bReceivesDecals = false;
	Weather2Text->bRenderInMainPass = true;
	Weather2Text->bRenderCustomDepth = false;
	Weather2Text->bRenderInDepthPass = false;
	Weather2Text->SetHiddenInGame(true);
	
	Weather3Text = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Weather3TextComponent"));
	Weather3Text->SetupAttachment(FootHook);
	Weather3Text->SetComponentTickEnabled(false);
	Weather3Text->SetGenerateOverlapEvents(false);
	Weather3Text->SetMobility(EComponentMobility::Movable);
	Weather3Text->SetRelativeTransform(FTransform::Identity);
	Weather3Text->SetRelativeLocation(FVector(6.f, -90.f, -222.f));
	Weather3Text->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	Weather3Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	Weather3Text->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	Weather3Text->SetWorldSize(35.f);
	Weather3Text->SetHorizontalAlignment(EHTA_Right);
	Weather3Text->SetCastShadow(false);
	Weather3Text->bReceivesDecals = false;
	Weather3Text->bRenderInMainPass = true;
	Weather3Text->bRenderCustomDepth = false;
	Weather3Text->bRenderInDepthPass = false;
	Weather3Text->SetHiddenInGame(true);
	
	// Line2
	Line2Component = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Line2Component"));
	Line2Component->SetupAttachment(SplineComponent);
	Line2Component->SetComponentTickEnabled(false);
	Line2Component->SetGenerateOverlapEvents(false);
	Line2Component->SetMobility(EComponentMobility::Movable);
	Line2Component->SetRelativeLocation(FVector(0.f, 0.f, -244.f));
	Line2Component->SetHiddenInGame(true);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Plain.Bkg_Plain"));
	if (NameMesh.Succeeded())
	{
		NameBkgComponent->SetStaticMesh(NameMesh.Object);
		InfosBkgComponent->SetStaticMesh(NameMesh.Object);
		Line1Component->SetStaticMesh(NameMesh.Object);
		WeatherBkgComponent->SetStaticMesh(NameMesh.Object);
		Line2Component->SetStaticMesh(NameMesh.Object);
	}
	
	GlobeAnchorComponent = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchorComponent"));
	GlobeAnchorComponent->SetAdjustOrientationForGlobeWhenMoving(true);
	Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	GlobeAnchorComponent->SetGeoreference(Georeference);
}

void APoi::SetBroadcastCaptureEnabled_Implementation(bool bEnabled, class UTextureRenderTarget2D* SharedRT)
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

void APoi::BeginPlay()
{
	Super::BeginPlay();
	// Les panneaux masquent les textes des runners (M_RawText compare sa profondeur a la CustomDepth)
	TrailRawText::EnableOcclusion(this);
	TrailRawText::ApplyToAllTexts(this);
	if (FootComponent)
	{
		FootComponent->SetTranslucentSortPriority(-10); // poteau dessine avant les panneaux
	}
	DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	UCesiumFlyToComponent* FlyToComponent = DynaPawn->GetComponentByClass<UCesiumFlyToComponent>();
	FlyToComponent->OnFlightComplete.AddDynamic(this, &APoi::UpdateGlobeAnchor);
	
	if (UWorld* World = GetWorld())
	{
		BroadCastSubsystem = FWorldUtils::GetWorldSubsystemOrLog<UBroadcastCaptureSubsystem>(this, TEXT(__FUNCTION__), false);
		ScaleSubsystem = World->GetSubsystem<UScaleSubsystem>();
		if (ScaleSubsystem)
		{
			ScaleSubsystem->RegisterScalableActor(this);
		}
	}
	
	SettingsSubsystem = FWorldUtils::GetGISubsystemOrLog<USettingsSubsystem>(this, TEXT(__FUNCTION__), false);
	SettingsSubsystem->OnMinMaxSet.AddDynamic(this, &APoi::SetMinMax);
	
	// LookAt
	GetWorldTimerManager().SetTimer(
		LookAtTimerHandle,
		this,
		&APoi::OnLookAtTimerTick,
		SettingsSubsystem->GetUpdateIntervalSeconds(),
		true,
		// premier declenchement aleatoire : les acteurs ne tournent pas tous sur la meme image
		FMath::FRandRange(0.f, SettingsSubsystem->GetUpdateIntervalSeconds())
	);
	
	SpringArmComponent->SetUsingAbsoluteRotation(false);
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, SettingsSubsystem->GetZAnchor()));
	SpringArmComponent->TargetArmLength = SettingsSubsystem->GetArmLength();
	SpringArmComponent->SetRelativeRotation(FRotator(SettingsSubsystem->GetCameraPitch(), 180.f, 0.f));
	InitLocation = FootHook->GetRelativeLocation();
}

void APoi::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LookAtTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APoi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bOrbitEnabled) return;
	AddActorWorldRotation(FRotator(0.f, OrbitSpeedDegPerSec * DeltaTime, 0.f));
}

/**
 * @brief Capitalizing the first letter of a string
 * @param In 
 * @return 
 */
FString APoi::CapitalizeFirst(const FString& In)
{
	if (In.IsEmpty())
		return In;

	FString Out = In;
	Out[0] = FChar::ToUpper(Out[0]);
	return Out;
}

/**
 * @brief Aligning the Globe Anchor 
 */
void APoi::UpdateGlobeAnchor()
{
	GlobeAnchorComponent->SnapLocalUpToEllipsoidNormal();
}

/**
 * @brief Updating the Poi visuals & datas
 * @param NewPoiDatas 
 * @param NewRaceSetup 
 */
void APoi::UpdatePoi(FRacePOI NewPoiDatas, FRaceSetup NewRaceSetup)
{
	PoiDatas = MoveTemp(NewPoiDatas);
	TrailRawText::EnableOcclusion(this);
	Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	GlobeAnchorComponent->SetGeoreference(Georeference);
	
	ScaleSubsystem->SetActorScaleMinMax(this, SettingsSubsystem->GetMinMaxById(NewRaceSetup.raceId));
	// Main Picto
	MainPicto->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	
	// Name
	if (NameText)
	{
		NameText->SetText(FText::FromString(PoiDatas.name));
		NameText->SetRelativeLocation(FVector(5.0f, 0.f, 60.f));
		NameText->UpdateBounds();
		
		FVector StartLocation, EndLocation;
		GetBoundsLocation(NameText, SplineComponent, StartLocation, EndLocation);
		
		SplineComponent->SetMobility(EComponentMobility::Movable);
		SplineComponent->ClearSplinePoints();
		SplineComponent->AddSplinePointAtIndex(StartLocation, 0, ESplineCoordinateSpace::Local, false);
		SplineComponent->AddSplinePointAtIndex(EndLocation, 1, ESplineCoordinateSpace::Local, false);
		SplineComponent->SetSplinePointType(0, ESplinePointType::Linear, false);
		SplineComponent->SetSplinePointType(1, ESplinePointType::Linear, true);
		
		NameBkgComponent->SetMobility(EComponentMobility::Movable);
		NameBkgComponent->SetStartAndEnd(
			StartLocation, 
			SplineComponent->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local),
			EndLocation,
			SplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local));
		NameBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
		NameBkgComponent->SetStartScale(FVector2D(1.0f, 1.0f));
		NameBkgComponent->SetEndScale(FVector2D(1.0f, 1.0f));
		NameBkgComponent->UpdateMesh();
		NameBkgComponent->SetRelativeScale3D(FVector(0.2f, 1.f, 1.f));
	}
	
	// Infos
	if (InfosBkgComponent)
	{
		NameText->UpdateBounds();
		FVector StartLocation, EndLocation;
		GetBoundsLocation(NameText, SplineComponent, StartLocation, EndLocation);
		
		const FTransform SplineXf = SplineComponent->GetComponentTransform();
		const FTransform MeshXf   = InfosBkgComponent->GetComponentTransform();

		// spline local -> world
		const FVector StartW = SplineXf.TransformPosition(StartLocation);
		const FVector EndW   = SplineXf.TransformPosition(EndLocation);

		// world -> mesh local
		const FVector StartL = MeshXf.InverseTransformPosition(StartW);
		const FVector EndL   = MeshXf.InverseTransformPosition(EndW);

		// world -> mesh local (vecteur)
		const FVector DirL = (EndL - StartL);
		const float TangentLen = DirL.Size();
		const FVector TanL = DirL.GetSafeNormal() * TangentLen;

		InfosBkgComponent->SetMobility(EComponentMobility::Movable);
		InfosBkgComponent->SetStartAndEnd(StartL, TanL, EndL, TanL);
		
		InfosBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
		InfosBkgComponent->SetStartScale(FVector2D(1.0f, 1.0f));
		InfosBkgComponent->SetEndScale(FVector2D(1.0f, 1.0f));
		InfosBkgComponent->UpdateMesh();
		InfosBkgComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -120.f));
	}
	
	// Line1
	if (Line1Component)
	{
		NameText->UpdateBounds();
		FVector StartLocation, EndLocation;
		GetBoundsLocation(NameText, SplineComponent, StartLocation, EndLocation);
		
		const FTransform SplineXf = SplineComponent->GetComponentTransform();
		const FTransform MeshXf   = Line1Component->GetComponentTransform();

		// spline local -> world
		const FVector StartW = SplineXf.TransformPosition(StartLocation);
		const FVector EndW   = SplineXf.TransformPosition(EndLocation);

		// world -> mesh local
		const FVector StartL = MeshXf.InverseTransformPosition(StartW);
		const FVector EndL   = MeshXf.InverseTransformPosition(EndW);
		
		// world -> mesh local (vecteur)
		const FVector DirL = (EndL - StartL);
		const float TangentLen = DirL.Size(); 
		const FVector TanL = DirL.GetSafeNormal() * TangentLen;
		
		Line1Component->SetMobility(EComponentMobility::Movable);
		Line1Component->SetStartAndEnd(StartL, TanL, EndL, TanL);
		Line1Component->SetForwardAxis(ESplineMeshAxis::X);
		Line1Component->SetStartScale(FVector2D(0.2f, 0.05f));
		Line1Component->SetEndScale(FVector2D(0.2f, 0.05f));
		Line1Component->UpdateMesh();
		Line1Component->SetRelativeLocation(FVector(0.0f, 0.0f, -126.f));
	}
	
	if (AltText)
	{
		AltText->SetMobility(EComponentMobility::Movable);
		AltText->SetText(FText::FromString(FString::Printf(TEXT("%.0f m"), PoiDatas.elevation)));
		AltText->SetTextRenderColor(FColor(0.,0,0,255));
		AltText->SetRelativeLocation(FVector(6.f, -50.f, -60.f));
	}
	
	if (AltPicto)
	{
		AltPicto->SetRelativeLocation(FVector(5.f, 190.f, -51.f));
	}
	
	// Line2
	if (Line2Component)
	{
		NameText->UpdateBounds();
		FVector StartLocation, EndLocation;
		GetBoundsLocation(NameText, SplineComponent, StartLocation, EndLocation);
		
		const FTransform SplineXf = SplineComponent->GetComponentTransform();
		const FTransform MeshXf   = Line2Component->GetComponentTransform();

		// spline local -> world
		const FVector StartW = SplineXf.TransformPosition(StartLocation);
		const FVector EndW   = SplineXf.TransformPosition(EndLocation);

		// world -> mesh local
		const FVector StartL = MeshXf.InverseTransformPosition(StartW);
		const FVector EndL   = MeshXf.InverseTransformPosition(EndW);

		// world -> mesh local (vecteur)
		const FVector DirL = (EndL - StartL);
		const float TangentLen = DirL.Size();
		const FVector TanL = DirL.GetSafeNormal() * TangentLen;

		Line2Component->SetMobility(EComponentMobility::Movable);
		Line2Component->SetStartAndEnd(StartL, TanL, EndL, TanL);
		Line2Component->SetForwardAxis(ESplineMeshAxis::X);
		Line2Component->SetStartScale(FVector2D(0.2f, 0.05f));
		Line2Component->SetEndScale(FVector2D(0.2f, 0.05f));
		Line2Component->UpdateMesh();
		Line2Component->SetRelativeLocation(FVector(0.0f, 0.0f, -252.f));
	}
		
	if (WeatherBkgComponent)
	{
		NameText->UpdateBounds();
		FVector StartLocation, EndLocation;
		GetBoundsLocation(NameText, SplineComponent, StartLocation, EndLocation);
		
		const FTransform SplineXf = SplineComponent->GetComponentTransform();
		const FTransform MeshXf   = WeatherBkgComponent->GetComponentTransform();

		// spline local -> world
		const FVector StartW = SplineXf.TransformPosition(StartLocation);
		const FVector EndW   = SplineXf.TransformPosition(EndLocation);

		// world -> mesh local
		const FVector StartL = MeshXf.InverseTransformPosition(StartW);
		const FVector EndL   = MeshXf.InverseTransformPosition(EndW);
		
		// world -> mesh local (vecteur)
		const FVector DirL = (EndL - StartL);
		const float TangentLen = DirL.Size(); 
		const FVector TanL = DirL.GetSafeNormal() * TangentLen;

		WeatherBkgComponent->SetMobility(EComponentMobility::Movable);
		WeatherBkgComponent->SetStartAndEnd(StartL, TanL, EndL, TanL);
		WeatherBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
		WeatherBkgComponent->SetStartScale(FVector2D(1.0f, 1.0f));
		WeatherBkgComponent->SetEndScale(FVector2D(1.0f, 1.0f));
		WeatherBkgComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -246.f));
		WeatherBkgComponent->UpdateMesh();
	}
	
	if (TempText)
	{
		TempText->SetWorldSize(55.f);
		TempText->SetTextRenderColor(FColor(0,0,0,255));
	}
	if (Weather1Text)
	{
		Weather1Text->SetWorldSize(35.f);
		Weather1Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
		Weather1Text->SetTextRenderColor(FColor(0,0,0,255));
	}
	if (Weather2Text)
	{
		Weather2Text->SetWorldSize(35.f);
		Weather2Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
		Weather2Text->SetTextRenderColor(FColor(0,0,0,255));
	}
	if (Weather3Text)
	{
		Weather3Text->SetWorldSize(35.f);
		Weather3Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
		Weather3Text->SetTextRenderColor(FColor(0,0,0,255));
	}
}

/**
 * @brief Updating the Weather visuals & datas
 * @param WeatherDatas 
 */
void APoi::UpdatePoiWeather(FOpenWeatherResponse WeatherDatas)
{
	PoiWeatherDatas = MoveTemp(WeatherDatas);
	const FString Type = PoiWeatherDatas.current.weather[0].icon;
	const FString PictoPath = FString::Printf(
		TEXT("/Game/LTVContent/2D/Pictos/WEATHER/%s.%s"), *Type, *Type);
		
	UTexture2D* Tex = LoadObject<UTexture2D>(
	nullptr,
	*PictoPath);
		
	UMaterialInterface* BaseMat = WeatherPicto->GetMaterial(0);
	UMaterialInstanceDynamic* MID = WeatherPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMat);
	if (!MID) return;
	
	MID->SetTextureParameterValue(TEXT("Photo"), Tex);
	
	TempText->SetText(FText::FromString(FString::Printf(TEXT("%.2f °C"), PoiWeatherDatas.current.temp)));
	Weather1Text->SetText(FText::FromString(FString::Printf(TEXT("Feels like %.2f °C"), PoiWeatherDatas.current.feels_like)));
	
	Weather2Text->SetText(FText::FromString(FString::Printf(TEXT("%s"), *CapitalizeFirst(PoiWeatherDatas.current.weather[0].description))));	
	Weather3Text->SetText(FText::FromString(FString::Printf(TEXT("Humidity %d %%"), PoiWeatherDatas.current.humidity)));	
}

/**
 * @brief Show/Hide Weather components & requesting Weather datas
 * @param bShow 
 */
void APoi::TogglePoiWeather(bool bShow)
{
	if (WeatherBkgComponent && Weather1Text && Weather2Text && Weather3Text && WeatherPicto)
	{
		bShowWeather = bShow;
		WeatherBkgComponent->SetHiddenInGame(!bShow);
		Weather1Text->SetHiddenInGame(!bShow);
		Weather2Text->SetHiddenInGame(!bShow);
		Weather3Text->SetHiddenInGame(!bShow);
		WeatherPicto->SetHiddenInGame(!bShow);
		Line2Component->SetHiddenInGame(!bShow);
		TempText->SetHiddenInGame(!bShow);
		FootHook->SetRelativeLocation(
		bShow ? FVector(InitLocation.X, InitLocation.Y, 880.f) : InitLocation );
		if (bShow)
		{
			FetchWeather();
			GetWorldTimerManager().SetTimer(
				WeatherHandle,
				this,
				&APoi::FetchWeather,
				SettingsSubsystem->GetWeatherFrequency(),
				true
			);
		} else
		{
			GetWorldTimerManager().ClearTimer(WeatherHandle);
			WeatherHandle.Invalidate();
		}
	}
}

void APoi::UpdateDayNight(bool bIsDay)
{
	if (bIsDay)
	{
		NameText->SetCustomPrimitiveDataFloat(0, 0.125);
		AltText->SetCustomPrimitiveDataFloat(0, 0.125);
		Weather1Text->SetCustomPrimitiveDataFloat(0, 0.125);
		Weather2Text->SetCustomPrimitiveDataFloat(0, 0.125);
		Weather3Text->SetCustomPrimitiveDataFloat(0, 0.125);
	} else
	{
		NameText->SetCustomPrimitiveDataFloat(0, 0.25);
		AltText->SetCustomPrimitiveDataFloat(0, 0.25);
		Weather1Text->SetCustomPrimitiveDataFloat(0, 0.25);
		Weather2Text->SetCustomPrimitiveDataFloat(0, 0.25);
		Weather3Text->SetCustomPrimitiveDataFloat(0, 0.25);
	}
}

/**
 * @brief Fetching Weather Datas
 */
void APoi::FetchWeather()
{
	const FString Key = PoiDatas.name;
	WeatherSubsystem->PerformHttpRequestForWeather(
		Key,
		PoiDatas.weather,
		[WeakThis = TWeakObjectPtr<APoi>(this)](FWeatherResult&& R)
		{
			if (!WeakThis.IsValid()) return;

			if (!R.bSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Weather failed: %s"), *R.RequestKey, *R.Error);
				return;
			}
			WeakThis->UpdatePoiWeather(R.Data);
		}
	);
}

/**
 * @brief Playing orbit animation
 * @param RotationSpeed 
 */
void APoi::StartAnimation(float RotationSpeed)
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

/**
 * @brief Stop orbit animation
 */
void APoi::StopAnimation()
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
			// PC->SetViewTargetWithBlend(OriginalViewTarget, 1.f);
			BroadCastSubsystem->RequestViewTarget(PC, OriginalViewTarget, 1.f);
			OriginalViewTarget = nullptr;
		}
	}
	GetWorldTimerManager().ClearTimer(RotHandle);
	RotHandle.Invalidate();
	
	bLookAtEnabled = true;
	GetWorldTimerManager().SetTimer(
		LookAtTimerHandle,
		this,
		&APoi::OnLookAtTimerTick,
		SettingsSubsystem->GetUpdateIntervalSeconds(),
		true,
		// premier declenchement aleatoire : les acteurs ne tournent pas tous sur la meme image
		FMath::FRandRange(0.f, SettingsSubsystem->GetUpdateIntervalSeconds())
	);
	SetActorTickEnabled(false);
	bOrbitEnabled = false;
	PrimaryActorTick.SetTickFunctionEnable(false);
}

/**
 * @brief Updating the MinMax
 * @param MinValue
 * @param MaxValue
 * @param RaceID 
 */
void APoi::SetMinMax(float MinValue, float MaxValue, int64 RaceID)
{
	if (PoiSubsystem->DoesPoiBelongsToRace(PoiDatas.poiId, RaceID))
	{
		MinMax = FMinMax(FVector(MinValue), FVector(MaxValue));
		ScaleSubsystem->RegisterScalableActor(this);
		ScaleSubsystem->SetActorScaleMinMax(this, MinMax);
	}
}
FMinMax APoi::GetMinMax() const
{
	return MinMax;
}

/**
 * @brief LookAt tick
 */
void APoi::OnLookAtTimerTick()
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

void APoi::UpdatePitch(float NewPitch)
{
	float CameraPitch = SettingsSubsystem->GetCameraPitch();
	FRotator CameraRot = FRotator(CameraPitch, 180.0f, 0.0f);
	SpringArmComponent->SetRelativeRotation(CameraRot);
}

void APoi::UpdateLength(float NewLength)
{
	SpringArmComponent->TargetArmLength = NewLength;
}

void APoi::UpdateZAnchor(float NewZ)
{
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, NewZ));
}

// Fwd/Bkwd
void APoi::CameraControl_GetLength_Implementation()
{
	if (!SettingsSubsystem) return;
	ArmLength = SettingsSubsystem->GetArmLength();
	SpringArmComponent->TargetArmLength = ArmLength;
	SpringArmComponent->bDoCollisionTest = false;
}

void APoi::CameraControl_Fwd_Implementation(float Value)
{
	
	ArmLength += FMath::Clamp(Value, -1.0f, 1.0f) * 1000.f;
	SpringArmComponent->TargetArmLength = ArmLength;
}

void APoi::CameraControl_SaveFwd_Implementation()
{
	SettingsSubsystem->SetArmLength(ArmLength);
	SpringArmComponent->bDoCollisionTest = true;
}

// Pitch
void APoi::CameraControl_GetPitch_Implementation()
{
	if (!SettingsSubsystem) return;
	Pitch = SettingsSubsystem->GetCameraPitch();
	FRotator CameraRot = FRotator(Pitch, 180.0f, 0.0f);
	SpringArmComponent->SetRelativeRotation(CameraRot);
}

void APoi::CameraControl_Pitch_Implementation(float Value)
{
	Pitch += FMath::Clamp(Value, -1.0f, 1.0f);
	FRotator CameraRot = FRotator(Pitch, 180.0f, 0.0f);
	SpringArmComponent->SetRelativeRotation(CameraRot);
}

void APoi::CameraControl_SavePitch_Implementation()
{
	SettingsSubsystem->SetCameraPitch(Pitch);
}

// Height
void APoi::CameraControl_GetHeight_Implementation()
{
	if (!SettingsSubsystem) return;
	Height = SettingsSubsystem->GetZAnchor();
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, Height));
}

void APoi::CameraControl_Height_Implementation(float Value)
{
	SpringArmComponent->bDoCollisionTest = false;
	Height += FMath::Clamp(Value, -1.0f, 1.0f) * 10.f;
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, Height));
}
void APoi::CameraControl_SaveHeight_Implementation()
{
	SpringArmComponent->bDoCollisionTest = true;
	SettingsSubsystem->SetZAnchor(Height);
}

// Reset
void APoi::CameraControl_Reset_Implementation()
{
	SpringArmComponent->TargetArmLength = 500000.f;
	SpringArmComponent->SetRelativeRotation(FRotator(15.f, 180.f, 0.f));
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, 500.f));
}