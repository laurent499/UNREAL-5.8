// Copyright LTVProd 2026 All Rights Reserved.


#include "Components/Nike/Nike_NameComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TrailTextWidgetComponent.h"

UNike_NameComponent::UNike_NameComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> NameMesh(
        TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Bkg_Plain.Bkg_Plain"));
    if (NameMesh.Succeeded())
    {
        NameBkgComponent->SetStaticMesh(NameMesh.Object);
    }
    
    NameBkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor(1.0, 1.0, 1.0,1.0));
    NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
    NameBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
}

void UNike_NameComponent::UpdateName(const FString& RunnerName)
{
    Super::UpdateName(RunnerName);
    
    if (NameTextComponent)
    {
        NameTextComponent->SetText(FText::FromString(RunnerName));
        FVector NameLocalSize = NameTextComponent->GetTextLocalSize();
		
        StartLocation = FVector(0.f, NameLocalSize.Y / 2 + 150.f, 0.f);
        LastLocation = FVector(0.f, -NameLocalSize.Y / 2 - 150.f, 0.f);
        
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
        NameBkgComponent->SetStartScale(FVector2D(1.0f, 1.f));
        NameBkgComponent->SetEndScale(FVector2D(1.0f, 1.f));
        NameBkgComponent->UpdateMesh();
    }
    NameTextComponent->SetHiddenInGame(!bShow3DText);
    
    if (WidgetNameComponent)
    {
        WidgetNameComponent->SetRelativeLocation(FVector(10.f, 0.f, 60.f));
        WidgetNameComponent->SetLabelText(FText::FromString(RunnerName));
        WidgetNameComponent->SetLabelCasseToUpper(FText::FromString(RunnerName));
        WidgetNameComponent->SetLabelHJustification(ETextJustify::Center);
        WidgetNameComponent->SetLabelColor(FColor::Black);
    }
    WidgetNameComponent->SetHiddenInGame(!bShowWidgetText);
}