// All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"

/**
 * Textes des runners : couleur brute, identique de jour comme de nuit.
 * M_RawText est un materiau Unlit dessine APRES le tonemapper (Translucency After Motion Blur) :
 * ni l'eclairage, ni l'exposition, ni la LUT, ni le bloom, ni le brouillard ne le modifient.
 * La couleur affichee est exactement celle passee a SetTextRenderColor.
 */
namespace TrailRawText
{
	inline UMaterialInterface* GetMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LTVContent/Materials/Masters/M_RawText.M_RawText"));
		}
		return Cached.Get();
	}

	/**
	 * Fait ecrire dans la CustomDepth tous les elements d'un acteur sauf ses textes : M_RawText
	 * (dessine apres le tonemapper, sans test de profondeur natif) s'en sert pour se masquer
	 * derriere les cartes translucides, qui n'ecrivent pas dans la profondeur normale.
	 */
	inline void EnableOcclusion(AActor* Owner)
	{
		if (!Owner)
		{
			return;
		}
		// Les elements de NameComponent / ClubComponent... sont des sous-objets dont l'Outer est le
		// composant, pas l'acteur : GetComponents() ne les voit pas. On parcourt donc toute la
		// hierarchie attachee (cartes, photos, drapeaux, fonds), a chaque appel.
		TArray<USceneComponent*> Attached;
		if (USceneComponent* Root = Owner->GetRootComponent())
		{
			Root->GetChildrenComponents(/*bIncludeAllDescendants=*/true, Attached);
		}
		for (USceneComponent* Component : Attached)
		{
			UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component);
			if (Primitive && !Primitive->IsA<UTextRenderComponent>())
			{
				Primitive->SetRenderCustomDepth(true);
			}
		}
	}

	/**
	 * Applique le materiau brut a TOUS les textes d'un acteur (checkpoints, POI, km...) en gardant
	 * la couleur que le code leur a deja donnee (SetTextRenderColor) : plus de degrade, plus de
	 * bloom, meme rendu de jour comme de nuit.
	 */
	inline void ApplyToAllTexts(AActor* Owner)
	{
		UMaterialInterface* Material = GetMaterial();
		if (!Owner || !Material)
		{
			return;
		}
		TArray<USceneComponent*> All;
		if (USceneComponent* Root = Owner->GetRootComponent())
		{
			Root->GetChildrenComponents(/*bIncludeAllDescendants=*/true, All);
		}
		for (USceneComponent* Component : All)
		{
			if (UTextRenderComponent* Text = Cast<UTextRenderComponent>(Component))
			{
				if (Text->TextMaterial != Material)
				{
					Text->SetTextMaterial(Material);
				}
			}
		}
	}

	/** Applique le materiau brut et la couleur. Idempotent : peut etre rappele a chaque mise a jour du runner. */
	inline void Apply(UTextRenderComponent* Text, const FColor& Color)
	{
		if (!Text)
		{
			return;
		}
		Text->SetTextRenderColor(Color);
		if (UMaterialInterface* Material = GetMaterial())
		{
			if (Text->TextMaterial != Material)
			{
				Text->SetTextMaterial(Material);
			}
		}
	}
}
