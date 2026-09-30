// All Rights Reserved


#include "Components/Generic/Generic_FlagComponent.h"


// Sets default values for this component's properties
UGeneric_FlagComponent::UGeneric_FlagComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Flag
	FlagHook = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("FlagHook"));
	FlagHook->SetupAttachment(this);
	FlagHook->SetRelativeTransform(FTransform::Identity);
	FlagHook->SetRelativeLocation(FVector(-20.f, 145.f, 0.f));
	
	FlagSMC->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	FlagSMC->SetupAttachment(FlagHook);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FlagMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Generic_Flag.Generic_Flag"));
	if (FlagMesh.Succeeded())
	{
		FlagSMC->SetStaticMesh(FlagMesh.Object);
		FlagSMC->SetCustomPrimitiveDataFloat(10, 0.f); // Slope value
		FlagSMC->SetCustomPrimitiveDataFloat(11, 0.f); // UseSlope = 0	M_MasterPC
		FlagSMC->SetCustomPrimitiveDataVector4(12, FLinearColor::White); // Color (12->15)
		FlagSMC->SetCustomPrimitiveDataFloat(16, 0.5f);	// Illum
	}
}