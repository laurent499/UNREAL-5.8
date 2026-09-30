// All Rights Reserved


#include "Components/UTMB/UTMB_FlagComponent.h"

UUTMB_FlagComponent::UUTMB_FlagComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
	PrimaryComponentTick.bCanEverTick = false;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FlagMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Flag_UTMB.Flag_UTMB"));
	if (FlagMesh.Succeeded())
	{
		FlagSMC->SetStaticMesh(FlagMesh.Object);
	}
	FlagSMC->SetRelativeScale3D(FVector(0.6f, 1.f, 0.6f));
}