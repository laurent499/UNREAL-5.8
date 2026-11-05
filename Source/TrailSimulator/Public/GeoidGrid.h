// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

THIRD_PARTY_INCLUDES_START
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/EarthGravitationalModel1996Grid.h>
THIRD_PARTY_INCLUDES_END

// -------------------- Geoid EGM96
/**
 * Ecart entre le geoide EGM96 (niveau de la mer) et l'ellipsoide WGS84, lu dans la grille officielle
 * Content/Data/Geoid/WW15MGH.DAC (chargee une fois, ~2 Mo en memoire).
 */
struct FGeoidGrid
{
	static const CesiumGeospatial::EarthGravitationalModel1996Grid* Get()
	{
		static TOptional<std::optional<CesiumGeospatial::EarthGravitationalModel1996Grid>> Grid;
		if (!Grid.IsSet())
		{
			TArray<uint8> Bytes;
			const FString File = FPaths::ProjectContentDir() / TEXT("Data/Geoid/WW15MGH.DAC");
			if (FFileHelper::LoadFileToArray(Bytes, *File))
			{
				Grid.Emplace(CesiumGeospatial::EarthGravitationalModel1996Grid::fromBuffer(
					std::span<const std::byte>(reinterpret_cast<const std::byte*>(Bytes.GetData()), Bytes.Num())));
			}
			else
			{
				Grid.Emplace(std::nullopt);
			}
			if (!Grid.GetValue())
			{
				UE_LOG(LogTemp, Error, TEXT("[Path] Grille du geoide illisible (%s) : trace place sur l'altitude GPS brute"), *File);
			}
		}
		return Grid.GetValue() ? &*Grid.GetValue() : nullptr;
	}
	static bool IsAvailable() { return Get() != nullptr; }
	static double UndulationM(double LonDeg, double LatDeg)
	{
		const CesiumGeospatial::EarthGravitationalModel1996Grid* G = Get();
		return G ? G->sampleHeight(CesiumGeospatial::Cartographic::fromDegrees(LonDeg, LatDeg, 0.0)) : 0.0;
	}
};
