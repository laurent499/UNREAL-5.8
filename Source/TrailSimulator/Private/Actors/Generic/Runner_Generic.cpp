// All Rights Reserved


#include "Actors/Generic/Runner_Generic.h"

#include "RaceManager.h"
#include "RaceSubsystem.h"
#include "Components/PhotoComponent.h"
#include "Components/Generic/Generic_FlagComponent.h"
#include "Components/Generic/Generic_NameComponent.h"
#include "Kismet/GameplayStatics.h"

ARunner_Generic::ARunner_Generic()
{		
	// Name
	NameComponent = CreateDefaultSubobject<UGeneric_NameComponent>(TEXT("NameComponent"));
	NameComponent->SetupAttachment(NameHook);
	NameComponent->SetRelativeTransform(FTransform::Identity);
	NameComponent->SetComponentTickEnabled(false);
	NameComponent->ShowWidgetName(false);
	NameComponent->Show3DName(true);
	
	// Photo
	PhotoHook = CreateDefaultSubobject<USceneComponent>(TEXT("PhotoHook"));
	PhotoHook->SetupAttachment(NameHook);
	PhotoHook->SetComponentTickEnabled(false);
	PhotoHook->SetRelativeTransform(FTransform::Identity);
	PhotoHook->SetRelativeLocation(FVector(0.f, 0.f, 83.f));
	
	PhotoComponent = CreateDefaultSubobject<UPhotoComponent>(TEXT("PhotoComponent"));
	PhotoComponent->SetupAttachment(PhotoHook);
	PhotoComponent->SetComponentTickEnabled(false);
	PhotoComponent->SetRelativeTransform(FTransform::Identity);
	
	// Flag
	FlagComponent = CreateDefaultSubobject<UGeneric_FlagComponent>(TEXT("FlagComponent"));
	FlagComponent->SetupAttachment(NameComponent);
	FlagComponent->SetComponentTickEnabled(false);
	FlagComponent->SetRelativeTransform(FTransform::Identity);
	FlagComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	
	// Slash
	SlashHook = CreateDefaultSubobject<USceneComponent>(TEXT("SlashHook"));
	SlashHook->SetupAttachment(NameComponent);
	SlashHook->SetComponentTickEnabled(false);
	SlashHook->SetRelativeTransform(FTransform::Identity);
	SlashHook->SetRelativeLocation(FVector(-20.f, 314.f, 0.f));
	
	SlashComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlashComponent"));
	SlashComponent->SetupAttachment(SlashHook);
	SlashComponent->SetComponentTickEnabled(false);
	SlashComponent->SetRelativeTransform(FTransform::Identity);
	SlashComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	SlashComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SlashMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Generic_Color.Generic_Color"));
	if (SlashMesh.Succeeded())
	{
		SlashComponent->SetStaticMesh(SlashMesh.Object);
	}
}

void ARunner_Generic::UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup)
{	
	Super::UpdateRunner(Runner, RaceSetup);
	
	// Foot
	if (FootComponent)
	{		
		UMaterialInterface* CoreMat = FootComponent->GetMaterial(0);
		UMaterialInstanceDynamic* MID = FootComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, CoreMat);
		FootComponent->SetColorParameterValueOnMaterials("BaseColor", FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
	}
	
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
	
	if (FlagComponent && IsFlagVisible() && !Runner.pays.IsEmpty() ){
		FlagComponent->UpdateFlag(Runner.pays);
	}
	
	if (SlashComponent)
	{
		UMaterialInterface* BaseMat = SlashComponent->GetMaterial(0);
		UMaterialInstanceDynamic* MID = SlashComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMat);
		if(!MID) return;
		if (GetGameInstance())
		{
			MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(FColor::FromHex("#98BF16ff")));
		}
	}
}

void ARunner_Generic::ToggleFlag(bool bDisplay)
{
	Super::ToggleFlag(bDisplay);
	if (FlagComponent)
	{
		FlagComponent->ToggleFlag(bDisplay);
	}
}

void ARunner_Generic::TogglePhoto(bool bDisplay)
{
	Super::TogglePhoto(bDisplay);
	if (PhotoComponent)
	{
		PhotoComponent->TogglePhoto(bDisplay);
	}
}

void ARunner_Generic::SetIsStacked(bool bInStacked)
{
	Super::SetIsStacked(bInStacked);
	if (PhotoComponent)
		PhotoComponent->TogglePhoto(!bInStacked);
}

void ARunner_Generic::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	
	if (bIsDay)
	{
		FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
	} else
	{
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

void ARunner_Generic::BeginPlay()
{
	Super::BeginPlay();
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &ARunner_Generic::UpdateDayNight);
	}
}
