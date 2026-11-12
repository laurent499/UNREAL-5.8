// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Km.h"
#include "CesiumFlyToComponent.h"
#include "CesiumGlobeAnchorComponent.h"
#include "RaceManager.h"
#include "RaceSubsystem.h"
#include "SettingsSubsystem.h"
#include "TemplateSubsystem.h"
#include "Components/SplineMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/RawTextMaterial.h"
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

	// Fond derrière la distance : dimensionné sur le texte dans ApplyKmStyle, masqué d'ici là
	BkgComp = CreateDefaultSubobject<USplineMeshComponent>(TEXT("BkgComponent"));
	BkgComp->SetupAttachment(MainRoot);
	BkgComp->SetMobility(EComponentMobility::Movable);
	BkgComp->SetComponentTickEnabled(false);
	BkgComp->SetGenerateOverlapEvents(false);
	BkgComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BkgComp->SetCastShadow(false);
	BkgComp->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BkgMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Plain.Bkg_Plain"));
	if (BkgMesh.Succeeded())
	{
		BkgComp->SetStaticMesh(BkgMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BkgMaterial(
		TEXT("/Game/LTVContent/Materials/Masters/M_PlainRounded.M_PlainRounded"));
	if (BkgMaterial.Succeeded())
	{
		BkgComp->SetMaterial(0, BkgMaterial.Object);
	}
	
	// Globe anchor
	GlobeAnchorComponent = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchorComponent"));
	GlobeAnchorComponent->SetAdjustOrientationForGlobeWhenMoving(true);
	Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
	GlobeAnchorComponent->SetGeoreference(Georeference);
	
}

void AKm::BeginPlay()
{
	Super::BeginPlay();
	TrailRawText::ApplyToAllTexts(this);
	
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
		true,
		// premier declenchement aleatoire : les acteurs ne tournent pas tous sur la meme image
		FMath::FRandRange(0.f, SettingsSubsystem->GetUpdateIntervalSeconds())
	);
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &AKm::UpdateDayNight);
	}
	// « Appliquer dans Unreal » sur la page Templates : fond et couleurs rejoués en direct
	if (UTemplateSubsystem* Templates = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTemplateSubsystem>() : nullptr)
	{
		TemplateChangedHandle = Templates->OnTemplateChanged.AddUObject(this, &AKm::HandleTemplateChanged);
	}
}

void AKm::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTemplateSubsystem* Templates = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTemplateSubsystem>() : nullptr)
	{
		Templates->OnTemplateChanged.Remove(TemplateChangedHandle);
	}
	GetWorldTimerManager().ClearTimer(LookAtTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AKm::HandleTemplateChanged(const FString& TemplateName)
{
	if (KmRaceID == 0 || !RaceSubsystem || RaceSubsystem->GetRaceSetupById(KmRaceID).templateName != TemplateName)
	{
		return;
	}
	ApplyKmStyle();
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

	KmRaceID = RaceID;
	ApplyKmStyle();
}

/**
 * @brief Fond derrière la distance et couleur du texte, lus dans la section km du template.
 * @note Appelé à la construction de la borne et au rechargement du template, jamais par image :
 *       un spline mesh statique de plus par borne, sans tick ni ombre.
 *       M_PlainRounded : primitive data 0-3 couleur, 4 éclairage, 5 alpha, 6 arrondi, 7 coins,
 *       8 biseau, 9 hauteur de référence.
 */
void AKm::ApplyKmStyle() const
{
	if (!RaceSubsystem || !LabelComp || !BkgComp)
	{
		return;
	}
	const FRaceSetup& Setup = RaceSubsystem->GetRaceSetupById(KmRaceID);
	const FTrailTemplate Tpl = UTemplateSubsystem::GetForRace(this, Setup);
	const FTrailTemplateKm& Style = Tpl.Km;

	// UTMB et Nike gardent leur habillage codé dans Unreal (pas de template éditable)
	const bool bTemplated = !Tpl.Base.Equals(TEXT("UTMB"), ESearchCase::IgnoreCase)
		&& !Tpl.Base.Equals(TEXT("Nike"), ESearchCase::IgnoreCase);
	if (bTemplated)
	{
		LabelComp->SetTextRenderColor(FTrailTemplate::ToSRGB(Style.Text, Setup.color));
	}

	const FLinearColor BkgColor = FTrailTemplate::ToLinear(Style.Bkg, Setup.color);
	const UStaticMesh* Mesh = BkgComp->GetStaticMesh();
	if (!bTemplated || BkgColor.A < 0.01f || !Mesh || LabelComp->Text.IsEmpty())
	{
		BkgComp->SetVisibility(false);
		return;
	}

	// Forme des angles
	float Radius = 0.f;
	float Slant = 0.f;
	if (Style.Shape.Equals(TEXT("globals"), ESearchCase::IgnoreCase))
	{
		Radius = Tpl.Globals.CornerRadius;
		Slant = Tpl.Globals.Slant;
	}
	else if (Style.Shape.Equals(TEXT("rounded"), ESearchCase::IgnoreCase))
	{
		Radius = Style.CornerRadius;
	}
	else if (Style.Shape.Equals(TEXT("slant"), ESearchCase::IgnoreCase))
	{
		Slant = Style.Slant;
	}
	Slant = FMath::Clamp(Slant, -60.f, 60.f);

	// Bornes du texte dans l'espace de MainRoot (le texte se lit depuis +X)
	const FBox TextBox = LabelComp->CalcBounds(LabelComp->GetRelativeTransform()).GetBox();
	const float TextHeight = TextBox.GetSize().Z;
	const float TextWidth = TextBox.GetSize().Y;
	const FBox MeshBox = Mesh->GetBoundingBox();
	// Bkg_Plain : longueur en X (étirée par le spline), hauteur en Z, épaisseur en Y
	const float MeshHeight = MeshBox.GetSize().Z;
	if (TextHeight <= KINDA_SMALL_NUMBER || TextWidth <= KINDA_SMALL_NUMBER || MeshHeight <= KINDA_SMALL_NUMBER)
	{
		BkgComp->SetVisibility(false);
		return;
	}
	const float Pad = FMath::Max(Style.Padding, 0.f) * TextHeight;
	const float BkgHeight = TextHeight + 2.f * Pad;
	// Le biseau reste dans le fond (M_PlainRounded y perd |tan| x hauteur de chaque côté) : on l'ajoute
	const float SlantInset = FMath::Abs(FMath::Tan(FMath::DegreesToRadians(Slant))) * BkgHeight;
	const float BkgWidth = TextWidth + 2.f * Pad + 2.f * SlantInset;

	const float ScaleZ = BkgHeight / MeshHeight;
	const FVector Center = TextBox.GetCenter();
	const float Depth = FMath::Max(FMath::Abs(MeshBox.Min.Y), FMath::Abs(MeshBox.Max.Y));
	// Derrière le texte, et centré sur lui quelle que soit l'origine du mesh
	const float X = TextBox.Min.X - Depth - 2.f;
	const float Z = Center.Z - MeshBox.GetCenter().Z * ScaleZ;
	const FVector Start(X, Center.Y + 0.5f * BkgWidth, Z);
	const FVector End(X, Center.Y - 0.5f * BkgWidth, Z);
	const FVector Tangent = End - Start;
	BkgComp->SetForwardAxis(ESplineMeshAxis::X, false);
	BkgComp->SetStartScale(FVector2D(1.f, ScaleZ), false);
	BkgComp->SetEndScale(FVector2D(1.f, ScaleZ), false);
	BkgComp->SetStartAndEnd(Start, Tangent, End, Tangent, false);
	BkgComp->UpdateMesh();

	BkgComp->SetCustomPrimitiveDataVector4(0, FLinearColor(BkgColor.R, BkgColor.G, BkgColor.B, 1.f));
	BkgComp->SetCustomPrimitiveDataFloat(4, 0.0625f);
	BkgComp->SetCustomPrimitiveDataFloat(5, BkgColor.A);
	BkgComp->SetCustomPrimitiveDataFloat(6, Radius);
	BkgComp->SetCustomPrimitiveDataFloat(7, 0.f);
	BkgComp->SetCustomPrimitiveDataFloat(8, Slant);
	BkgComp->SetCustomPrimitiveDataFloat(9, 0.f);
	BkgComp->SetVisibility(true);
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
	// Hors champ (ni vue principale ni capture OWL) : inutile de le tourner vers la camera
	if (!WasRecentlyRendered(0.5f)) return;

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