// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/UTMB/Km_UTMB.h"

#include "RaceManager.h"
#include "Kismet/GameplayStatics.h"


AKm_UTMB::AKm_UTMB()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> KmMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/Km/Km_UTMB.Km_UTMB"));
	if (KmMesh.Succeeded())
	{
		StaticMeshComp->SetStaticMesh(KmMesh.Object);
	}
}