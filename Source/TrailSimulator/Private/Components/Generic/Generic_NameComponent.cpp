// All Rights Reserved


#include "Components/Generic/Generic_NameComponent.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"


UGeneric_NameComponent::UGeneric_NameComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	NameLeftComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("SlashLeft"));
	NameLeftComponent->SetupAttachment(NameSplineComponent);
	NameLeftComponent->SetRelativeTransform(FTransform::Identity);
	NameLeftComponent->SetRelativeLocation(FVector(-20.0f, 100.0f, 0.0f));
	NameLeftComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameLeftMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Generic_Name_Left.Generic_Name_Left"));
	if (NameLeftMesh.Succeeded())
	{
		NameLeftComponent->SetStaticMesh(NameLeftMesh.Object);
		NameLeftComponent->SetCustomPrimitiveDataFloat(10, 0.f); // Slope value
		NameLeftComponent->SetCustomPrimitiveDataFloat(11, 0.f); // UseSlope = 0	M_MasterPC
		NameLeftComponent->SetCustomPrimitiveDataVector4(12, FLinearColor::White); // Color (12->15)
		NameLeftComponent->SetCustomPrimitiveDataFloat(16, 0.5f);	// Illum
	}
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Generic_Name.Generic_Name"));
	if (NameMesh.Succeeded())
	{
		NameBkgComponent->SetStaticMesh(NameMesh.Object);
		NameBkgComponent->SetCustomPrimitiveDataFloat(10, 0.f); // Slope value
		NameBkgComponent->SetCustomPrimitiveDataFloat(11, 0.f); // UseSlope = 0	M_MasterPC
		NameBkgComponent->SetCustomPrimitiveDataVector4(12, FLinearColor::White); // Color (12->15)
		NameBkgComponent->SetCustomPrimitiveDataFloat(16, 0.5f);	// Illum
	}
	
	NameRightComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("SlashRight"));
	NameRightComponent->SetupAttachment(NameSplineComponent);
	NameRightComponent->SetRelativeTransform(FTransform::Identity);
	NameRightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameRightMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Generic_Name_Right.Generic_Name_Right"));
	if (NameRightMesh.Succeeded())
	{
		NameRightComponent->SetStaticMesh(NameRightMesh.Object);
		NameRightComponent->SetCustomPrimitiveDataFloat(10, 0.f); // Slope value
		NameRightComponent->SetCustomPrimitiveDataFloat(11, 0.f); // UseSlope = 0	M_MasterPC
		NameRightComponent->SetCustomPrimitiveDataVector4(12, FLinearColor::White); // Color (12->15)
		NameRightComponent->SetCustomPrimitiveDataFloat(16, 0.5f);	// Illum
	}
	
	NameTextComponent->SetTextRenderColor(FColor(255, 255, 255, 255));
	// NameTextComponent->SetTextRenderColor(FColor(382, 382, 382, 255));
}

void UGeneric_NameComponent::UpdateName(const FString& RunnerName)
{
	Super::UpdateName(RunnerName);
	NameTextComponent->SetWorldSize(70.f);
	NameTextComponent->SetRelativeLocation(FVector(5.0f,40.0f, 39.0f));
	NameTextComponent->SetHorizontalAlignment(EHTA_Left);
	
	FVector NameLocalSize = NameTextComponent->GetTextLocalSize();
		
	StartLocation = FVector(0.f, 100.f, 0.f);
	LastLocation = FVector(0.f, -NameLocalSize.Y-50.f, 0.f);
	if (NameSplineComponent->GetNumberOfSplinePoints()>=2){
		NameSplineComponent->ClearSplinePoints();
		NameSplineComponent->AddSplinePointAtIndex(StartLocation, 0, ESplineCoordinateSpace::Local, false);
		NameSplineComponent->AddSplinePointAtIndex(LastLocation, 1, ESplineCoordinateSpace::Local, true);
	}
	
	NameBkgComponent->SetStartAndEnd(
		NameSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local), 
		NameSplineComponent->GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local),
		NameSplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local),
		NameSplineComponent->GetTangentAtSplinePoint(1, ESplineCoordinateSpace::Local));
	NameBkgComponent->SetForwardAxis(ESplineMeshAxis::X);
	NameBkgComponent->SetStartScale(FVector2D(1.0f, 1.0f));
	NameBkgComponent->SetEndScale(FVector2D(1.0f, 1.0f));
	NameBkgComponent->UpdateMesh();
	
	NameRightComponent->SetRelativeLocation(FVector(
		GetSplineComponent()->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).X - 20.f,
		GetSplineComponent()->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).Y,
		GetSplineComponent()->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).Z));
	NameRightComponent->SetRelativeRotation(FRotator(0.0f, -270.0f, 0.0f));
	
}

float UGeneric_NameComponent::GetMiddle() const
{
	return (NameLeftComponent->GetRelativeLocation().Y+100.f + 
	GetSplineComponent()->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local).Y-20.f) * -0.5;
}
