// All Rights Reserved


#include "Components/UTMB/UTMB_NameComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"

UUTMB_NameComponent::UUTMB_NameComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	NameTextComponent->SetWorldSize(90.f);
	NameTextComponent->SetTextRenderColor(FColor(255, 255, 255, 255));
	NameTextComponent->SetVerticalAlignment(EVRTA_TextBottom);
	NameTextComponent->SetHorizontalAlignment(EHTA_Left);
	NameTextComponent->SetRelativeLocation(FVector(10.f, -55.f, 15.f));
	RightSlashComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("RightSlash"));
	RightSlashComponent->SetupAttachment(NameSplineComponent);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SlashMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Slash_Name_UTMB.Slash_Name_UTMB"));
	if (SlashMesh.Succeeded())
	{
		RightSlashComponent->SetStaticMesh(SlashMesh.Object);
	}
	RightSlashComponent->SetRelativeTransform(FTransform::Identity);
	
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Name_UTMB.Bkg_Name_UTMB"));
	if (NameMesh.Succeeded())
	{
		NameBkgComponent->SetStaticMesh(NameMesh.Object);
	}
	NameBkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor(0.624, 0.0, 0.036,1.0));
	NameBkgComponent->SetCustomPrimitiveDataVector4(4, FLinearColor(0.0, 0.004025, 0.057805,1.0));
	// NameBkgComponent->SetCustomPrimitiveDataFloat(9, 1.f);		// alpha
	NameBkgComponent->SetCustomPrimitiveDataFloat(20, 0.1f);		// Coverage
	NameBkgComponent->SetCustomPrimitiveDataFloat(21, 0.7f);		// BlendWith
	NameBkgComponent->SetCustomPrimitiveDataFloat(22, 0.0625f);	// Illum
	NameBkgComponent->SetCustomPrimitiveDataFloat(23, 1.f);		// Alpha
	
	RightSlashComponent->SetCustomPrimitiveDataVector4(0, FLinearColor(0.0, 0.004025, 0.057805,1.0));
	RightSlashComponent->SetCustomPrimitiveDataVector4(4, FLinearColor(0.0,0.097587,1.0,1.0));
	// NameBkgComponent->SetCustomPrimitiveDataFloat(9, 1.f);		// alpha
	RightSlashComponent->SetCustomPrimitiveDataFloat(20, 1.f);		// Coverage
	RightSlashComponent->SetCustomPrimitiveDataFloat(21, 0.7f);	// BlendWith
	RightSlashComponent->SetCustomPrimitiveDataFloat(22, 0.0625f);	// Illum
	RightSlashComponent->SetCustomPrimitiveDataFloat(23, 1.f);		// Alpha
}

void UUTMB_NameComponent::UpdateName(const FString& RunnerName)
{	
	Super::UpdateName(RunnerName);
	if (NameTextComponent)
	{
		NameTextComponent->SetText(FText::FromString(RunnerName));
		FVector NameLocalSize = NameTextComponent->GetTextLocalSize();
		
		StartLocation = FVector(0.f, 0.f, 0.f);
		LastLocation = FVector(0.f, -NameLocalSize.Y - 100.f, 0.f);
		if (NameSplineComponent->GetNumberOfSplinePoints()>=2){
			NameSplineComponent->ClearSplinePoints();
			NameSplineComponent->AddSplinePointAtIndex(StartLocation, 0, ESplineCoordinateSpace::Local, false);
			NameSplineComponent->SetSplinePointType(0, ESplinePointType::Linear, false);
			NameSplineComponent->AddSplinePointAtIndex(LastLocation, 1, ESplineCoordinateSpace::Local, true);
			NameSplineComponent->SetSplinePointType(1, ESplinePointType::Linear, false);
			NameSplineComponent->UpdateSpline();
		}
		
		const FVector Delta = LastLocation - StartLocation;
		const FVector Tan = Delta * 0.5f;
		
		NameBkgComponent->SetStartAndEnd(StartLocation, Tan, LastLocation, Tan);
		NameBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
		NameBkgComponent->SetStartScale(FVector2D(1.0f, 1.0f));
		NameBkgComponent->SetEndScale(FVector2D(1.0f, 1.0f));
		NameBkgComponent->UpdateMesh();
	}
	
	RightSlashComponent->SetRelativeLocation(FVector(
		NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).X - 20.f,
		NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).Y,
		NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).Z));
	RightSlashComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

void UUTMB_NameComponent::UpdateMesh(FVector NewFirstLocation, FVector NewLastLocation)
{
	NameSplineComponent->SetLocationAtSplinePoint(0, NewFirstLocation, ESplineCoordinateSpace::Local);
	NameSplineComponent->SetLocationAtSplinePoint(1, NewLastLocation, ESplineCoordinateSpace::Local);
	NameSplineComponent->UpdateSpline();
	NameBkgComponent->SetStartPosition(NewFirstLocation, false);
	NameBkgComponent->SetEndPosition(NewLastLocation, false);
	NameBkgComponent->UpdateMesh();
}

float UUTMB_NameComponent::GetMiddle() const
{
	return (NameSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Y + 60.f + 
	NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).Y + 20.f) * -0.5;
}


void UUTMB_NameComponent::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	if (bIsDay)
	{
		NameBkgComponent->SetCustomPrimitiveDataFloat(22, 0.5f);
		RightSlashComponent->SetCustomPrimitiveDataFloat(22, 0.5f);
	} else
	{
		NameBkgComponent->SetCustomPrimitiveDataFloat(22, 0.025f);
		RightSlashComponent->SetCustomPrimitiveDataFloat(22, 0.025f);
		
	}
}
