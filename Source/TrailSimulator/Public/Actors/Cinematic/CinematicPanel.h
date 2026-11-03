// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CinematicPanel.generated.h"

/**
 * @brief Panneau d'etape pour les levels cinema : une image (PNG a fond transparent, point d'ancrage en bas au centre)
 * posee au point du trace, toujours tournee vers la camera. Visible dans le viewport ; Opacite, Taille et Hauteur
 * sont animables dans Sequencer. Cree par le bouton « Creer les panneaux » du Trace cinema.
 */
UCLASS(meta=(DisplayName="Panneau cinema"))
class TRAILSIMULATOR_API ACinematicPanel : public AActor
{
	GENERATED_BODY()

public:
	ACinematicPanel();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	/** Image du panneau (point d'ancrage en bas au centre) */
	UPROPERTY(EditAnywhere, Category="Panneau")
	TObjectPtr<class UTexture2D> Image;
	/** Camera vers laquelle le panneau se tourne et dont la distance regle la taille a l'ecran */
	UPROPERTY(EditAnywhere, Category="Panneau")
	TObjectPtr<AActor> Camera;
	/** 0 invisible, 1 opaque */
	UPROPERTY(EditAnywhere, Interp, Category="Panneau", meta=(ClampMin=0, ClampMax=1, UIMin=0, UIMax=1))
	float Opacity = 1.f;
	/** Taille a l'ecran : hauteur de l'image en fraction de la hauteur de l'image rendue (0,25 = un quart) */
	UPROPERTY(EditAnywhere, Interp, Category="Panneau", meta=(ClampMin=0, UIMin=0, UIMax=1))
	float ScreenSize = 0.22f;
	/** Taille fixe en metres si ScreenSize vaut 0 */
	UPROPERTY(EditAnywhere, Interp, Category="Panneau", meta=(ClampMin=0))
	float SizeM = 300.f;
	/** Decalage vertical du point d'ancrage au-dessus du point du trace (m) */
	UPROPERTY(EditAnywhere, Interp, Category="Panneau")
	float HeightM = 0.f;

	/** Position du point du trace (monde), fixee a la creation */
	UPROPERTY(EditAnywhere, Category="Panneau")
	FVector Anchor = FVector::ZeroVector;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UStaticMeshComponent> Plane;
	UPROPERTY(Transient)
	TObjectPtr<class UMaterialInstanceDynamic> MID;

	void UpdatePanel();
};
