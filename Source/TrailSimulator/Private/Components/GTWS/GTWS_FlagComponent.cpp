// All Rights Reserved


#include "Components/GTWS/GTWS_FlagComponent.h"


UGTWS_FlagComponent::UGTWS_FlagComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FlagMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/GTWS_Flag.GTWS_Flag"));
	if (FlagMesh.Succeeded())
	{
		FlagSMC->SetStaticMesh(FlagMesh.Object);
	}

}