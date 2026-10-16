// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TrailTemplateTypes.generated.h"

/**
 * @brief Template graphique d'une course, lu sur la régie (GET template/<nom>).
 * @note Les couleurs arrivent en hexa sRGB (#RRGGBBAA), déjà résolues par la régie,
 *       sauf "race" qui désigne la couleur de la course et se résout ici.
 *       Les noms des champs suivent le JSON (la correspondance ignore la casse).
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FTrailTemplateGlobals
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString Primary = TEXT("#000000FF");
	UPROPERTY(BlueprintReadOnly) FString Secondary = TEXT("race");
	UPROPERTY(BlueprintReadOnly) FString Accent = TEXT("race");
	UPROPERTY(BlueprintReadOnly) FString TextLight = TEXT("#FFFFFFFF");
	UPROPERTY(BlueprintReadOnly) FString TextDark = TEXT("#000000FF");
	/** 0 = angles vifs, 1 = arrondi maximal. */
	UPROPERTY(BlueprintReadOnly) float CornerRadius = 0.f;
	/** Biseau des fonds, en degrés. */
	UPROPERTY(BlueprintReadOnly) float Slant = 0.f;
	/** Dossier des pictos : /Game/LTVContent/2D/Pictos/<PictoSet>. */
	UPROPERTY(BlueprintReadOnly) FString PictoSet = TEXT("Generic");
};

/** Couleurs d'un empilement checkpoint ou POI, de haut en bas. */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FTrailTemplateStack
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString Foot = TEXT("race");
	UPROPERTY(BlueprintReadOnly) FString MainPicto = TEXT("#FFFFFFFF");
	UPROPERTY(BlueprintReadOnly) FString NameBkg = TEXT("#000000FF");
	UPROPERTY(BlueprintReadOnly) FString NameBorder = TEXT("race");
	UPROPERTY(BlueprintReadOnly) FString NameText = TEXT("#FFFFFFFF");
	UPROPERTY(BlueprintReadOnly) FString InfosBkg = TEXT("race");
	UPROPERTY(BlueprintReadOnly) FString InfosText = TEXT("#000000FF");
	UPROPERTY(BlueprintReadOnly) FString InfosPicto = TEXT("#000000FF");
	UPROPERTY(BlueprintReadOnly) FString Line1 = TEXT("#000000FF");
	UPROPERTY(BlueprintReadOnly) FString WeatherBkg = TEXT("race");
	UPROPERTY(BlueprintReadOnly) FString WeatherText = TEXT("#000000FF");
	UPROPERTY(BlueprintReadOnly) FString Line2 = TEXT("race");
};

/** Cartouche des coureurs. */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FTrailTemplateRunner
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString Foot = TEXT("race");
	UPROPERTY(BlueprintReadOnly) FString NameBkg = TEXT("#FFFFFFFF");
	UPROPERTY(BlueprintReadOnly) FString NameText = TEXT("#FFFFFFFF");
	UPROPERTY(BlueprintReadOnly) FString ClubBkg = TEXT("#000000FF");
	UPROPERTY(BlueprintReadOnly) FString ClubText = TEXT("#FFFFFFFF");
	UPROPERTY(BlueprintReadOnly) FString Slash = TEXT("#98BF16FF");
	/** Éléments présents dans le design ; l'opérateur peut toujours les masquer. */
	UPROPERTY(BlueprintReadOnly) bool ShowPhoto = true;
	UPROPERTY(BlueprintReadOnly) bool ShowClub = true;
	UPROPERTY(BlueprintReadOnly) bool ShowFlag = true;
	UPROPERTY(BlueprintReadOnly) float BkgLeftMargin = 250.f;
	UPROPERTY(BlueprintReadOnly) float BkgRightMargin = 50.f;
	/** Hauteur du cartouche au-dessus du coureur (VDelta). */
	UPROPERTY(BlueprintReadOnly) float Height = 100.f;
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FTrailTemplate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString Name;
	/** Mise en page prédéfinie : famille de classes Generic, GTWS, UTMB ou Nike. */
	UPROPERTY(BlueprintReadOnly) FString Base = TEXT("Generic");
	UPROPERTY(BlueprintReadOnly) FTrailTemplateGlobals Globals;
	UPROPERTY(BlueprintReadOnly) FTrailTemplateStack Checkpoint;
	UPROPERTY(BlueprintReadOnly) FTrailTemplateStack Poi;
	UPROPERTY(BlueprintReadOnly) FTrailTemplateRunner Runner;

	/**
	 * @brief Convertit une couleur du template en linéaire.
	 * @param Value Hexa sRGB, ou "race" pour la couleur de la course.
	 * @param RaceColor Couleur de la course, en hexa sRGB.
	 */
	static FLinearColor ToLinear(const FString& Value, const FString& RaceColor)
	{
		const FString& Hex = Value.Equals(TEXT("race"), ESearchCase::IgnoreCase) ? RaceColor : Value;
		return FLinearColor::FromSRGBColor(FColor::FromHex(Hex));
	}

	/** Même conversion, pour les TextRender qui prennent une couleur sRGB. */
	static FColor ToSRGB(const FString& Value, const FString& RaceColor)
	{
		const FString& Hex = Value.Equals(TEXT("race"), ESearchCase::IgnoreCase) ? RaceColor : Value;
		return FColor::FromHex(Hex);
	}
};
