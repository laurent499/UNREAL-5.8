// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CinematicGeoAnchor.generated.h"

/**
 * @brief Cale un nuage de points (ou un mesh) exprime en Lambert-93 / IGN69 sur le globe Cesium
 * Pour un nuage LiDAR importe (LAS, LAZ, E57), l'origine Lambert est lue dans l'asset ; pour un mesh
 * (OBJ, PLY), on donne les coordonnees Lambert-93 de son point (0, 0, 0), axes X est, Y sud, Z haut, en cm
 * (convention de l'import LiDAR). Le calage tient compte de la convergence des meridiens et du facteur
 * d'echelle de la projection : la cible recoit une transformation complete, pas seulement une position.
 */
UCLASS(meta=(DisplayName="Calage Lambert-93"))
class TRAILSIMULATOR_API ACinematicGeoAnchor : public AActor
{
	GENERATED_BODY()

public:
	ACinematicGeoAnchor();

	/** Acteur a caler : nuage LiDAR (LidarPointCloudActor) ou mesh */
	UPROPERTY(EditAnywhere, Category="Calage")
	TObjectPtr<AActor> Target;
	/** Sans nuage LiDAR : coordonnees Lambert-93 (m : est, nord, altitude IGN69) du point (0, 0, 0) de la cible */
	UPROPERTY(EditAnywhere, Category="Calage")
	FVector ManualLambert93Origin = FVector::ZeroVector;
	/** Correction d'altitude (m), si l'ecart geoide IGN69 / EGM96 se voit contre le relief Cesium */
	UPROPERTY(EditAnywhere, Category="Calage")
	float HeightOffsetM = 0.f;

	/** Tileset a decouper sous la cible (vide : le premier du level) */
	UPROPERTY(EditAnywhere, Category="Decoupe Cesium")
	TObjectPtr<class ACesium3DTileset> Tileset;
	/** Contour de la decoupe (Lambert-93 : est, nord en m), par exemple celui d'un mesh issu de laz_to_mesh.py ;
	 * vide : rectangle d'emprise de la cible */
	UPROPERTY(EditAnywhere, Category="Decoupe Cesium")
	TArray<FVector2D> ClipOutlineLambert93;
	/** Marge autour de l'emprise de la cible (m) */
	UPROPERTY(EditAnywhere, Category="Decoupe Cesium")
	float ClipMarginM = 0.f;

	/** Centre de la cible apres calage (longitude, latitude en degres, hauteur ellipsoidale en m) */
	UPROPERTY(VisibleAnywhere, Category="Calage")
	FVector CenterLonLatHeight = FVector::ZeroVector;

	UFUNCTION(CallInEditor, Category="Calage", meta=(DisplayName="Caler sur Cesium"))
	void Align();
	/** Masque les tuiles Cesium sous l'emprise de la cible (polygone + overlay de decoupe sur le tileset) */
	UFUNCTION(CallInEditor, Category="Decoupe Cesium", meta=(DisplayName="Decouper Cesium"))
	void ClipCesium();

	/** Lambert-93 (m) vers longitude / latitude RGF93 (degres), assimile a WGS84 */
	static FVector2D Lambert93ToLonLat(double East, double North);

private:
	UPROPERTY(VisibleAnywhere, Category="Decoupe Cesium")
	TObjectPtr<class ACesiumCartographicPolygon> ClipPolygon;

	/** Point local de la cible (cm) vers Lambert-93 (m) ; faux si la cible manque */
	bool LocalToLambert(const FVector& Local, FVector& OutLambert) const;
	/** Boite locale de la cible (cm) */
	FBox GetLocalBounds() const;
	FVector LambertToUnreal(const FVector& Lambert) const;
};
