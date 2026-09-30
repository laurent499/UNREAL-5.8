// Copyright LTVProd 2026 All Rights Reserved.


#include "Components/Nike/Nike_FlagComponent.h"


UNike_FlagComponent::UNike_FlagComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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