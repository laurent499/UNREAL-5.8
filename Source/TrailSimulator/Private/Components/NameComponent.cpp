// All Rights Reserved


#include "Components/NameComponent.h"
#include "TrailSharedTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Engine/Font.h"
#include "Components/RawTextMaterial.h"

UNameComponent::UNameComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	NameSplineComponent = ObjectInitializer.CreateDefaultSubobject<USplineComponent>(this, FName("Spline"));
	NameSplineComponent->SetupAttachment(this);
	NameSplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	NameSplineComponent->SetMobility(EComponentMobility::Movable);
	
	NameTextComponent = ObjectInitializer.CreateDefaultSubobject<UTextRenderComponent>(this, FName("NameText"));
	NameTextComponent->SetupAttachment(this);
	NameTextComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	NameTextComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	NameTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	NameTextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	NameTextComponent->SetWorldSize(90.f);
	NameTextComponent->SetCastShadow(false);
	NameTextComponent->bReceivesDecals = false;
	NameTextComponent->bRenderInMainPass = true;
	NameTextComponent->bRenderCustomDepth = false;
	NameTextComponent->bRenderInDepthPass = false;	
	NameTextComponent->SetMobility(EComponentMobility::Movable);
	
	NameBkgComponent = ObjectInitializer.CreateDefaultSubobject<USplineMeshComponent>(this, TEXT("NameBkgComponent"));
	NameBkgComponent->SetupAttachment(NameSplineComponent);
	NameBkgComponent->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Name_UTMB.Bkg_Name_UTMB"));
	if (NameMesh.Succeeded())
	{
		NameBkgComponent->SetStaticMesh(NameMesh.Object);
	}
	
	WidgetNameComponent = ObjectInitializer.CreateDefaultSubobject<UTrailTextWidgetComponent>(this, TEXT("NameWidget"));
	WidgetNameComponent->SetupAttachment(NameSplineComponent);
	WidgetNameComponent->SetRelativeTransform(FTransform::Identity);
	WidgetNameComponent->SetRelativeLocation(FVector(0.1f, 0.f, 0.f));
	
	NameSplineComponent->SetRelativeTransform(FTransform::Identity);
	NameTextComponent->SetRelativeTransform(FTransform::Identity);
	NameBkgComponent->SetRelativeTransform(FTransform::Identity);
}

void UNameComponent::UpdateName(const FString& RunnerName)
{	
	if (NameTextComponent)
	{
		NameTextComponent->SetText(FText::FromString(RunnerName));
		TrailRawText::Apply(NameTextComponent, FColor::White);
		FVector NameLocalSize = NameTextComponent->GetTextLocalSize();
		
		StartLocation = FVector(0.f, 0.f, 0.f);
		LastLocation = FVector(0.f, -NameLocalSize.Y, 0.f);
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
	NameTextComponent->SetHiddenInGame(!bShow3DText);
	
	if (WidgetNameComponent)
	{
		WidgetNameComponent->SetLabelText(FText::FromString(RunnerName));
		WidgetNameComponent->SetLabelColor(FColor::Black);
	}
	WidgetNameComponent->SetHiddenInGame(!bShowWidgetText);
}

void UNameComponent::UpdateMesh(FVector NewFirstLocation, FVector NewLastLocation)
{
	NameSplineComponent->SetLocationAtSplinePoint(0, NewFirstLocation, ESplineCoordinateSpace::Local);
	NameSplineComponent->SetLocationAtSplinePoint(1, NewLastLocation, ESplineCoordinateSpace::Local);
	NameSplineComponent->UpdateSpline();
	NameBkgComponent->SetStartPosition(NewFirstLocation, false);
	NameBkgComponent->SetEndPosition(NewLastLocation, false);
	NameBkgComponent->UpdateMesh();
}

FStartAndEnd UNameComponent::GetStartAndEnd() const
{
	FStartAndEnd StartAndEnd = FStartAndEnd(StartLocation, LastLocation);
	return StartAndEnd;
}

void UNameComponent::ShowWidgetName(bool bIsVisible)
{
	if (WidgetNameComponent)
	{
		WidgetNameComponent->SetHiddenInGame(!bIsVisible);
		bShowWidgetText = bIsVisible;
	}
}

void UNameComponent::Show3DName(bool bIsVisible)
{
	if (NameTextComponent)
	{
		NameTextComponent->SetHiddenInGame(!bIsVisible);
		bShow3DText = bIsVisible;
	}
}

float UNameComponent::GetMiddle() const
{
	FVector MiddleVector = NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local) - NameSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
	return -MiddleVector.Y/2;
}

FVector UNameComponent::GetLocalSize() const
{
	return NameTextComponent->GetTextLocalSize();
}

FVector UNameComponent::GetWorldLastPointLocation() const
{
	return NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
}

void UNameComponent::UpdateDayNight(bool bIsDay)
{
	if (bIsDay)
	{
		NameBkgComponent->SetCustomPrimitiveDataFloat(7, 0.5f);
		NameTextComponent->SetCustomPrimitiveDataFloat(0, 0.5);
	} else
	{
		NameBkgComponent->SetCustomPrimitiveDataFloat(7, 0.025f);
        NameTextComponent->SetCustomPrimitiveDataFloat(0, 0.025);
	}
}

TObjectPtr<class USplineComponent> UNameComponent::GetSplineComponent() const
{
	return NameSplineComponent;
}