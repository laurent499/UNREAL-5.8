// All Rights Reserved

#include "Components/GTWS/GTWS_NameComponent.h"
#include "GTWS/GTWSStyle.h"
#include "Components/SplineComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"

UGTWS_NameComponent::UGTWS_NameComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	NameTextComponent->SetWorldSize(90.f);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Plain.Bkg_Plain"));
	if (NameMesh.Succeeded())
	{
		NameBkgComponent->SetStaticMesh(NameMesh.Object);
	}
	NameBkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor::White);
	NameBkgComponent->SetCustomPrimitiveDataFloat(5,1.f);
}

void UGTWS_NameComponent::UpdateName(const FString& RunnerName)
{
	Super::UpdateName(RunnerName);
	NameTextComponent->SetTextRenderColor(FColor::Black);
	NameTextComponent->SetRelativeLocation(FVector(10.f, 0.0f, 60.0f));
	NameTextComponent->SetHorizontalAlignment(EHTA_Left);
	NameTextComponent->AddRelativeLocation(FVector(0.f, 100.f, 0.f));
	
	// Bornes provisoires : ARunner_GTWS les réécrit ensuite avec la largeur partagée nom/club
	FVector NameLocalSize = NameTextComponent->GetTextLocalSize();
	StartLocation = FVector(0.f, GTWSStyle::BkgLeftMargin, 0.f);
	LastLocation = FVector(0.f, -NameLocalSize.Y - GTWSStyle::BkgRightMargin, 0.f);
	if (NameSplineComponent->GetNumberOfSplinePoints()>=2){
		NameSplineComponent->ClearSplinePoints();
		NameSplineComponent->AddSplinePointAtIndex(StartLocation, 0, ESplineCoordinateSpace::Local, false);
		NameSplineComponent->AddSplinePointAtIndex(LastLocation, 1, ESplineCoordinateSpace::Local, true);
	}
	
	const FVector Delta = LastLocation - StartLocation;
	const FVector Tan = Delta * 0.5f;
		
	NameBkgComponent->SetStartAndEnd(StartLocation, Tan, LastLocation, Tan);
	NameBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
	NameBkgComponent->SetStartScale(FVector2D(1.0f, 1.0f));
	NameBkgComponent->SetEndScale(FVector2D(1.0f, 1.0f));
	NameBkgComponent->UpdateMesh();
}

float UGTWS_NameComponent::GetMiddle() const
{
	return 
		(LastLocation.Y - StartLocation.Y) * 0.5f;
}

FStartAndEnd UGTWS_NameComponent::GetStartAndEnd() const
{
	return Super::GetStartAndEnd();
}

float UGTWS_NameComponent::GetSplineLength() const
{
	return NameSplineComponent->GetSplineLength();
}

FVector UGTWS_NameComponent::GetSplineLastPoint() const
{
	return NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local);
}

void UGTWS_NameComponent::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	if (bIsDay)
	{
		NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
	} else
	{
		NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		
	}
}