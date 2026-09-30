// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Nike/Runner_Nike.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Components/Nike/Nike_ClubComponent.h"
#include "Components/Nike/Nike_FlagComponent.h"
#include "Components/Nike/Nike_NameComponent.h"
#include "Engine/Font.h"


ARunner_Nike::ARunner_Nike()
{    
   PresentationRoot->SetRelativeLocation(FVector(0.0f, 0.0f, 700.0f));
    NameComponent = CreateDefaultSubobject<UNike_NameComponent>(TEXT("NikeNameComponent"));
    NameComponent->SetupAttachment(NameHook);
    NameComponent->SetComponentTickEnabled(false);
    NameComponent->SetRelativeTransform(FTransform::Identity);
    NameComponent->ShowWidgetName(true);
    NameComponent->Show3DName(false);
    
    // Club
    ClubHook = CreateDefaultSubobject<USceneComponent>(TEXT("ClubHook"));
    ClubHook->SetupAttachment(FootHook);
    ClubHook->SetComponentTickEnabled(false);
    ClubHook->SetRelativeTransform(FTransform::Identity);
    ClubHook->SetRelativeLocation(FVector(0.f, 0.f, -6.f));
    ClubHook->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    
    ClubComponent = CreateDefaultSubobject<UNike_ClubComponent>(TEXT("ClubComponent"));
    ClubComponent->SetupAttachment(ClubHook);
    ClubComponent->SetRelativeTransform(FTransform::Identity);
    ClubComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.f));
    ClubComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    ClubComponent->ShowWidgetName(true);
    ClubComponent->Show3DName(false);
        
    ConstructorHelpers::FObjectFinder<UFont> TextFont(
         TEXT("/Game/LTVContent/2D/Fonts/Nike/HelveticaNeue-Bold-02_Font.HelveticaNeue-Bold-02_Font"));
    if (TextFont.Succeeded())
    {
        LabelFont = TextFont.Object;
        FSlateFontInfo FontInfo;
        FontInfo.FontObject = LabelFont;
        FontInfo.Size = 40.f;
         FontInfo.OutlineSettings.OutlineSize = 0;
        ClubComponent->WidgetClubComponent->SetLabelFont(FontInfo);
        ClubComponent->WidgetClubComponent->SetBlendMode(EWidgetBlendMode::Masked);
        FontInfo.Size = 60.f;
        NameComponent->WidgetNameComponent->SetLabelFont(FontInfo);
        NameComponent->WidgetNameComponent->SetBlendMode(EWidgetBlendMode::Masked);
    } 
    
    // Flag
    FlagComponent = CreateDefaultSubobject<UNike_FlagComponent>(TEXT("FlagComponent"));
    FlagComponent->SetupAttachment(ClubComponent);
    FlagComponent->SetComponentTickEnabled(false);
    FlagComponent->SetRelativeTransform(FTransform::Identity);
    FlagComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

void ARunner_Nike::UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup)
{
    Super::UpdateRunner(Runner, RaceSetup);
    if (FootComponent)
        FootComponent->SetColorParameterValueOnMaterials("BaseColor", FLinearColor::FromSRGBColor(FColor::White));
    
    if (NameComponent)
    {
        NameComponent->UpdateName(Runner.nom);
    }
    if (ClubComponent && IsClubVisible())
    {
        ClubComponent->SetRelativeLocation(FVector(0.0f, -50.0f, -73.0f));
        ClubComponent->UpdateClub(Runner);
    }
    if (FlagComponent)
    {
        FVector FlagLocation = FVector(40.f, ClubComponent->WidgetClubComponent->GetCurrentDrawSize().X / 2 + 10.f, 20.f);
        if (Runner.club.IsEmpty())
        {
            FlagLocation.Y = 10.f;
        }
        FlagComponent->UpdateFlag(Runner.pays);
        FlagComponent->SetRelativeLocation(FlagLocation);
        FlagComponent->SetRelativeScale3D(FVector(0.45f, 1.f, 0.45f));
    }
}