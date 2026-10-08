// All Rights Reserved


#include "Actors/UTMB/Runner_UTMB.h"

#include "RaceManager.h"
#include "Components/PhotoComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/RawTextMaterial.h"
#include "Components/UTMB/UTMB_FlagComponent.h"
#include "Components/UTMB/UTMB_NameComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Color.h"


ARunner_UTMB::ARunner_UTMB()
{	
	NameComponent = CreateDefaultSubobject<UUTMB_NameComponent>(TEXT("UTMBNameComponent"));
	NameComponent->SetupAttachment(NameHook);
	NameComponent->SetComponentTickEnabled(false);
	NameComponent->SetRelativeTransform(FTransform::Identity);
	NameComponent->ShowWidgetName(false);
	NameComponent->Show3DName(true);
	
	// Flag
	FlagHook = CreateDefaultSubobject<USceneComponent>(TEXT("FlagHook"));
	FlagHook->SetupAttachment(NameHook);
	FlagHook->SetComponentTickEnabled(false);
	FlagHook->SetRelativeTransform(FTransform::Identity);
	FlagHook->SetRelativeLocation(FVector(20.f, -80.f, 95.f));
	
	FlagComponent = CreateDefaultSubobject<UUTMB_FlagComponent>(TEXT("FlagComponent"));
	FlagComponent->SetupAttachment(FlagHook);
	FlagComponent->SetComponentTickEnabled(false);
	FlagComponent->SetRelativeTransform(FTransform::Identity);
	FlagComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	
	// Photo
	PhotoHook = CreateDefaultSubobject<USceneComponent>(TEXT("PhotoHook"));
	PhotoHook->SetupAttachment(NameHook);
	PhotoHook->SetComponentTickEnabled(false);
	PhotoHook->SetRelativeTransform(FTransform::Identity);
	PhotoHook->SetRelativeLocation(FVector(0.f, -275.f, 120.f));
	
	PhotoComponent = CreateDefaultSubobject<UPhotoComponent>(TEXT("PhotoComponent"));
	PhotoComponent->SetupAttachment(PhotoHook);
	PhotoComponent->SetComponentTickEnabled(false);
	PhotoComponent->SetGenerateOverlapEvents(false);
	PhotoComponent->SetRelativeTransform(FTransform::Identity);
	PhotoComponent->SetRelativeScale3D(FVector(0.8f));
	
	/** Index */
	// Index Bkg
	IndexBkgHook = CreateDefaultSubobject<USceneComponent>(TEXT("IndexBkgHook"));
	IndexBkgHook->SetupAttachment(NameHook);	
	IndexBkgHook->SetComponentTickEnabled(false);
	IndexBkgHook->SetRelativeTransform(FTransform::Identity);
	IndexBkgHook->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	
	IndexBkgComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndexBkgComponent"));
	IndexBkgComponent->SetupAttachment(IndexBkgHook);
	IndexBkgComponent->SetComponentTickEnabled(false);
	IndexBkgComponent->SetGenerateOverlapEvents(false);
	IndexBkgComponent->SetRelativeTransform(FTransform::Identity);
	IndexBkgComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	IndexBkgComponent->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> IndexMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/UTMB.UTMB"));
	if (IndexMesh.Succeeded())
	{
		IndexBkgComponent->SetStaticMesh(IndexMesh.Object);
	}
	
	// Index Left
	IndexLeftComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndexLeft"));
	IndexLeftComponent->SetupAttachment(IndexBkgHook);
	IndexLeftComponent->SetComponentTickEnabled(false);
	IndexLeftComponent->SetGenerateOverlapEvents(false);
	IndexLeftComponent->SetRelativeTransform(FTransform::Identity);
	IndexLeftComponent->SetRelativeLocation(FVector(0.f, -414.f, -122.f));
	IndexLeftComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> IndexLeftMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/IndexLeft_UTMB.IndexLeft_UTMB"));
	if (IndexLeftMesh.Succeeded())
	{
		IndexLeftComponent->SetStaticMesh(IndexLeftMesh.Object);
	}

	// Index Barre
	IndexBarreComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndexBarreComponent"));
	IndexBarreComponent->SetupAttachment(IndexBkgHook);
	IndexBarreComponent->SetComponentTickEnabled(false);
	IndexBarreComponent->SetGenerateOverlapEvents(false);
	IndexBarreComponent->SetRelativeTransform(FTransform::Identity);
	IndexBarreComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	IndexBarreComponent->SetRelativeLocation(FVector(0.f, -414.f, -122.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> IndexBarreMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/IndexBarre_UTMB.IndexBarre_UTMB"));
	if (IndexBarreMesh.Succeeded())
	{
		IndexBarreComponent->SetStaticMesh(IndexBarreMesh.Object);
	}
	
	// Index Right
	IndexRightComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndexRight"));
	IndexRightComponent->SetupAttachment(IndexBkgHook);
	IndexRightComponent->SetComponentTickEnabled(false);
	IndexRightComponent->SetGenerateOverlapEvents(false);
	IndexRightComponent->SetRelativeTransform(FTransform::Identity);
	IndexRightComponent->SetRelativeLocation(FVector(0.f, -424.f, -122.f));
	IndexRightComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> IndexRightMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/IndexRight_UTMB.IndexRight_UTMB"));
	if (IndexRightMesh.Succeeded())
	{
		IndexRightComponent->SetStaticMesh(IndexRightMesh.Object);
	}
	
	// UTMB Label
	UtmbLabelComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("UtmbLabelComponent"));
	UtmbLabelComponent->SetupAttachment(IndexBkgHook);
	UtmbLabelComponent->SetComponentTickEnabled(false);
	UtmbLabelComponent->SetGenerateOverlapEvents(false);
	UtmbLabelComponent->SetRelativeTransform(FTransform::Identity);
	UtmbLabelComponent->SetText(FText::FromString("UTMB"));
	UtmbLabelComponent->SetWorldSize(65.f);
	UtmbLabelComponent->SetRelativeLocation(FVector(13.f, -30.f, -93.f));
	
	// Index Label
	IndexLabelComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IndexLabelComponent"));
	IndexLabelComponent->SetupAttachment(IndexBkgHook);
	IndexLabelComponent->SetComponentTickEnabled(false);
	IndexLabelComponent->SetGenerateOverlapEvents(false);
	IndexLabelComponent->SetRelativeTransform(FTransform::Identity);
	IndexLabelComponent->SetText(FText::FromString("INDEX"));
	IndexLabelComponent->SetWorldSize(65.f);
	IndexLabelComponent->SetTextRenderColor(FColor(0.f, 13.f, 68.f));
	IndexLabelComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	IndexLabelComponent->SetRelativeLocation(FVector(13.f, -237.f, -116.f));
			
	// Index Value
	IndexValueComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IndexValueComponent"));
	IndexValueComponent->SetupAttachment(IndexBkgHook);
	IndexValueComponent->SetComponentTickEnabled(false);
	IndexValueComponent->SetGenerateOverlapEvents(false);
	IndexValueComponent->SetRelativeTransform(FTransform::Identity);
	IndexValueComponent->SetHorizontalAlignment(EHTA_Center);
	IndexValueComponent->SetWorldSize(65.f);
	IndexValueComponent->SetTextRenderColor(FColor(0.f, 13.f, 68.f));
	IndexValueComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	IndexValueComponent->SetRelativeLocation(FVector(13.f, -525.f, -116.f));
	
}

void ARunner_UTMB::UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup)
{
	Super::UpdateRunner(Runner, RaceSetup);
	
	// Foot
	if (FootComponent)
	{		
		UMaterialInterface* CoreMat = FootComponent->GetMaterial(0);
		UMaterialInstanceDynamic* MID = FootComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, CoreMat);
		MID->SetVectorParameterValue("BaseColor", FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
	}
	
	// Name
	if (NameComponent)
	{
		NameComponent->UpdateName(Runner.nom);
		NameHook->SetRelativeLocation(FVector(
			NameHook->GetRelativeLocation().X,
			NameComponent->GetMiddle(),
			NameHook->GetRelativeLocation().Z));
	}
	
	if (PhotoComponent)
	{
		if (IsPhotoVisible())
		{
			FString NewPhotoUrl = Runner.photo;
			if (!Runner.photo.Contains("jpg") && !Runner.photo.Contains("jpeg") && !Runner.photo.Contains("png") && !Runner.photo.Contains("gif"))
			{
				NewPhotoUrl += ".png";
			}
			PhotoComponent->UpdatePhoto(NewPhotoUrl, FVector(0.75f, 1.0f, 0.75f));
		}
	}
	
	if (FlagComponent && IsFlagVisible() && !Runner.pays.IsEmpty()){
		
		FlagComponent->UpdateFlag(Runner.pays);
	}
	if (IndexValueComponent)
	{
		IndexValueComponent->SetText(FText::FromString(FString::FromInt(Runner.IndexM)));
	}
		// Couleurs brutes, jamais modifiees par l'eclairage, l'exposition ou le post-process
		TrailRawText::Apply(UtmbLabelComponent, FColor::White);
		TrailRawText::Apply(IndexLabelComponent, FColor(0, 13, 68));
		TrailRawText::Apply(IndexValueComponent, FColor(0, 13, 68));
}
void ARunner_UTMB::ToggleFlag(bool bDisplay)
{
	Super::ToggleFlag(bDisplay);
	if (FlagComponent)
	{
		FlagComponent->ToggleFlag(bDisplay);
	}
}

void ARunner_UTMB::TogglePhoto(bool bDisplay)
{
	Super::TogglePhoto(bDisplay);
	if (PhotoComponent)
	{
		PhotoComponent->TogglePhoto(bDisplay);
	}
}

void ARunner_UTMB::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	
	if (bIsDay)
	{
		IndexBkgComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
        IndexLeftComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
        IndexRightComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
        FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
	} else
	{
		IndexBkgComponent->SetScalarParameterValueOnMaterials("Illum", 0.025f);
        IndexLeftComponent->SetScalarParameterValueOnMaterials("Illum", 0.025f);
        IndexRightComponent->SetScalarParameterValueOnMaterials("Illum", 0.025f);
        FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.025f);
	}
	
	if (PhotoComponent)
	{
		PhotoComponent->UpdateDayNight(bIsDay);
	}
	if (NameComponent)
	{
		NameComponent->UpdateDayNight(bIsDay);
	}
	if (FlagComponent)
	{
		FlagComponent->UpdateDayNight(bIsDay);
	}
}

void ARunner_UTMB::BeginPlay()
{
	Super::BeginPlay();
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &ARunner_UTMB::UpdateDayNight);
	}
}
