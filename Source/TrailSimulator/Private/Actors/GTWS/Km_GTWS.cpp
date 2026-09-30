// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/GTWS/Km_GTWS.h"


// Sets default values
AKm_GTWS::AKm_GTWS()
{	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> KmMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/Km/Km_Gen.Km_Gen"));
	if (KmMesh.Succeeded())
	{
		StaticMeshComp->SetStaticMesh(KmMesh.Object);
	}
}
