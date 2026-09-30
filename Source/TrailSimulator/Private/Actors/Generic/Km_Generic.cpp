// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Generic/Km_Generic.h"


// Sets default values
AKm_Generic::AKm_Generic()
{	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> KmMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/Km/Km_Gen.Km_Gen"));
	if (KmMesh.Succeeded())
	{
		StaticMeshComp->SetStaticMesh(KmMesh.Object);
	}
}
