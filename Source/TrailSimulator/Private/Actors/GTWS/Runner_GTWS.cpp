// All Rights Reserved


#include "Actors/GTWS/Runner_GTWS.h"
#include "RaceManager.h"
#include "GTWS/GTWSStyle.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Components/PhotoComponent.h"
#include "Components/ClubComponent.h"
#include "Components/GTWS/GTWS_ClubComponent.h"
#include "Components/GTWS/GTWS_FlagComponent.h"
#include "Components/GTWS/GTWS_NameComponent.h"
#include "Kismet/GameplayStatics.h"

ARunner_GTWS::ARunner_GTWS()
{
	
	PresentationRoot->SetRelativeLocation(FVector(0.f, 0.f, 700.f));
	
	// Name
	NameComponent = CreateDefaultSubobject<UGTWS_NameComponent>(TEXT("NameComponent"));
	NameComponent->SetupAttachment(NameHook);
	NameComponent->SetComponentTickEnabled(false);
	NameComponent->SetRelativeTransform(FTransform::Identity);
	NameComponent->ShowWidgetName(false);
	NameComponent->Show3DName(true);
	NameHook->SetRelativeLocation(FVector(30.f, 0.f, 0.f));
	
	// Photo
	PhotoHook = CreateDefaultSubobject<USceneComponent>(TEXT("PhotoHook"));
	PhotoHook->SetupAttachment(NameHook);
	PhotoHook->SetComponentTickEnabled(false);
	PhotoHook->SetRelativeTransform(FTransform::Identity);
	PhotoHook->SetRelativeLocation(FVector(31.f, 130.f, -60.f));
	
	PhotoComponent = CreateDefaultSubobject<UPhotoComponent>(TEXT("PhotoComponent"));
	PhotoComponent->SetupAttachment(PhotoHook);
	PhotoComponent->SetComponentTickEnabled(false);
	PhotoComponent->SetGenerateOverlapEvents(false);
	PhotoComponent->SetRelativeTransform(FTransform::Identity);
	PhotoComponent->SetRelativeScale3D(FVector(0.8f, 1.f, 0.8f));
	
	// Club
	ClubHook = CreateDefaultSubobject<USceneComponent>(TEXT("ClubHook"));
	ClubHook->SetupAttachment(FootHook);
	ClubHook->SetComponentTickEnabled(false);
	ClubHook->SetRelativeTransform(FTransform::Identity);
	ClubHook->SetRelativeLocation(FVector(30.f, 0.f, -63.f));
	ClubHook->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	
	ClubComponent = CreateDefaultSubobject<UGTWS_ClubComponent>(TEXT("ClubComponent"));
	ClubComponent->SetupAttachment(ClubHook);
	ClubComponent->SetComponentTickEnabled(false);
	ClubComponent->SetRelativeTransform(FTransform::Identity);
	ClubComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.0f));
	ClubComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ClubComponent->ShowWidgetName(false);
	ClubComponent->Show3DName(true);
	
	// Flag
	FlagComponent = CreateDefaultSubobject<UGTWS_FlagComponent>(TEXT("FlagComponent"));
	FlagComponent->SetupAttachment(ClubHook);
	FlagComponent->SetComponentTickEnabled(false);
	FlagComponent->SetRelativeTransform(FTransform::Identity);
	FlagComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	FlagComponent->SetRelativeLocation(FVector(0.f, -6.f, 0.0f));
	FlagComponent->SetRelativeScale3D(FVector(0.515f, 1.f, 0.515f));
}

void ARunner_GTWS::UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup)
{
	Super::UpdateRunner(Runner, RaceSetup);
	
	// Foot
	if (FootComponent)
	{
		FootComponent->SetColorParameterValueOnMaterials("BaseColor", FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
	}
	
	if (ClubComponent && NameComponent)
	{
		// Les deux textes sont posés d'abord : il faut qu'ils le soient pour être mesurables
		NameComponent->UpdateName(Runner.nom);
		ClubComponent->UpdateClub(Runner);

		// Le plus large des deux impose la largeur des deux fonds
		const float ClubTextWidth = IsClubVisible() ? ClubComponent->GetLocalSize().Y : 0.f;
		const FStartAndEnd SharedBounds = GTWSStyle::MakeSharedBkgBounds(
			NameComponent->GetLocalSize().Y, ClubTextWidth);

		NameComponent->UpdateMesh(SharedBounds.StartLocation, SharedBounds.EndLocation);
		if (IsClubVisible())
		{
			ClubComponent->UpdateMesh(SharedBounds.StartLocation, SharedBounds.EndLocation);
		}
	}
	
	if (PhotoComponent)
	{		
		if (IsPhotoVisible()){
			
			FString NewPhotoUrl = Runner.photo;
			if (!Runner.photo.Contains("jpg") && !Runner.photo.Contains("jpeg") && !Runner.photo.Contains("png") && !Runner.photo.Contains("gif"))
			{
				NewPhotoUrl += ".png";
			}
			PhotoComponent->UpdatePhoto(NewPhotoUrl, FVector(0.75f, 1.0f, 0.75f));
			ClubHook->SetRelativeLocation(FVector(30.f, 0.f, -63.f));
			NameHook->SetRelativeLocation(FVector(30.f, 0.f, 0.f));
		} else
		{
			float MiddleClub = ClubComponent->GetMiddle() * 0.5f;
			ClubHook->SetRelativeLocation(FVector(30.f, -MiddleClub-0.515f, -63.f));
			NameHook->SetRelativeLocation(FVector(30.f, -MiddleClub-0.515f, 0.f));
		}
	}
	
	if (FlagComponent && !Runner.pays.IsEmpty() && IsFlagVisible()){
		FlagComponent->UpdateFlag(Runner.pays);
	}	
}

void ARunner_GTWS::ToggleClub(bool bDisplay)
{
	Super::ToggleClub(bDisplay);
	if (ClubComponent)
	{
		ClubComponent->ToggleClub(bDisplay);
	}
}

void ARunner_GTWS::ToggleFlag(bool bDisplay)
{
	Super::ToggleFlag(bDisplay);
	if (FlagComponent)
	{
		FlagComponent->ToggleFlag(bDisplay);
	}
}

void ARunner_GTWS::TogglePhoto(bool bDisplay)
{
	Super::TogglePhoto(bDisplay);
	if (PhotoComponent)
	{
		PhotoComponent->TogglePhoto(bDisplay);
	}
}

void ARunner_GTWS::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	
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

void ARunner_GTWS::BeginPlay()
{
	Super::BeginPlay();
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &ARunner_GTWS::UpdateDayNight);
	}
}