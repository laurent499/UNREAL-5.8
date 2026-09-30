// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TrailSharedTypes.h"

/**
 * @brief Style partagé du template GTWS.
 * @note Point unique de réglage : la couleur de fond pilote le ClubBkgComponent des runners
 *       ainsi que le NameBkgComponent des checkpoints et des POIs. Les marges pilotent
 *       la largeur commune aux fonds du nom et du club.
 */
namespace GTWSStyle
{
	/** Couleur de fond du template, en hexa sRGB (#RRGGBBAA). Seule valeur à modifier. */
	inline const TCHAR* BkgColorHex = TEXT("#000000ff");

	/** Couleur convertie en linéaire, à passer aux custom primitive data (slot 0). */
	inline FLinearColor GetBkgColor()
	{
		return FLinearColor::FromSRGBColor(FColor::FromHex(BkgColorHex));
	}

	/** Marge fixe à gauche des fonds, côté photo. */
	inline constexpr float BkgLeftMargin = 250.f;
	/** Marge de respiration à droite du texte le plus large. */
	inline constexpr float BkgRightMargin = 50.f;

	/**
	 * @brief Bornes communes au fond du nom et au fond du club.
	 * @note Le plus large des deux textes impose la largeur, pour que les deux barres
	 *       restent alignées quel que soit celui qui déborde.
	 */
	inline FStartAndEnd MakeSharedBkgBounds(float NameTextWidth, float ClubTextWidth)
	{
		const float Width = FMath::Max(NameTextWidth, ClubTextWidth);
		return FStartAndEnd(
			FVector(0.f, BkgLeftMargin, 0.f),
			FVector(0.f, -Width - BkgRightMargin, 0.f));
	}
}
