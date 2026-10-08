// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "WaterMaskSubsystem.generated.h"

/**
 * @brief Zone geographique (degres) couverte par les donnees d'eau OSM d'une course
 */
struct FWaterMaskBounds
{
	double South = 0.0;
	double West = 0.0;
	double North = 0.0;
	double East = 0.0;
};

/**
 * @brief Subsystem qui charge automatiquement les plans d'eau OpenStreetMap
 * autour du trace de chaque course (lacs, rivieres, torrents) et les applique
 * au tileset Cesium sous forme d'overlay GeoJSON (cle de couche "WaterOSM").
 *
 * La zone est deduite des points du trace recus de l'API, avec une marge.
 * Les donnees sont mises en cache dans Saved/WaterCache pour ne pas rappeler
 * l'API Overpass a chaque lancement. Le masque mondial JRC reste actif en fond.
 */
UCLASS()
class SUBSYSTEMS_API UWaterMaskSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Cle de couche utilisee par le materiau M_CesiumGlobalWater (parametres WaterOSM_*) */
	static const FString MaterialLayerKey;

	/** Le point (degres) est-il dans un plan d'eau OSM d'une course chargee ? (flore, faune...) */
	bool IsWaterAt(double Longitude, double Latitude);

private:
	/** Polygones d'eau decodes une fois, avec leur emprise, pour les requetes IsWaterAt */
	struct FWaterPolygon
	{
		FBox2D Bounds = FBox2D(ForceInit);
		TArray<TArray<FVector2D>> Rings; // anneau exterieur puis trous (regle pair-impair)
	};
	void RebuildWaterPolygons();
	TArray<FWaterPolygon> WaterPolygons;
	bool bWaterPolygonsDirty = true;

	UFUNCTION()
	void HandlePathDatasGathered(int64 RaceID, FRacePath RacePath);

	static bool ComputeBounds(const FRacePath& RacePath, double MarginKm, FWaterMaskBounds& OutBounds);
	static FString GetCacheFilePath(const FWaterMaskBounds& Bounds);

	void RequestOverpass(int64 RaceID, const FWaterMaskBounds& Bounds, const FString& CacheFile, int32 ServerIndex = 0);
	void SendOverpassRequest(int64 RaceID, const FWaterMaskBounds& Bounds, const FString& CacheFile, int32 ServerIndex);
	void OnRaceWaterReady(int64 RaceID, FString&& FeaturesJson);
	void ApplyToTilesets();

	UPROPERTY()
	TObjectPtr<class UPathSubsystem> PathSubsystem;

	/** Features GeoJSON (tableau JSON sans crochets) par course */
	TMap<int64, FString> RaceFeatures;

	TMap<int64, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ActiveRequests;
};
