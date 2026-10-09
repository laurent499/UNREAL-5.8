// All Rights Reserved

#include "CesiumBudgetSubsystem.h"
#include "Cesium3DTileset.h"
#include "Cesium3DTilesSelection/Tileset.h"
#include "EngineUtils.h"

static TAutoConsoleVariable<float> CVarCesiumLoadBudgetMs(
	TEXT("ts.Cesium.LoadBudgetMs"),
	2.5f,
	TEXT("Temps max (ms) par image pour construire les tuiles Cesium sur le thread de jeu (Cesium : 5)."));

static TAutoConsoleVariable<float> CVarCesiumUnloadBudgetMs(
	TEXT("ts.Cesium.UnloadBudgetMs"),
	2.5f,
	TEXT("Temps max (ms) par image pour liberer les tuiles Cesium sur le thread de jeu (Cesium : 5)."));

void UCesiumBudgetSubsystem::Tick(float DeltaTime)
{
	TimeUntilApply -= DeltaTime;
	if (TimeUntilApply > 0.f) return;
	TimeUntilApply = 1.f;

	const double LoadMs = FMath::Max(0.5f, CVarCesiumLoadBudgetMs.GetValueOnGameThread());
	const double UnloadMs = FMath::Max(0.5f, CVarCesiumUnloadBudgetMs.GetValueOnGameThread());
	for (TActorIterator<ACesium3DTileset> It(GetWorld()); It; ++It)
	{
		if (Cesium3DTilesSelection::Tileset* Tileset = It->GetTileset())
		{
			Cesium3DTilesSelection::TilesetOptions& Options = Tileset->getOptions();
			Options.mainThreadLoadingTimeLimit = LoadMs;
			Options.tileCacheUnloadTimeLimit = UnloadMs;
		}
	}
}

TStatId UCesiumBudgetSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCesiumBudgetSubsystem, STATGROUP_Tickables);
}

bool UCesiumBudgetSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
