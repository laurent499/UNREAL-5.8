// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "CartoImagerySubsystem.generated.h"

class ACesium3DTileset;
class UCesiumUrlTemplateRasterOverlay;

/**
 * @brief Section "imagery" des styles de rendu : fond cartographique (Mapbox, OSM, URL XYZ).
 * @note Les tuiles photo Google ne reçoivent jamais la carte (conditions Google Maps Platform,
 *       et changer le matériau du tileset forcerait un rechargement complet). En Carto, un relief
 *       Cesium World Terrain portant la carte est créé ; les tilesets photo sont masqués une fois
 *       ce relief chargé, puis réaffichés tels quels au retour en Réaliste (relief détruit).
 *       Clé Mapbox lue localement, jamais dans le JSON ni dans git : variable d'environnement
 *       LTV_MAPBOX_TOKEN, sinon %LOCALAPPDATA%/LTV/mapbox_token.txt.
 *       Contrat : Docs/Styles/CONTRAT_STYLES.md, clés : Docs/Styles/IMAGERY_CARTO.md.
 */
UCLASS()
class SUBSYSTEMS_API UCartoImagerySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void ApplySection(UWorld* World, const TSharedRef<FJsonObject>& Section);
	void RevertSection(UWorld* World);

	/** Relief porteur de la carte, créé au premier besoin (masqué tant qu'il n'est pas chargé). */
	ACesium3DTileset* EnsureTerrain(UWorld* World, ACesium3DTileset* PhotoReference, int64 AssetId);
	/** Masque les tilesets photo (et suspend leur chargement si demandé), affiche le relief. */
	void SwapToTerrain();
	/** Attend que le relief soit chargé avant de masquer les photos (pas d'écran vide en direct). */
	bool TickWaitTerrain(float DeltaTime);
	void StopWaiting();
	void RestorePhotoTilesets();

	FTSTicker::FDelegateHandle WaitHandle;
	float WaitElapsed = 0.f;
	float WaitTimeout = 8.f;
	float WaitProgress = 99.f;
	bool bSuspendPhoto = false;

	TWeakObjectPtr<UWorld> StyledWorld;
	TWeakObjectPtr<ACesium3DTileset> Terrain;
	TWeakObjectPtr<UCesiumUrlTemplateRasterOverlay> MapOverlay;
	/** Tilesets photo que ce style a masqués (et leur état SuspendUpdate d'origine). */
	TArray<TPair<TWeakObjectPtr<ACesium3DTileset>, bool>> HiddenPhoto;
};
