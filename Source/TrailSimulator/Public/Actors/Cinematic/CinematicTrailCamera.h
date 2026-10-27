// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "CinematicTrailCamera.generated.h"

/**
 * @brief Camera cinema qui survole le rail d'un ACinematicTrail
 * Progress (0 depart, 1 arrivee) et les reglages de cadrage sont animables dans Sequencer : la position
 * ne depend que de ces valeurs (aucun lissage d'une image a l'autre), donc le rendu Movie Render Queue
 * est identique a la previsualisation et se refait a l'identique. Ne pas ajouter de piste Transform.
 */
UCLASS(meta=(DisplayName="Camera cinema sur trace"))
class TRAILSIMULATOR_API ACinematicTrailCamera : public ACineCameraActor
{
	GENERATED_BODY()

public:
	ACinematicTrailCamera(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, Category="Trace cinema")
	TObjectPtr<class ACinematicTrail> Trail;

	/** Position le long du rail : 0 depart, 1 arrivee */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema", meta=(ClampMin=0, ClampMax=1, UIMin=0, UIMax=1))
	float Progress = 0.f;
	/** Hauteur de la camera au-dessus du point suivi (m) */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema")
	float HeightM = 120.f;
	/** Recul derriere le point suivi (m), negatif = devant */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema")
	float BackM = 130.f;
	/** Decalage lateral (m), positif = a droite du sens de course */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema")
	float SideM = 0.f;
	/** Rotation de la camera autour du point suivi (deg) : 0 derriere, 90 sur le cote, 180 face */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema")
	float OrbitDeg = 0.f;
	/** Distance d'anticipation du cap et de la visee (m) */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema", meta=(ClampMin=10))
	float LeadM = 800.f;
	/** Visee : 0 = point moyen en avance sur le trace, 1 = point suivi */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema", meta=(ClampMin=0, ClampMax=1, UIMin=0, UIMax=1))
	float AimAtPivot = 0.f;
	/** Ajout au tangage calcule (deg), negatif = vers le bas */
	UPROPERTY(EditAnywhere, Interp, Category="Trace cinema")
	float PitchOffsetDeg = 0.f;

	/** Place la camera d'apres Progress et les reglages */
	void UpdateFromTrail();
};
