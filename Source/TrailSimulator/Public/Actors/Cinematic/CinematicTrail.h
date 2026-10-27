// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CinematicTrail.generated.h"

/**
 * @brief Trace d'une course pour les levels cinema (rendu Movie Render Queue, hors live)
 * Charge race/<id>/path depuis la regie dans l'editeur (bouton Charger la course), garde les points
 * avec le level et redessine le trace et le rail de camera a chaque construction : rien ne depend
 * des subsystems du live, le trace est la en edition comme pendant le rendu.
 */
UCLASS(meta=(DisplayName="Trace cinema"))
class TRAILSIMULATOR_API ACinematicTrail : public AActor
{
	GENERATED_BODY()

public:
	ACinematicTrail();
	virtual void OnConstruction(const FTransform& Transform) override;

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

	void BuildTrailMeshes();
};
