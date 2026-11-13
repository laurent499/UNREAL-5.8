// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineMeshComponent.h"

/**
 * @brief Finitions des panneaux (checkpoints, POI) quand le template arrondit les fonds.
 * @note Calculé d'après les bornes des fonds, donc rejouable sans dérive (rechargement du
 *       template). Ne fait rien quand l'arrondi vaut 0 : la mise en page d'origine reste intacte.
 */
namespace RoundedPanel
{
	/**
	 * @brief Recentre un filet dans l'espace entre deux fonds et le raccourcit pour qu'il
	 *        ne dépasse pas des coins arrondis.
	 * @param Line Filet (spline mesh) à ajuster.
	 * @param Upper Fond au-dessus du filet.
	 * @param Lower Fond au-dessous du filet.
	 * @param CornerRadius Arrondi du template (part de la hauteur du fond).
	 * @param Slant Biseau du template, en degrés : le filet est raccourci pour rester sous la
	 *        partie commune des deux fonds penchés (M_PlainRounded les biseaute autour de leur centre).
	 */
	inline void FitLine(USplineMeshComponent* Line, UPrimitiveComponent* Upper, UPrimitiveComponent* Lower, float CornerRadius, float Slant = 0.f)
	{
		const bool bSlanted = FMath::Abs(Slant) >= 0.01f;
		if ((CornerRadius <= 0.f && !bSlanted) || !Line || !Upper || !Lower)
		{
			return;
		}
		Upper->UpdateBounds();
		Lower->UpdateBounds();
		Line->UpdateBounds();
		const FBox UpperBox = Upper->Bounds.GetBox();
		const FBox LowerBox = Lower->Bounds.GetBox();
		const FBox LineBox = Line->Bounds.GetBox();

		// Vertical : au milieu de l'espace entre les deux fonds (s'il y en a un)
		const float GapTop = UpperBox.Min.Z;
		const float GapBottom = LowerBox.Max.Z;
		if (GapTop > GapBottom)
		{
			const float DeltaZ = 0.5f * (GapTop + GapBottom) - LineBox.GetCenter().Z;
			Line->AddWorldOffset(FVector(0.f, 0.f, DeltaZ));
		}

		// Horizontal : largeur du fond moins un rayon de chaque côté, et moins le décalage du
		// biseau sur la hauteur du plus haut des deux fonds (chaque fond perd |tan| x sa hauteur)
		const FVector UpperSize = UpperBox.GetSize();
		const float BarWidth = FVector2D(UpperSize.X, UpperSize.Y).Size();
		const float BarHeight = UpperSize.Z;
		const float Radius = FMath::Clamp(CornerRadius * FMath::Min(BarWidth, BarHeight), 0.f, 0.5f * FMath::Min(BarWidth, BarHeight));
		const float SlantInset = bSlanted
			? FMath::Abs(FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(Slant, -60.f, 60.f)))) * FMath::Max(BarHeight, LowerBox.GetSize().Z)
			: 0.f;
		const float TargetWidth = FMath::Max(BarWidth - 2.f * Radius - 2.f * SlantInset, 0.f);

		const FVector Start = Line->GetStartPosition();
		const FVector End = Line->GetEndPosition();
		const float LocalLength = FVector::Dist(Start, End);
		const FVector LineSize = LineBox.GetSize();
		const float WorldLength = FVector2D(LineSize.X, LineSize.Y).Size();
		if (LocalLength <= KINDA_SMALL_NUMBER || WorldLength <= KINDA_SMALL_NUMBER)
		{
			return;
		}
		const float LocalPerWorld = LocalLength / WorldLength;
		const FVector Dir = (End - Start).GetSafeNormal();
		const FVector Mid = 0.5f * (Start + End);
		const FVector Half = Dir * (0.5f * TargetWidth * LocalPerWorld);
		const FVector Tangent = 2.f * Half;
		Line->SetStartAndEnd(Mid - Half, Tangent, Mid + Half, Tangent, false);
		Line->UpdateMesh();
	}
}
