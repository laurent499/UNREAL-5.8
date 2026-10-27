// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrailSharedTypes.h"
#include "CinematicTrail.generated.h"

/**
 * @brief Trace d'une course pour les levels cinema (rendu Movie Render Queue, hors live)
 * Charge le trace, le setup, les checkpoints et les POI de la course depuis la regie dans l'editeur
 * (bouton Charger la course) et les garde avec le level. Le trace et le rail sont redessines a chaque
 * construction ; dans l'editeur, checkpoints et POI ne sont que des reperes nommes. Au lancement du
 * rendu (PIE de Movie Render Queue), les vrais acteurs du live apparaissent avec le template de la course.
 */
UCLASS(meta=(DisplayName="Trace cinema"))
class TRAILSIMULATOR_API ACinematicTrail : public AActor
{
	GENERATED_BODY()

public:
	ACinematicTrail();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	/** Id de la course dans la regie */
	UPROPERTY(EditAnywhere, Category="Course")
	int64 RaceID = 0;
	UPROPERTY(EditAnywhere, Category="Course")
	FString MainURL = TEXT("https://simulacre.ltvprod.cc/");
	/** Decalage vertical du trace (m), comme le ZOffset de la regie */
	UPROPERTY(EditAnywhere, Category="Course")
	float HeightOffsetM = 0.f;
	/** Recentre le geo-referencement Cesium sur le depart au chargement (precision des coordonnees) */
	UPROPERTY(EditAnywhere, Category="Course")
	bool bCenterGeoreferenceOnStart = true;

	UPROPERTY(EditAnywhere, Category="Trace")
	bool bShowTrail = true;
	UPROPERTY(EditAnywhere, Category="Trace")
	FLinearColor TrailColor = FLinearColor(1.f, 0.35f, 0.f);
	/** Echelle du cylindre du trace, 50 comme en live */
	UPROPERTY(EditAnywhere, Category="Trace", meta=(ClampMin=1))
	float TrailWidth = 50.f;
	UPROPERTY(EditAnywhere, Category="Trace", meta=(ClampMin=0))
	float Glow = 1.f;
	/** Longueur d'un morceau de trace (m) : plus court = plus fidele, plus de composants */
	UPROPERTY(EditAnywhere, Category="Trace", meta=(ClampMin=5))
	float TrailStepM = 25.f;

	/** Fait apparaitre au rendu les checkpoints du live (classe selon le template de la course) */
	UPROPERTY(EditAnywhere, Category="Panneaux")
	bool bSpawnCheckpoints = true;
	/** Fait apparaitre au rendu les POI du live */
	UPROPERTY(EditAnywhere, Category="Panneaux")
	bool bSpawnPois = true;
	/** Taille du texte des reperes de l'editeur (cm) */
	UPROPERTY(EditAnywhere, Category="Panneaux", meta=(ClampMin=100))
	float MarkerTextSize = 4000.f;

	/** Lissage du rail suivi par la camera (m), meme principe que le survol live */
	UPROPERTY(EditAnywhere, Category="Rail", meta=(ClampMin=1))
	float RailSmoothM = 250.f;

	/** Telecharge le trace de RaceID depuis la regie et le garde dans le level */
	UFUNCTION(CallInEditor, Category="Course", meta=(DisplayName="Charger la course"))
	void LoadRace();
	/** Redessine le trace (apres un deplacement manuel du geo-referencement par exemple) */
	UFUNCTION(CallInEditor, Category="Course", meta=(DisplayName="Recalculer"))
	void Rebuild();

	class USplineComponent* GetRail() const { return Rail; }
	bool HasTrail() const { return PointsLLH.Num() >= 2; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USceneComponent> Root;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USplineComponent> Trail;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USplineComponent> Rail;
	UPROPERTY()
	TObjectPtr<class UStaticMesh> TrailMesh;
	UPROPERTY()
	TArray<TObjectPtr<class USplineMeshComponent>> TrailMeshes;

	/** Points du trace (lon, lat, altitude GPS en m), sauvegardes avec le level */
	UPROPERTY()
	TArray<FVector> PointsLLH;
	/** Setup de la course (couleur, template), sauvegarde avec le level */
	UPROPERTY(VisibleAnywhere, Category="Course")
	FRaceSetup RaceSetup;
	/** Checkpoints du trace et index du point du trace ou ils sont poses */
	UPROPERTY()
	TArray<FRaceCheckpoint> Checkpoints;
	UPROPERTY()
	TArray<int32> CheckpointPointIndices;
	UPROPERTY()
	TArray<FRacePOI> Pois;

	/** Reperes de l'editeur, recrees a chaque construction */
	UPROPERTY(Transient)
	TArray<TObjectPtr<class UTextRenderComponent>> Markers;

	void BuildTrailMeshes();
	void BuildMarkers();
	FVector GetCheckpointLocation(int32 Index) const;
	FVector GetPoiLocation(const FRacePOI& Poi) const;
	void SpawnRaceActors();
};
