// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Nike/Checkpoint_Nike.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Engine/Font.h"
#include "Fonts/FontMeasure.h"

ACheckpoint_Nike::ACheckpoint_Nike()
{
    MainPicto->DestroyComponent();
    NameText->DestroyComponent();
    DistText->DestroyComponent();
    AltText->DestroyComponent();
    TempText->DestroyComponent();
    Weather1Text->DestroyComponent();
    Weather2Text->DestroyComponent();
    Weather3Text->DestroyComponent();
    MainPicto->DestroyComponent();
    DistPicto->DestroyComponent();
    AltPicto->DestroyComponent();
    WeatherPicto->DestroyComponent();
    SplineComponent->DestroyComponent();
    
    // FootComponent->SetWorldScale3D(FVector(0.25f, 0.25f, 0.25f));
        
    SplineHook->SetRelativeLocation(FVector(0.f, 0.f, -28.f));
        
    NameSplineComponent = CreateDefaultSubobject<USplineComponent>("NameSplineComponent");
    NameSplineComponent->SetupAttachment(SplineHook);
    NameSplineComponent->SetRelativeTransform(FTransform::Identity);
    NameSplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    NameWidgetComponent = CreateDefaultSubobject<UTrailTextWidgetComponent>("NameWidgetComponent");
    NameWidgetComponent->SetupAttachment(NameSplineComponent);
    NameWidgetComponent->SetComponentTickEnabled(false);
    NameWidgetComponent->SetRelativeTransform(FTransform::Identity);
    NameWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    NameBkgComponent->SetupAttachment(NameSplineComponent);
    
    Infos1SplineComponent = CreateDefaultSubobject<USplineComponent>("Infos1SplineComponent");
    Infos1SplineComponent->SetComponentTickEnabled(false);
    Infos1SplineComponent->SetupAttachment(SplineHook);
    Infos1SplineComponent->SetRelativeTransform(FTransform::Identity);
    Infos1SplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos1WidgetComponent = CreateDefaultSubobject<UTrailTextWidgetComponent>("Infos1WidgetComponent");
    Infos1WidgetComponent->SetupAttachment(Infos1SplineComponent);
    Infos1WidgetComponent->SetComponentTickEnabled(false);
    Infos1WidgetComponent->SetRelativeTransform(FTransform::Identity);
    Infos1WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos1BkgComponent = CreateDefaultSubobject<USplineMeshComponent>("Infos1BkgComponent");
    Infos1BkgComponent->SetupAttachment(Infos1SplineComponent);
    Infos1BkgComponent->SetMobility(EComponentMobility::Movable);
    Infos1BkgComponent->SetComponentTickEnabled(false);
    Infos1BkgComponent->SetRelativeTransform(FTransform::Identity);
    Infos1BkgComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos2SplineComponent = CreateDefaultSubobject<USplineComponent>("Infos2SplineComponent");
    Infos2SplineComponent->SetupAttachment(SplineHook);
    Infos2SplineComponent->SetComponentTickEnabled(false);
    Infos2SplineComponent->SetRelativeTransform(FTransform::Identity);
    Infos2SplineComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos2WidgetComponent = CreateDefaultSubobject<UTrailTextWidgetComponent>("Infos2WidgetComponent");
    Infos2WidgetComponent->SetupAttachment(Infos2SplineComponent);
    Infos2WidgetComponent->SetComponentTickEnabled(false);
    Infos2WidgetComponent->SetRelativeTransform(FTransform::Identity);
    Infos2WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    Infos2BkgComponent = CreateDefaultSubobject<USplineMeshComponent>("Infos2BkgComponent");
    Infos2BkgComponent->SetupAttachment(Infos2SplineComponent);
    Infos2BkgComponent->SetMobility(EComponentMobility::Movable);
    Infos2BkgComponent->SetComponentTickEnabled(false);
    Infos2BkgComponent->SetRelativeTransform(FTransform::Identity);
    Infos2BkgComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    
    // Bkg Infos
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BkgMesh(
        TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Plain.Bkg_Plain"));
    if (BkgMesh.Succeeded())
    {
        Infos1BkgComponent->SetStaticMesh(BkgMesh.Object);
        Infos2BkgComponent->SetStaticMesh(BkgMesh.Object);
    }
    // Name
    ConstructorHelpers::FObjectFinder<UFont> NameTextFont(
        TEXT("/Game/LTVContent/2D/Fonts/Nike/HelveticaNeue-Bold-02_Font.HelveticaNeue-Bold-02_Font"));
    if (NameTextFont.Succeeded())
    {
        NameFont = NameTextFont.Object;
        FSlateFontInfo FontInfo;
        FontInfo.FontObject = NameFont;
        FontInfo.Size = 60.f;
        NameWidgetComponent->SetBlendMode(EWidgetBlendMode::Masked);
        NameWidgetComponent->SetLabelFont(FontInfo);
    } 
    
    // Infos
    ConstructorHelpers::FObjectFinder<UFont> InfosTextFont(
        TEXT("/Game/LTVContent/2D/Fonts/Courier_Font.Courier_Font"));
    if (InfosTextFont.Succeeded())
    {
        InfosFont = InfosTextFont.Object;
        FSlateFontInfo FontInfo;
        FontInfo.FontObject = InfosFont;
        FontInfo.Size = 40.f;
        FontInfo.OutlineSettings.OutlineSize = 0;
        Infos1WidgetComponent->SetBlendMode(EWidgetBlendMode::Masked);
        Infos1WidgetComponent->SetLabelFont(FontInfo);
        Infos2WidgetComponent->SetBlendMode(EWidgetBlendMode::Masked);
        Infos2WidgetComponent->SetLabelFont(FontInfo);
    } 
}

void ACheckpoint_Nike::UpdateCheckpoint(FRaceCheckpoint NewCheckpointDatas, FRaceSetup NewRaceSetup)
{
    Super::UpdateCheckpoint(NewCheckpointDatas, NewRaceSetup);
    
    constexpr float HorizontalPadding = 12.f;
    
    const TSharedRef<FSlateFontMeasure> FontMeasure =
            FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    
    if (FootComponent)
    {
        FootComponent->SetColorParameterValueOnMaterials("BaseColor", FLinearColor::FromSRGBColor(FColor::White));
    }
        
    // Name
    float NameHalfBlockWidth = 0;
    if (NameWidgetComponent)
    {
        NameWidgetComponent->SetLabelHJustification(ETextJustify::Left);
        NameWidgetComponent->SetLabelText(FText::FromString(NewCheckpointDatas.name));
        NameWidgetComponent->SetLabelCasseToUpper(FText::FromString(NewCheckpointDatas.name));
        NameWidgetComponent->SetLabelColor(FLinearColor::White);
        
        const FSlateFontInfo& FontInfo = NameWidgetComponent->GetFontInfo();
        const FVector2D TextSizePx =
            FontMeasure->Measure(NewCheckpointDatas.name.ToUpper(), FontInfo);
        
        NameHalfBlockWidth =
            TextSizePx.X * 0.5 + HorizontalPadding;
        
        const FVector NameStartInWidgetLocal(0.f,  NameHalfBlockWidth, -15.f);
        const FVector NameEndInWidgetLocal  (0.f, -NameHalfBlockWidth, -15.f);

        FVector NameStartLocation =
            NameSplineComponent->GetComponentTransform().InverseTransformPosition(
                NameWidgetComponent->GetComponentTransform().TransformPosition(
                NameStartInWidgetLocal
                )
        );
        
        FVector NameLastLocation =
            NameSplineComponent->GetComponentTransform().InverseTransformPosition(
                NameWidgetComponent->GetComponentTransform().TransformPosition(
                NameEndInWidgetLocal
                )
        );
        NameStartLocation.X = 0.f;
        NameLastLocation.X = 0.f;
        
        NameSplineComponent->ClearSplinePoints();

        NameSplineComponent->AddSplinePointAtIndex(
            NameStartLocation,
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
            NameLastLocation,
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
        NameSplineComponent->SetRelativeLocation(FVector(
            0.f, 
            NameSplineComponent->GetRelativeLocation().Y, 
            NameSplineComponent->GetRelativeLocation().Z));
        
        const FVector Delta = NameLastLocation - NameStartLocation;
        const FVector Tan = Delta * 0.5f;
        
        NameBkgComponent->SetStartAndEnd(NameStartLocation, Tan, NameLastLocation, Tan);
        NameBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
        NameBkgComponent->SetStartScale(FVector2D(1.f, 0.6f));
        NameBkgComponent->SetEndScale(FVector2D(1.f, 0.6f));
        NameBkgComponent->UpdateMesh();
        NameBkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor::Black);
        NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
        NameBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
        NameBkgComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
        NameWidgetComponent->SetRelativeLocation(FVector(5.f, 0.f, 21.f));
        NameWidgetComponent->SetRelativeScale3D(FVector(1.f, 1.f, 0.5f));
    }
    
    // Test Longueur texte
    FText DistanceTxt = FText::FromString(FString::Printf(TEXT("Distance %.1f km"),NewCheckpointDatas.distance));
    const FSlateFontInfo& Infos1Infos = Infos1WidgetComponent->GetFontInfo();
    const FVector2D Infos1SizePx =
            FontMeasure->Measure(DistanceTxt, Infos1Infos);
    
    FText ElevationTxt = FText::FromString(FString::Printf(TEXT("Next Station %.0f+ feet"),NewCheckpointDatas.elevation));
    const FSlateFontInfo& Infos2Infos = Infos2WidgetComponent->GetFontInfo();
    const FVector2D Infos2SizePx =
            FontMeasure->Measure(ElevationTxt, Infos2Infos);
    
    float InfosHalfBlockWidth;
        
    if (Infos1SizePx.X > Infos2SizePx.X)
    {
        InfosHalfBlockWidth = Infos1SizePx.X * 0.5 + HorizontalPadding;
    } else
    {
        InfosHalfBlockWidth = Infos2SizePx.X * 0.5 + HorizontalPadding;
    }
    const FVector InfosStartLocationLocal = FVector(0.f,  InfosHalfBlockWidth + 50.f, -15.f);
    const FVector InfosLastLocationLocal  = FVector(0.f, -InfosHalfBlockWidth - 50.f, -15.f);
    StartLocation =
            NameSplineComponent->GetComponentTransform().InverseTransformPosition(
                NameWidgetComponent->GetComponentTransform().TransformPosition(
                InfosStartLocationLocal
                )
            );
   
    LastLocation =
        NameSplineComponent->GetComponentTransform().InverseTransformPosition(
            NameWidgetComponent->GetComponentTransform().TransformPosition(
            InfosLastLocationLocal
            )
        );
    
    StartLocation.X = 0.f;   
    LastLocation.X = 0.f;
    
    if (Infos1WidgetComponent)
    {
        Infos1WidgetComponent->SetLabelText(DistanceTxt);
        Infos1WidgetComponent->SetLabelHJustification(ETextJustify::Left);
        
        Infos1WidgetComponent->SetLabelColor(FLinearColor::Black);
        Infos1SplineComponent->SetRelativeLocation(FVector(0.f, 0.f, -30.f));
        
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
        Infos1BkgComponent->SetStartScale(FVector2D(1.f, 0.25f));
        Infos1BkgComponent->SetEndScale(FVector2D(1.f, 0.25f));
        Infos1BkgComponent->UpdateMesh();
        FVector TxtLoc = FVector(10.f, StartLocation.Y - 20.f, 20.f);
        Infos1WidgetComponent->SetRelativeLocation(TxtLoc);
        Infos1WidgetComponent->SetPivot(FVector2D(0.f, 0.5f));
        Infos1WidgetComponent->SetRelativeScale3D(FVector(1.f, 1.f, 0.5f));
    }
    
    if (Infos1BkgComponent)
    {
        Infos1BkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor::White);
        Infos1BkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
        Infos1BkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
    }
    
    if (Infos2WidgetComponent)
    {
        Infos2WidgetComponent->SetLabelText(ElevationTxt);
        Infos2WidgetComponent->SetLabelHJustification(ETextJustify::Left);
        Infos2WidgetComponent->SetLabelColor(FLinearColor::Black);

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
        Infos2BkgComponent->SetStartScale(FVector2D(1.f, 0.25));
        Infos2BkgComponent->SetEndScale(FVector2D(1.f, 0.25));
        Infos2BkgComponent->UpdateMesh();
        FVector TxtLoc = FVector(10.f, StartLocation.Y - 20.f, 20.f);
        Infos2WidgetComponent->SetRelativeLocation(TxtLoc);
        Infos2WidgetComponent->SetPivot(FVector2D(0.f, 0.5f));
        Infos2WidgetComponent->SetRelativeScale3D(FVector(1.f, 1.f, 0.5f));
    }
    
    if (Infos2BkgComponent)
    {
        Infos2BkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor::White);
        Infos2BkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
        Infos2BkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
    }
    SplineHook->SetRelativeScale3D(FVector(1.f, 1.f, 2.f));
    NameSplineComponent->SetRelativeLocation(FVector(0.f, StartLocation.Y - NameHalfBlockWidth, 28.5f));
    Infos1SplineComponent->SetRelativeLocation(FVector(0.f, LeftY, -30.f));
    Infos2SplineComponent->SetRelativeLocation(FVector(0.f, LeftY, -60.f));
}