// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrailSharedTypes.h"
#include "PathSegmentsComponent.generated.h"

/**
 * Segments mis en valeur sur le trace d'une course (onglet Segments de la regie).
 * Chaque segment recolore la portion [KmIn, KmOut] du trace et affiche son nom au-dessus.
 * Aucun objet dessine en plus : les morceaux du trace changent de couleur (Custom Primitive Data),
 * seuls les deux morceaux coupes par chaque borne sont scindes. Un segment visible reste affiche
 * meme quand le trace complet est masque. La ou le trace repasse sur un segment (aller-retour),
 * l'autre passage est masque pour que les deux ne se chevauchent pas.
 */
UCLASS(ClassGroup=(Trail))
class TRAILSIMULATOR_API UPathSegmentsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPathSegmentsComponent();

	/** Remplace les segments de la course. Sans effet visible tant que le trace n'est pas construit. */
	void SetSegments(const TArray<FPathSegment>& InSegments);
	/** Reapplique les segments, par exemple apres le deplacement d'un checkpoint sur lequel une borne est posee */
	void Refresh();
	/** Appele par APath::ChangePathVisibility, apres qu'il a masque ou affiche tout le trace */
	void OnPathVisibilityChanged(bool bShowPath);

	/** Hauteur du texte (cm) */
	UPROPERTY(EditAnywhere, Category="Segments")
	float LabelWorldSize = 4000.f;
	/** Hauteur du bas du fond au-dessus du trace (cm) */
	UPROPERTY(EditAnywhere, Category="Segments")
	float LabelHeight = 3000.f;
	/** Marge du fond autour du texte, en fraction de la hauteur du texte */
	UPROPERTY(EditAnywhere, Category="Segments")
	float LabelPadding = 0.3f;
	/** Epaisseur du trace dans un segment visible, en multiple de l'epaisseur normale */
	UPROPERTY(EditAnywhere, Category="Segments", meta=(ClampMin="1.0", ClampMax="5.0"))
	float HighlightThickness = 1.4f;
	/** Trace qui repasse sur un segment visible (aller-retour) : masque a moins de cette distance du segment (cm, 0 = jamais) */
	UPROPERTY(EditAnywhere, Category="Segments", meta=(ClampMin="0.0"))
	float OverlapRadius = 1500.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void Apply();
	void RestoreTrace();
	void ConfigurePiece(class USplineMeshComponent* Piece, float Key0, float Key1) const;
	class USplineMeshComponent* CreatePiece(class USplineMeshComponent* Original);
	void OnLookAtTimerTick();

	UFUNCTION()
	void UpdateDayNight(bool bIsDay);

	UPROPERTY()
	TArray<FPathSegment> Segments;

	/** Morceaux ajoutes au trace par les coupures (aussi dans APath::PathSplineMeshes) */
	UPROPERTY()
	TArray<TObjectPtr<class USplineMeshComponent>> ExtraPieces;
	/** Morceaux d'origine raccourcis par une coupure */
	TArray<int32> SplitIndices;
	/** Morceaux recolores (dans un segment visible) */
	UPROPERTY()
	TArray<TObjectPtr<class USplineMeshComponent>> HighlightedPieces;
	/** Morceaux du trace masques parce qu'un segment visible passe au meme endroit */
	UPROPERTY()
	TArray<TObjectPtr<class USplineMeshComponent>> OverlappedPieces;
	/** Echelles d'origine (debut, fin) des morceaux recolores, dans l'ordre de HighlightedPieces */
	TArray<TPair<FVector2D, FVector2D>> HighlightedBaseScales;
	UPROPERTY()
	TArray<TObjectPtr<class UTextRenderComponent>> Labels;
	UPROPERTY()
	TArray<TObjectPtr<class UStaticMeshComponent>> Plates;
	UPROPERTY()
	TObjectPtr<class UStaticMesh> PlateMesh;
	UPROPERTY()
	TObjectPtr<class UMaterialInterface> PlateMaterial;

	float LabelIllum = 0.125f;
	FTimerHandle LookAtTimerHandle;
};
