// All Rights Reserved

#include "SeaLevelSubsystem.h"
#include "GeoidGrid.h"
#include "CesiumGeoreference.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

static TAutoConsoleVariable<int32> CVarSeaFlatten(
	TEXT("ts.Sea.Flatten"),
	1,
	TEXT("1 : les tuiles Cesium sous le niveau de la mer sont remontees a ce niveau (mers plates). 0 : relief d'origine."));

void USeaLevelSubsystem::Tick(float DeltaTime)
{
	TimeUntilUpdate -= DeltaTime;
	if (TimeUntilUpdate > 0.f) return;
	TimeUntilUpdate = 0.5f;

	UWorld* World = GetWorld();
	if (!MPC)
	{
		MPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/LTVContent/Materials/Masters/MPC_Trail.MPC_Trail"));
		if (!MPC) { TimeUntilUpdate = 1e9f; return; }
	}
	if (!Georeference.IsValid())
	{
		Georeference = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(World, ACesiumGeoreference::StaticClass()));
	}
	const APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!Georeference.IsValid() || !PC || !PC->PlayerCameraManager) return;

	const FVector Llh = Georeference->TransformUnrealPositionToLongitudeLatitudeHeight(PC->PlayerCameraManager->GetCameraLocation());
	const double SeaM = FGeoidGrid::UndulationM(Llh.X, Llh.Y);
	const FVector SeaPoint = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Llh.X, Llh.Y, SeaM));
	const FVector Up = (Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Llh.X, Llh.Y, SeaM + 1000.0)) - SeaPoint).GetSafeNormal();

	// Rayon de Gauss WGS84 a cette latitude : courbure moyenne de la mer autour de la camera
	const double SinLat = FMath::Sin(FMath::DegreesToRadians(Llh.Y));
	const double E2 = 0.00669437999014, A = 6378137.0;
	const double RadiusM = A * FMath::Sqrt(1.0 - E2) / (1.0 - E2 * SinLat * SinLat);

	const bool bOn = CVarSeaFlatten.GetValueOnGameThread() != 0;
	UKismetMaterialLibrary::SetVectorParameterValue(World, MPC, TEXT("SeaLevelOrigin"), FLinearColor(SeaPoint.X, SeaPoint.Y, SeaPoint.Z, RadiusM * 100.0));
	UKismetMaterialLibrary::SetVectorParameterValue(World, MPC, TEXT("SeaLevelUp"), FLinearColor(Up.X, Up.Y, Up.Z, bOn ? 1.f : 0.f));
}

TStatId USeaLevelSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USeaLevelSubsystem, STATGROUP_Tickables);
}

bool USeaLevelSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
