// All Rights Reserved

#include "Components/ClubComponent.h"
#include "RaceManager.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Engine/Font.h"
#include "Components/RawTextMaterial.h"

UClubComponent::UClubComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
		
	ClubSplineComponent = CreateDefaultSubobject<USplineComponent>(FName("Spline"));
	ClubSplineComponent->SetupAttachment(this);
	ClubSplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ClubSplineComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	ClubSplineComponent->SetMobility(EComponentMobility::Movable);
		
	ClubTextComponent = CreateDefaultSubobject<UTextRenderComponent>(FName("ClubText"));
	ClubTextComponent->SetupAttachment(ClubSplineComponent);
	ClubTextComponent->SetRelativeTransform(FTransform::Identity);
	ClubTextComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ClubTextComponent->SetRelativeLocation(FVector(10.f, -30.f, 30.f));
	
	ClubTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	ClubTextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	ClubTextComponent->SetWorldSize(55.f);
	ClubTextComponent->SetHorizSpacingAdjust(0.f);
	ClubTextComponent->SetTextRenderColor(FColor(255, 255, 255, 255));
	ClubTextComponent->SetCastShadow(false);
	ClubTextComponent->bReceivesDecals = false;
	ClubTextComponent->bRenderInMainPass = true;
	ClubTextComponent->bRenderCustomDepth = false;
	ClubTextComponent->bRenderInDepthPass = false;
	ClubTextComponent->SetMobility(EComponentMobility::Movable);
	
	ClubBkgComponent = CreateDefaultSubobject<USplineMeshComponent>(FName("ClubBkgComponent"));
	ClubBkgComponent->SetupAttachment(ClubSplineComponent);
	ClubBkgComponent->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ClubMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/GTWS_SplineMeshClub.GTWS_SplineMeshClub"));
	if (ClubMesh.Succeeded())
	{
		ClubBkgComponent->SetStaticMesh(ClubMesh.Object);
	}
	
	WidgetClubComponent = ObjectInitializer.CreateDefaultSubobject<UTrailTextWidgetComponent>(this, TEXT("ClubWidget"));
	WidgetClubComponent->SetupAttachment(this);
	WidgetClubComponent->SetRelativeTransform(FTransform::Identity);
	WidgetClubComponent->SetRelativeLocation(FVector(10.f, 0.f, 0.f));
	
	ConstructorHelpers::FObjectFinder<UFont> TextFont(
		TEXT("/Game/LTVContent/2D/Fonts/Owanium/Oxanium-Bold_Font.Oxanium-Bold_Font"));
	if (TextFont.Succeeded())
	{
		ClubFont = TextFont.Object;
		FSlateFontInfo FontInfo;
		FontInfo.FontObject = ClubFont;
		FontInfo.Size = 30.f;
		WidgetClubComponent->SetLabelFont(FontInfo);
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("No Font"));
	}
}

void UClubComponent::UpdateClub(const FRunnerStruct& RunnerDatas)
{
	if (ClubTextComponent)
	{
		ClubTextComponent->SetText(FText::FromString(RunnerDatas.club.ToUpper()));
		TrailRawText::Apply(ClubTextComponent, FColor::White);
		
		FVector ClubLocalSize = ClubTextComponent->GetTextLocalSize();
		StartLocation = FVector(0.f, ClubLocalSize.Y + 100.f, 0.f);
		LastLocation = FVector(0.f, -ClubLocalSize.Y - 100.f, 0.f);
		
		ClubSplineComponent->ClearSplinePoints();
		ClubSplineComponent->AddSplinePointAtIndex(StartLocation, 0, ESplineCoordinateSpace::Local, false);
		ClubSplineComponent->SetSplinePointType(0, ESplinePointType::Linear, false);
		ClubSplineComponent->AddSplinePointAtIndex(LastLocation, 1, ESplineCoordinateSpace::Local, false);
		ClubSplineComponent->SetSplinePointType(1, ESplinePointType::Linear, false);
		ClubSplineComponent->UpdateSpline();
	
		const FVector Delta = LastLocation - StartLocation;
		const FVector Tan = Delta * 0.5f;
	
		ClubBkgComponent->SetStartAndEnd(StartLocation, Tan, LastLocation, Tan);
		ClubBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
		ClubBkgComponent->SetStartScale(FVector2D(1.0f, 1.0f));
		ClubBkgComponent->SetEndScale(FVector2D(1.0f, 1.0f));
		ClubBkgComponent->UpdateMesh();
		
	}
	ClubTextComponent->SetHiddenInGame(!bShow3DText);
	
	
	if (WidgetClubComponent)
	{
		WidgetClubComponent->SetLabelText(FText::FromString(RunnerDatas.club));
		WidgetClubComponent->SetLabelColor(FColor(255,255,255,255));
	} 
	WidgetClubComponent->SetHiddenInGame(!bShowWidgetText);
	
}

FVector UClubComponent::GetLocalSize() const
{
	return ClubTextComponent->GetTextLocalSize();
}

void UClubComponent::UpdateMesh(FVector NewFirstLocation, FVector NewLastLocation)
{
	ClubSplineComponent->SetLocationAtSplinePoint(0, NewFirstLocation, ESplineCoordinateSpace::Local);
	ClubSplineComponent->SetLocationAtSplinePoint(1, NewLastLocation, ESplineCoordinateSpace::Local);
	ClubSplineComponent->UpdateSpline();
	ClubBkgComponent->SetStartPosition(NewFirstLocation, false);
	ClubBkgComponent->SetEndPosition(NewLastLocation, false);
	ClubBkgComponent->UpdateMesh();
}

FStartAndEnd UClubComponent::GetStartAndEnd() const
{
	FStartAndEnd StartAndEnd = FStartAndEnd(StartLocation, LastLocation);
	return StartAndEnd;
}

float UClubComponent::GetMiddle() const
{
	return (ClubSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).Y - ClubSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Y) * 0.5f;
}

void UClubComponent::ToggleClub(bool bShow) const
{
	ClubSplineComponent->SetHiddenInGame(!bShow);
	ClubTextComponent->SetHiddenInGame(!bShow);
	ClubBkgComponent->SetHiddenInGame(!bShow);
}

void UClubComponent::ShowWidgetName(bool bIsVisible)
{
	bShowWidgetText = bIsVisible;
}

void UClubComponent::Show3DName(bool bIsVisible)
{
	bShow3DText = bIsVisible;
}

void UClubComponent::UpdateDayNight(bool bIsDay) const
{
	
}
