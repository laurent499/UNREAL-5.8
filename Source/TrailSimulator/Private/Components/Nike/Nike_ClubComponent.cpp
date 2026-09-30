// Copyright LTVProd 2026 All Rights Reserved.


#include "Components/Nike/Nike_ClubComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Fonts/FontMeasure.h"


UNike_ClubComponent::UNike_ClubComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ClubMesh(
        TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Plain.Bkg_Plain"));
    if (ClubMesh.Succeeded())
    {
        ClubBkgComponent->SetStaticMesh(ClubMesh.Object);
    }
    
    ClubBkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor(0, 0, 0,1.0));
    ClubBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
    ClubBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
}

void UNike_ClubComponent::UpdateClub(const FRunnerStruct& RunnerDatas)
{
 
    ClubTextComponent->SetHiddenInGame(true);
    WidgetClubComponent->SetHiddenInGame(false);
    if (WidgetClubComponent)
    {
        
        constexpr float HorizontalPadding = 12.f;
        const FSlateFontInfo& FontInfo = WidgetClubComponent->GetFontInfo();
        const TSharedRef<FSlateFontMeasure> FontMeasure =
            FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

        const FVector2D TextSizePx =
            FontMeasure->Measure(RunnerDatas.club, FontInfo);
        const float HalfBlockWidth =
            TextSizePx.X * 0.5 + HorizontalPadding;
        
        // const FVector StartInWidgetLocal(0.f,  HalfBlockWidth, 0.f);
        const FVector StartInWidgetLocal(0.f,  HalfBlockWidth + 100.f, 0.f);
        const FVector EndInWidgetLocal  (0.f, -HalfBlockWidth, 0.f);

        StartLocation =
            ClubSplineComponent->GetComponentTransform().InverseTransformPosition(
                WidgetClubComponent->GetComponentTransform().TransformPosition(
                    StartInWidgetLocal
                )
            );
        StartLocation.Z = 8.f;
        LastLocation =
            ClubSplineComponent->GetComponentTransform().InverseTransformPosition(
                WidgetClubComponent->GetComponentTransform().TransformPosition(
                    EndInWidgetLocal
                )
            );
        LastLocation.Z = 8.f;
        ClubSplineComponent->ClearSplinePoints();

        ClubSplineComponent->AddSplinePointAtIndex(
            StartLocation,
            0,
            ESplineCoordinateSpace::Local,
            false
        );

        ClubSplineComponent->SetSplinePointType(
            0,
            ESplinePointType::Linear,
            false
        );

        ClubSplineComponent->AddSplinePointAtIndex(
            LastLocation,
            1,
            ESplineCoordinateSpace::Local,
            false
        );

        ClubSplineComponent->SetSplinePointType(
            1,
            ESplinePointType::Linear,
            false
        );

        ClubSplineComponent->UpdateSpline();
        
        const FVector Delta = LastLocation - StartLocation;
        const FVector Tan = Delta * 0.5f;
        
        ClubBkgComponent->SetStartAndEnd(StartLocation, Tan, LastLocation, Tan);
        ClubBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
        ClubBkgComponent->SetStartScale(FVector2D(1.f, 0.6f));
        ClubBkgComponent->SetEndScale(FVector2D(1.f, 0.6f));
        ClubBkgComponent->UpdateMesh();
        
        WidgetClubComponent->SetRelativeLocation(FVector(1.0f, 0.0f, 31.0f));
        WidgetClubComponent->SetLabelText(FText::FromString(RunnerDatas.club));
        WidgetClubComponent->SetLabelHJustification(ETextJustify::Center);
        WidgetClubComponent->SetLabelColor(FColor::White);
    } 
}