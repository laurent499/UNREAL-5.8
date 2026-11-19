// All Rights Reserved

#include "CesiumBudgetSubsystem.h"
#include "Cesium3DTileset.h"
#include "Cesium3DTilesSelection/Tileset.h"
#include "EngineUtils.h"
#include "Misc/CoreDelegates.h"
#include "Misc/OutputDevice.h"
#include <atomic>

static TAutoConsoleVariable<float> CVarCesiumLoadBudgetMs(
	TEXT("ts.Cesium.LoadBudgetMs"),
	2.5f,
	TEXT("Temps max (ms) par image pour construire les tuiles Cesium sur le thread de jeu (Cesium : 5)."));

static TAutoConsoleVariable<float> CVarCesiumUnloadBudgetMs(
	TEXT("ts.Cesium.UnloadBudgetMs"),
	2.5f,
	TEXT("Temps max (ms) par image pour liberer les tuiles Cesium sur le thread de jeu (Cesium : 5)."));

static TAutoConsoleVariable<bool> CVarCesiumAutoReload(
	TEXT("ts.Cesium.AutoReload"),
	false,
	TEXT("Recharge automatiquement les tilesets Google quand leurs tuiles sont refusees (session Google expiree, HTTP 400/401/403). "
		 "Coupe par defaut : le decor disparait quelques secondes a l'antenne, le rechargement se fait depuis la regie."));

static TAutoConsoleVariable<int32> CVarCesiumAutoReloadErrors(
	TEXT("ts.Cesium.AutoReloadErrors"),
	10,
	TEXT("Nombre de tuiles refusees en 30 s qui declenche le rechargement automatique."));

static TAutoConsoleVariable<float> CVarCesiumAutoReloadCooldownS(
	TEXT("ts.Cesium.AutoReloadCooldownS"),
	120.f,
	TEXT("Delai minimal (s) entre deux rechargements automatiques."));

namespace CesiumReload
{
	// Compte les tuiles refusees par le serveur. Google ouvre une session au chargement du tileset
	// (parametre session= de chaque URL) ; quand elle expire, toutes les nouvelles tuiles repondent 400
	// et plus rien ne se charge. Recreer le tileset ouvre une nouvelle session.
	std::atomic<int32> RejectedTiles{0};

	class FRejectedTileLogWatcher final : public FOutputDevice
	{
	public:
		virtual bool CanBeUsedOnAnyThread() const override { return true; }
		virtual bool CanBeUsedOnMultipleThreads() const override { return true; }
		virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
		{
			static const FName LogCesiumName(TEXT("LogCesium"));
			if (Category != LogCesiumName || !V) return;
			if (FCString::Strstr(V, TEXT("Received status code 400"))
				|| FCString::Strstr(V, TEXT("Received status code 401"))
				|| FCString::Strstr(V, TEXT("Received status code 403")))
			{
				RejectedTiles.fetch_add(1, std::memory_order_relaxed);
			}
		}
	};

	TUniquePtr<FRejectedTileLogWatcher> Watcher;
	float TimeSinceAutoReload = 1.e6f;
	float TimeInErrorWindow = 0.f;

	void EnsureWatcher()
	{
		if (Watcher.IsValid() || !GLog) return;
		Watcher = MakeUnique<FRejectedTileLogWatcher>();
		GLog->AddOutputDevice(Watcher.Get());
		FCoreDelegates::OnPreExit.AddLambda([]()
		{
			if (Watcher.IsValid() && GLog) GLog->RemoveOutputDevice(Watcher.Get());
			Watcher.Reset();
		});
	}

	bool IsGoogleTileset(const ACesium3DTileset& Tileset)
	{
		return Tileset.GetIonAssetID() == 2275207 || Tileset.GetUrl().Contains(TEXT("tile.googleapis.com"));
	}

	// Recharge les tilesets du monde (bGoogleOnly : seulement les tuiles photorealistes Google).
	int32 ReloadTilesets(UWorld* World, bool bGoogleOnly, const TCHAR* Reason)
	{
		if (!World) return 0;
		int32 Count = 0;
		for (TActorIterator<ACesium3DTileset> It(World); It; ++It)
		{
			if (bGoogleOnly && !IsGoogleTileset(**It)) continue;
			It->RefreshTileset();
			++Count;
		}
		RejectedTiles.store(0, std::memory_order_relaxed);
		UE_LOG(LogTemp, Warning, TEXT("[Cesium] %d tileset(s) recharge(s) : %s"), Count, Reason);
		return Count;
	}
}

static FAutoConsoleCommandWithWorldAndArgs CmdCesiumReload(
	TEXT("ts.Cesium.Reload"),
	TEXT("Recharge les tuiles Cesium sans relancer le jeu (nouvelle session Google). Argument google = seulement les tuiles Google."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const bool bGoogleOnly = Args.Num() > 0 && Args[0].Equals(TEXT("google"), ESearchCase::IgnoreCase);
		CesiumReload::ReloadTilesets(World, bGoogleOnly, TEXT("commande"));
	}));

void UCesiumBudgetSubsystem::Tick(float DeltaTime)
{
	CesiumReload::EnsureWatcher();
	float& TimeSinceAutoReload = CesiumReload::TimeSinceAutoReload;
	float& TimeInErrorWindow = CesiumReload::TimeInErrorWindow;
	TimeSinceAutoReload += DeltaTime;
	TimeInErrorWindow += DeltaTime;
	if (TimeInErrorWindow >= 30.f)
	{
		const int32 Rejected = CesiumReload::RejectedTiles.exchange(0, std::memory_order_relaxed);
		TimeInErrorWindow = 0.f;
		if (Rejected >= FMath::Max(1, CVarCesiumAutoReloadErrors.GetValueOnGameThread()))
		{
			if (!CVarCesiumAutoReload.GetValueOnGameThread())
			{
				UE_LOG(LogTemp, Warning, TEXT("[Cesium] %d tuiles refusees en 30 s (session Google expiree ?) : rechargement auto coupe, recharger depuis la regie (ts.Cesium.Reload)"), Rejected);
			}
			else if (TimeSinceAutoReload >= CVarCesiumAutoReloadCooldownS.GetValueOnGameThread())
			{
				TimeSinceAutoReload = 0.f;
				CesiumReload::ReloadTilesets(GetWorld(), true, *FString::Printf(TEXT("auto, %d tuiles refusees en 30 s"), Rejected));
			}
		}
	}

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
