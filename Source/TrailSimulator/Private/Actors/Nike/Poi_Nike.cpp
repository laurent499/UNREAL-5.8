// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Nike/Poi_Nike.h"
#include "Engine/Font.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Fonts/FontMeasure.h"


APoi_Nike::APoi_Nike()
{
    MainPicto->SetHiddenInGame(true);
    NameText->SetHiddenInGame(true);
    AltText->SetHiddenInGame(true);
    TempText->SetHiddenInGame(true);
    Weather1Text->SetHiddenInGame(true);
    Weather2Text->SetHiddenInGame(true);
    Weather3Text->SetHiddenInGame(true);
    MainPicto->SetHiddenInGame(true);
    AltPicto->SetHiddenInGame(true);
    WeatherPicto->SetHiddenInGame(true);
        
    NameWidgetComponent = CreateDefaultSubobject<UTrailTextWidgetComponent>("NameWidgetComponent");
    NameWidgetComponent->SetupAttachment(FootHook);
    NameWidgetComponent->SetComponentTickEnabled(false);
    NameWidgetComponent->SetRelativeTransform(FTransform::Identity);
    NameWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    NameSplineComponent = CreateDefaultSubobject<USplineComponent>("NameSplineComponent");
    NameSplineComponent->SetupAttachment(FootHook);
    NameSplineComponent->SetRelativeTransform(FTransform::Identity);
    NameSplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    NameBkgComponent->SetupAttachment(NameSplineComponent);
    
    Infos1WidgetComponent = CreateDefaultSubobject<UTrailTextWidgetComponent>("Infos1WidgetComponent");
    Infos1WidgetComponent->SetComponentTickEnabled(false);
    Infos1WidgetComponent->SetupAttachment(FootHook);
    Infos1WidgetComponent->SetRelativeTransform(FTransform::Identity);
    Infos1WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos2WidgetComponent = CreateDefaultSubobject<UTrailTextWidgetComponent>("Infos2WidgetComponent");
    Infos2WidgetComponent->SetComponentTickEnabled(false);
    Infos2WidgetComponent->SetupAttachment(FootHook);
    Infos2WidgetComponent->SetRelativeTransform(FTransform::Identity);
    Infos2WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos1SplineComponent = CreateDefaultSubobject<USplineComponent>("Infos1SplineComponent");
    Infos1SplineComponent->SetupAttachment(FootHook);
    Infos1SplineComponent->SetComponentTickEnabled(false);
    Infos1SplineComponent->SetRelativeTransform(FTransform::Identity);
    Infos1SplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos1BkgComponent = CreateDefaultSubobject<USplineMeshComponent>("Infos1BkgComponent");
    Infos1BkgComponent->SetupAttachment(NameSplineComponent);
    Infos1BkgComponent->SetMobility(EComponentMobility::Movable);
    Infos1BkgComponent->SetComponentTickEnabled(false);
    Infos1BkgComponent->SetRelativeTransform(FTransform::Identity);
    Infos1BkgComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos2SplineComponent = CreateDefaultSubobject<USplineComponent>("Infos2SplineComponent");
    Infos2SplineComponent->SetupAttachment(FootHook);
    Infos2SplineComponent->SetComponentTickEnabled(false);
    Infos2SplineComponent->SetRelativeTransform(FTransform::Identity);
    Infos2SplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos2BkgComponent = CreateDefaultSubobject<USplineMeshComponent>("Infos2BkgComponent");
    Infos2BkgComponent->SetupAttachment(NameSplineComponent);
    Infos2BkgComponent->SetMobility(EComponentMobility::Movable);
    Infos2BkgComponent->SetComponentTickEnabled(false);
    Infos2BkgComponent->SetRelativeTransform(FTransform::Identity);
    Infos2BkgComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    // Name
    ConstructorHelpers::FObjectFinder<UFont> NameTextFont(
        TEXT("/Game/LTVContent/2D/Fonts/Nike/HelveticaNeue-Bold-02_Font.HelveticaNeue-Bold-02_Font"));
    if (NameTextFont.Succeeded())
    {
        NameFont = NameTextFont.Object;
        FSlateFontInfo FontInfo;
        FontInfo.FontObject = NameFont;
        FontInfo.Size = 40.f;
        NameWidgetComponent->SetLabelFont(FontInfo);
    } 
    
    // Infos
    ConstructorHelpers::FObjectFinder<UFont> InfosTextFont(
        TEXT("/Game/LTVContent/2D/Fonts/Nike/Courier_Font.Courier_Font"));
    if (NameTextFont.Succeeded())
    {
        InfosFont = InfosTextFont.Object;
        FSlateFontInfo FontInfo;
        FontInfo.FontObject = InfosFont;
        FontInfo.Size = 20.f;
        NameWidgetComponent->SetLabelFont(FontInfo);
    } 
}

void APoi_Nike::UpdatePoi(FRacePOI NewRacePoi, FRaceSetup NewRaceSetup)
{
    Super::UpdatePoi(NewRacePoi, NewRaceSetup);
    
    if (NameWidgetComponent)
    {        
        NameWidgetComponent->SetLabelHJustification(ETextJustify::Left);
        NameWidgetComponent->SetLabelText(FText::FromString(NewRacePoi.name));
        NameWidgetComponent->SetLabelColor(FLinearColor::White);
        
        // FVector2D DrawSize = NameWidgetComponent->GetCurrentDrawSize();
        constexpr float HorizontalPadding = 12.f;
        const FSlateFontInfo& FontInfo = NameWidgetComponent->GetFontInfo();
        const TSharedRef<FSlateFontMeasure> FontMeasure =
            FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

        const FVector2D TextSizePx =
            FontMeasure->Measure(NewRacePoi.name, FontInfo);
        const float HalfBlockWidth =
            TextSizePx.X * 0.5 + HorizontalPadding;
        
        const FVector StartInWidgetLocal(0.f,  HalfBlockWidth + 100.f, 0.f);
        const FVector EndInWidgetLocal  (0.f, -HalfBlockWidth, 0.f);

        StartLocation =
            NameSplineComponent->GetComponentTransform().InverseTransformPosition(
                NameWidgetComponent->GetComponentTransform().TransformPosition(
                    StartInWidgetLocal
                )
            );
        
        LastLocation =
            NameSplineComponent->GetComponentTransform().InverseTransformPosition(
                NameWidgetComponent->GetComponentTransform().TransformPosition(
                    EndInWidgetLocal
                )
            );
        
        
        NameSplineComponent->ClearSplinePoints();

        NameSplineComponent->AddSplinePointAtIndex(
            StartLocation,
            0,
            ESplineCoordinateSpace::Local,
            false
        );

        NameSplineComponent->SetSplinePointType(
            0,
            ESplinePointType::Linear,
            false
        );

        NameSplineComponent->AddSplinePointAtIndex(
            LastLocation,
            1,
            ESplineCoordinateSpace::Local,
            false
        );

        NameSplineComponent->SetSplinePointType(
            1,
            ESplinePointType::Linear,
            false
        );

        NameSplineComponent->UpdateSpline();
        
        const FVector Delta = LastLocation - StartLocation;
        const FVector Tan = Delta * 0.5f;
        
        NameBkgComponent->SetStartAndEnd(StartLocation, Tan, LastLocation, Tan);
        NameBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
        NameBkgComponent->SetStartScale(FVector2D(1.f, 0.6f));
        NameBkgComponent->SetEndScale(FVector2D(1.f, 0.6f));
        NameBkgComponent->UpdateMesh();
    }
    
    if (Infos1WidgetComponent)
    {
        Infos1WidgetComponent->SetLabelText(FText::FromString(FString::Printf(TEXT("%f"),NewRacePoi.elevation)));
        Infos1WidgetComponent->SetLabelHJustification(ETextJustify::Left);
        Infos1WidgetComponent->SetLabelColor(FLinearColor::Black);
        
        constexpr float HorizontalPadding = 12.f;
        const FSlateFontInfo& FontInfo = Infos1WidgetComponent->GetFontInfo();
        const TSharedRef<FSlateFontMeasure> FontMeasure =
            FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

        const FVector2D TextSizePx =
            FontMeasure->Measure(NewRacePoi.name, FontInfo);
        const float HalfBlockWidth =
            TextSizePx.X * 0.5 + HorizontalPadding;
        
        const FVector StartInWidgetLocal(0.f,  HalfBlockWidth + 100.f, 0.f);
        const FVector EndInWidgetLocal  (0.f, -HalfBlockWidth, 0.f);

        StartLocation =
            Infos1SplineComponent->GetComponentTransform().InverseTransformPosition(
                Infos1WidgetComponent->GetComponentTransform().TransformPosition(
                    StartInWidgetLocal
                )
            );
        
        LastLocation =
            Infos1SplineComponent->GetComponentTransform().InverseTransformPosition(
                Infos1WidgetComponent->GetComponentTransform().TransformPosition(
                    EndInWidgetLocal
                )
            );
        
        Infos1SplineComponent->ClearSplinePoints();
        Infos1SplineComponent->AddSplinePointAtIndex(
            StartLocation,
            0,
            ESplineCoordinateSpace::Local,
            false
        );

        Infos1SplineComponent->SetSplinePointType(
            0,
            ESplinePointType::Linear,
            false
        );

        Infos1SplineComponent->AddSplinePointAtIndex(
            LastLocation,
            1,
            ESplineCoordinateSpace::Local,
            false
        );

        Infos1SplineComponent->SetSplinePointType(
            1,
            ESplinePointType::Linear,
            false
        );
        Infos1SplineComponent->UpdateSpline();
        
        const FVector Delta = LastLocation - StartLocation;
        const FVector Tan = Delta * 0.5f;
        
        Infos1BkgComponent->SetStartAndEnd(StartLocation, Tan, LastLocation, Tan);
        Infos1BkgComponent->SetForwardAxis(ESplineMeshAxis::X);
        Infos1BkgComponent->SetStartScale(FVector2D(1.f, 1.f));
        Infos1BkgComponent->SetEndScale(FVector2D(1.f, 1.f));
        Infos1BkgComponent->UpdateMesh();
    }
    
    if (Infos2WidgetComponent)
    {        
        Infos2WidgetComponent->SetLabelText(FText::FromString(FString::Printf(TEXT("%f"),NewRacePoi.elevation)));
        Infos2WidgetComponent->SetLabelHJustification(ETextJustify::Left);
        Infos2WidgetComponent->SetLabelColor(FLinearColor::Black);
        
        constexpr float HorizontalPadding = 12.f;
        const FSlateFontInfo& FontInfo = Infos2WidgetComponent->GetFontInfo();
        const TSharedRef<FSlateFontMeasure> FontMeasure =
            FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

        const FVector2D TextSizePx =
            FontMeasure->Measure(NewRacePoi.name, FontInfo);
        const float HalfBlockWidth =
            TextSizePx.X * 0.5 + HorizontalPadding;
        
        const FVector StartInWidgetLocal(0.f,  HalfBlockWidth + 100.f, 0.f);
        const FVector EndInWidgetLocal  (0.f, -HalfBlockWidth, 0.f);

        StartLocation =
            Infos2SplineComponent->GetComponentTransform().InverseTransformPosition(
                Infos2WidgetComponent->GetComponentTransform().TransformPosition(
                    StartInWidgetLocal
                )
            );
        
        LastLocation =
            Infos2SplineComponent->GetComponentTransform().InverseTransformPosition(
                Infos2WidgetComponent->GetComponentTransform().TransformPosition(
                    EndInWidgetLocal
                )
            );
        
        
        Infos2SplineComponent->ClearSplinePoints();

        Infos2SplineComponent->AddSplinePointAtIndex(
            StartLocation,
            0,
            ESplineCoordinateSpace::Local,
            false
        );

        Infos2SplineComponent->SetSplinePointType(
            0,
            ESplinePointType::Linear,
            false
        );

        Infos2SplineComponent->AddSplinePointAtIndex(
            LastLocation,
            1,
            ESplineCoordinateSpace::Local,
            false
        );

        Infos2SplineComponent->SetSplinePointType(
            1,
            ESplinePointType::Linear,
            false
        );

        Infos2SplineComponent->UpdateSpline();
        
        const FVector Delta = LastLocation - StartLocation;
        const FVector Tan = Delta * 0.5f;
        
        Infos2BkgComponent->SetStartAndEnd(StartLocation, Tan, LastLocation, Tan);
        Infos2BkgComponent->SetForwardAxis(ESplineMeshAxis::X);
        Infos2BkgComponent->SetStartScale(FVector2D(1.f, 1.f));
        Infos2BkgComponent->SetEndScale(FVector2D(1.f, 1.f));
        Infos2BkgComponent->UpdateMesh();
        
        
    }
}