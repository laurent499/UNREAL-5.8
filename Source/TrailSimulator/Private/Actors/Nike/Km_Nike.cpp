// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Nike/Km_Nike.h"

AKm_Nike::AKm_Nike()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> KmMesh(
        TEXT("/Game/LTVContent/Meshes/SM/Stack/Km/Km_Gen.Km_Gen"));
    if (KmMesh.Succeeded())
    {
        StaticMeshComp->SetStaticMesh(KmMesh.Object);
    }
}
