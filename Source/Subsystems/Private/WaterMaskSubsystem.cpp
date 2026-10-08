// All Rights Reserved


#include "WaterMaskSubsystem.h"
#include "PathSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Algo/Reverse.h"
#include "Async/Async.h"
#include "Tasks/Task.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Containers/Ticker.h"
#include "EngineUtils.h"
#include "Cesium3DTileset.h"
#include "CesiumRasterOverlay.h"
#include "CesiumGeoJsonDocumentRasterOverlay.h"

DEFINE_LOG_CATEGORY_STATIC(LogWaterMask, Log, All);

const FString UWaterMaskSubsystem::MaterialLayerKey = TEXT("WaterOSM");

namespace WaterMask
{
	/** Marge autour du trace, en km */
	constexpr double MarginKm = 5.0;
	/** Au-dela de cette emprise (degres), on ne requete pas OSM : le masque JRC suffit */
	constexpr double MaxSpanDegrees = 2.0;
	/** Duree de validite du cache, en jours */
	constexpr double CacheMaxAgeDays = 30.0;
	/** Cle de couche de l'overlay JRC mondial, sert a reperer le tileset a equiper */
	const TCHAR* JrcLayerKey = TEXT("Water");
	/** Serveurs Overpass, essayes dans l'ordre si le precedent echoue */
	const TCHAR* OverpassUrls[] = {
		TEXT("https://overpass-api.de/api/interpreter"),
		TEXT("https://overpass.private.coffee/api/interpreter"),
		TEXT("https://overpass.kumi.systems/api/interpreter"),
	};
	constexpr int32 MaxRounds = 3;
	constexpr float RetryDelaySeconds = 20.0f;

	using FRing = TArray<FVector2D>; // X = lon, Y = lat

	FRing ReadGeometry(const TSharedPtr<FJsonObject>& Object)
	{
		FRing Ring;
		const TArray<TSharedPtr<FJsonValue>>* Points = nullptr;
		if (!Object->TryGetArrayField(TEXT("geometry"), Points))
		{
			return Ring;
		}
		Ring.Reserve(Points->Num());
		for (const TSharedPtr<FJsonValue>& Value : *Points)
		{
			const TSharedPtr<FJsonObject>* Point = nullptr;
			if (Value->TryGetObject(Point))
			{
				Ring.Emplace((*Point)->GetNumberField(TEXT("lon")), (*Point)->GetNumberField(TEXT("lat")));
			}
		}
		return Ring;
	}

	bool IsClosed(const FRing& Ring)
	{
		return Ring.Num() >= 4 && Ring[0].Equals(Ring.Last(), 1e-9);
	}

	/** Recoud les ways d'une relation multipolygone en anneaux fermes */
	TArray<FRing> StitchRings(TArray<FRing> Parts)
	{
		TArray<FRing> Rings;
		while (Parts.Num() > 0)
		{
			FRing Current = Parts.Pop(EAllowShrinking::No);
			bool bExtended = true;
			while (!IsClosed(Current) && bExtended)
			{
				bExtended = false;
				for (int32 i = 0; i < Parts.Num(); ++i)
				{
					FRing& Part = Parts[i];
					if (Part.Num() == 0)
					{
						continue;
					}
					if (Current.Last().Equals(Part[0], 1e-9))
					{
						Current.Append(Part.GetData() + 1, Part.Num() - 1);
					}
					else if (Current.Last().Equals(Part.Last(), 1e-9))
					{
						Algo::Reverse(Part);
						Current.Append(Part.GetData() + 1, Part.Num() - 1);
					}
					else
					{
						continue;
					}
					Parts.RemoveAtSwap(i);
					bExtended = true;
					break;
				}
			}
			if (IsClosed(Current))
			{
				Rings.Add(MoveTemp(Current));
			}
		}
		return Rings;
	}

	bool PointInRing(const FVector2D& P, const FRing& Ring)
	{
		bool bInside = false;
		for (int32 i = 0, j = Ring.Num() - 1; i < Ring.Num(); j = i++)
		{
			const FVector2D& A = Ring[i];
			const FVector2D& B = Ring[j];
			if ((A.Y > P.Y) != (B.Y > P.Y) && P.X < (B.X - A.X) * (P.Y - A.Y) / (B.Y - A.Y) + A.X)
			{
				bInside = !bInside;
			}
		}
		return bInside;
	}

	FString RingToJson(const FRing& Ring)
	{
		TArray<FString> Coords;
		Coords.Reserve(Ring.Num());
		for (const FVector2D& P : Ring)
		{
			Coords.Add(FString::Printf(TEXT("[%.7f,%.7f]"), P.X, P.Y));
		}
		return TEXT("[") + FString::Join(Coords, TEXT(",")) + TEXT("]");
	}

	/** Largeur (m) d'un cours d'eau lineaire, d'apres le tag width ou le type */
	double LineWidthMeters(const TSharedPtr<FJsonObject>& Tags)
	{
		FString Width;
		if (Tags->TryGetStringField(TEXT("width"), Width))
		{
			const double Parsed = FCString::Atod(*Width);
			if (Parsed > 0.5)
			{
				return FMath::Clamp(Parsed, 1.0, 200.0);
			}
		}
		const FString Waterway = Tags->GetStringField(TEXT("waterway"));
		if (Waterway == TEXT("river"))
		{
			return 15.0;
		}
		if (Waterway == TEXT("canal"))
		{
			return 8.0;
		}
		return 3.0; // stream
	}

	/**
	 * @brief Transforme un cours d'eau lineaire en polygones de la largeur voulue :
	 * un quadrilatere par segment, qui se chevauchent aux jonctions.
	 * Retourne la liste des polygones au format MultiPolygon (sans crochets externes).
	 */
	FString BufferLine(const FRing& Line, double WidthMeters)
	{
		constexpr double MetersPerDegree = 111320.0;
		const double HalfWidth = 0.5 * WidthMeters;
		TArray<FString> Quads;
		Quads.Reserve(Line.Num() - 1);
		for (int32 i = 0; i + 1 < Line.Num(); ++i)
		{
			const FVector2D& A = Line[i];
			const FVector2D& B = Line[i + 1];
			const double LonScale = MetersPerDegree * FMath::Max(FMath::Cos(FMath::DegreesToRadians(0.5 * (A.Y + B.Y))), 0.01);
			// Segment en metres locaux, puis normale
			const FVector2D Dir((B.X - A.X) * LonScale, (B.Y - A.Y) * MetersPerDegree);
			const double Length = Dir.Size();
			if (Length < 0.01)
			{
				continue;
			}
			// Prolonge legerement chaque segment pour boucher les jonctions
			const FVector2D Along = Dir / Length * HalfWidth;
			const FVector2D Normal(-Dir.Y / Length * HalfWidth, Dir.X / Length * HalfWidth);
			const FVector2D AlongDeg(Along.X / LonScale, Along.Y / MetersPerDegree);
			const FVector2D NormalDeg(Normal.X / LonScale, Normal.Y / MetersPerDegree);
			const FVector2D A0 = A - AlongDeg;
			const FVector2D B0 = B + AlongDeg;
			const FRing Quad = { A0 + NormalDeg, B0 + NormalDeg, B0 - NormalDeg, A0 - NormalDeg, A0 + NormalDeg };
			Quads.Add(TEXT("[") + RingToJson(Quad) + TEXT("]"));
		}
		return FString::Join(Quads, TEXT(","));
	}

	/**
	 * @brief Convertit la reponse Overpass en features GeoJSON (sans les crochets du tableau)
	 */
	FString OverpassToFeatures(const FString& OverpassJson)
	{
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OverpassJson);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return FString();
		}

		TArray<FString> Features;
		const TArray<TSharedPtr<FJsonValue>>* Elements = nullptr;
		if (!Root->TryGetArrayField(TEXT("elements"), Elements))
		{
			return FString();
		}

		for (const TSharedPtr<FJsonValue>& Value : *Elements)
		{
			const TSharedPtr<FJsonObject> Element = Value->AsObject();
			if (!Element.IsValid())
			{
				continue;
			}
			const TSharedPtr<FJsonObject>* TagsPtr = nullptr;
			if (!Element->TryGetObjectField(TEXT("tags"), TagsPtr))
			{
				continue;
			}
			const TSharedPtr<FJsonObject>& Tags = *TagsPtr;
			const FString Type = Element->GetStringField(TEXT("type"));
			const bool bLinear = Tags->HasField(TEXT("waterway")) && Tags->GetStringField(TEXT("waterway")) != TEXT("riverbank");

			if (Type == TEXT("way"))
			{
				const FRing Ring = ReadGeometry(Element);
				if (bLinear && Ring.Num() >= 2)
				{
					const FString Polygons = BufferLine(Ring, LineWidthMeters(Tags));
					if (!Polygons.IsEmpty())
					{
						Features.Add(FString::Printf(
							TEXT("{\"type\":\"Feature\",\"properties\":{},\"geometry\":{\"type\":\"MultiPolygon\",\"coordinates\":[%s]}}"),
							*Polygons));
					}
				}
				else if (!bLinear && IsClosed(Ring))
				{
					Features.Add(FString::Printf(
						TEXT("{\"type\":\"Feature\",\"properties\":{},\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[%s]}}"),
						*RingToJson(Ring)));
				}
			}
			else if (Type == TEXT("relation") && !bLinear)
			{
				TArray<FRing> OuterParts;
				TArray<FRing> InnerParts;
				const TArray<TSharedPtr<FJsonValue>>* Members = nullptr;
				if (!Element->TryGetArrayField(TEXT("members"), Members))
				{
					continue;
				}
				for (const TSharedPtr<FJsonValue>& MemberValue : *Members)
				{
					const TSharedPtr<FJsonObject> Member = MemberValue->AsObject();
					if (!Member.IsValid() || Member->GetStringField(TEXT("type")) != TEXT("way"))
					{
						continue;
					}
					FRing Part = ReadGeometry(Member);
					if (Part.Num() < 2)
					{
						continue;
					}
					(Member->GetStringField(TEXT("role")) == TEXT("inner") ? InnerParts : OuterParts).Add(MoveTemp(Part));
				}

				const TArray<FRing> Outers = StitchRings(MoveTemp(OuterParts));
				const TArray<FRing> Inners = StitchRings(MoveTemp(InnerParts));
				TArray<FString> Polygons;
				for (const FRing& Outer : Outers)
				{
					FString Polygon = RingToJson(Outer);
					for (const FRing& Inner : Inners)
					{
						if (PointInRing(Inner[0], Outer))
						{
							Polygon += TEXT(",") + RingToJson(Inner);
						}
					}
					Polygons.Add(TEXT("[") + Polygon + TEXT("]"));
				}
				if (Polygons.Num() > 0)
				{
					Features.Add(FString::Printf(
						TEXT("{\"type\":\"Feature\",\"properties\":{},\"geometry\":{\"type\":\"MultiPolygon\",\"coordinates\":[%s]}}"),
						*FString::Join(Polygons, TEXT(","))));
				}
			}
		}
		return FString::Join(Features, TEXT(",\n"));
	}
}

void UWaterMaskSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PathSubsystem = Collection.InitializeDependency<UPathSubsystem>();
	if (PathSubsystem)
	{
		PathSubsystem->OnPathDatasGathered.AddDynamic(this, &UWaterMaskSubsystem::HandlePathDatasGathered);
	}
}

void UWaterMaskSubsystem::Deinitialize()
{
	for (auto& Pair : ActiveRequests)
	{
		Pair.Value->OnProcessRequestComplete().Unbind();
		Pair.Value->CancelRequest();
	}
	ActiveRequests.Empty();
	RaceFeatures.Empty();
	bWaterPolygonsDirty = true;
	if (PathSubsystem)
	{
		PathSubsystem->OnPathDatasGathered.RemoveDynamic(this, &UWaterMaskSubsystem::HandlePathDatasGathered);
	}
	Super::Deinitialize();
}

/**
 * @brief Emprise du trace + marge. Retourne false si le trace est vide ou trop etendu.
 */
bool UWaterMaskSubsystem::ComputeBounds(const FRacePath& RacePath, double MarginKm, FWaterMaskBounds& OutBounds)
{
	if (RacePath.Points.Num() == 0)
	{
		return false;
	}
	OutBounds.South = OutBounds.North = RacePath.Points[0].lat;
	OutBounds.West = OutBounds.East = RacePath.Points[0].lon;
	for (const FRacePathPoint& Point : RacePath.Points)
	{
		OutBounds.South = FMath::Min<double>(OutBounds.South, Point.lat);
		OutBounds.North = FMath::Max<double>(OutBounds.North, Point.lat);
		OutBounds.West = FMath::Min<double>(OutBounds.West, Point.lon);
		OutBounds.East = FMath::Max<double>(OutBounds.East, Point.lon);
	}

	const double MidLat = FMath::DegreesToRadians(0.5 * (OutBounds.South + OutBounds.North));
	const double MarginLat = MarginKm / 111.0;
	const double MarginLon = MarginKm / (111.0 * FMath::Max(FMath::Cos(MidLat), 0.1));
	// Arrondi au centieme pour que le cache soit reutilise d'un chargement a l'autre
	OutBounds.South = FMath::FloorToDouble((OutBounds.South - MarginLat) * 100.0) / 100.0;
	OutBounds.North = FMath::CeilToDouble((OutBounds.North + MarginLat) * 100.0) / 100.0;
	OutBounds.West = FMath::FloorToDouble((OutBounds.West - MarginLon) * 100.0) / 100.0;
	OutBounds.East = FMath::CeilToDouble((OutBounds.East + MarginLon) * 100.0) / 100.0;

	return (OutBounds.North - OutBounds.South) <= WaterMask::MaxSpanDegrees
		&& (OutBounds.East - OutBounds.West) <= WaterMask::MaxSpanDegrees;
}

FString UWaterMaskSubsystem::GetCacheFilePath(const FWaterMaskBounds& Bounds)
{
	return FPaths::ProjectSavedDir() / TEXT("WaterCache") / FString::Printf(
		TEXT("osm_water_%.2f_%.2f_%.2f_%.2f.geojson"), Bounds.South, Bounds.West, Bounds.North, Bounds.East);
}

/**
 * @brief A la reception d'un trace : charge l'eau OSM de sa zone (cache ou Overpass)
 */
void UWaterMaskSubsystem::HandlePathDatasGathered(int64 RaceID, FRacePath RacePath)
{
	FWaterMaskBounds Bounds;
	if (!ComputeBounds(RacePath, WaterMask::MarginKm, Bounds))
	{
		UE_LOG(LogWaterMask, Warning, TEXT("[WaterMask] Race %lld : trace vide ou trop etendu, masque JRC seul"), RaceID);
		return;
	}

	const FString CacheFile = GetCacheFilePath(Bounds);
	IFileManager& FileManager = IFileManager::Get();
	const FDateTime CacheTime = FileManager.GetTimeStamp(*CacheFile);
	const bool bCacheFresh = CacheTime != FDateTime::MinValue()
		&& (FDateTime::UtcNow() - CacheTime).GetTotalDays() < WaterMask::CacheMaxAgeDays;

	FString Cached;
	if (bCacheFresh && FFileHelper::LoadFileToString(Cached, *CacheFile))
	{
		UE_LOG(LogWaterMask, Log, TEXT("[WaterMask] Race %lld : eau OSM lue depuis le cache %s"), RaceID, *CacheFile);
		OnRaceWaterReady(RaceID, MoveTemp(Cached));
		return;
	}
	RequestOverpass(RaceID, Bounds, CacheFile);
}

void UWaterMaskSubsystem::RequestOverpass(int64 RaceID, const FWaterMaskBounds& Bounds, const FString& CacheFile, int32 ServerIndex)
{
	// Les serveurs Overpass sont souvent satures (504) : on fait plusieurs tours, espaces de quelques secondes
	constexpr int32 NumServers = UE_ARRAY_COUNT(WaterMask::OverpassUrls);
	if (ServerIndex >= NumServers * WaterMask::MaxRounds)
	{
		UE_LOG(LogWaterMask, Warning, TEXT("[WaterMask] Race %lld : aucun serveur Overpass n'a repondu, masque JRC seul"), RaceID);
		return;
	}
	if (ServerIndex > 0 && ServerIndex % NumServers == 0)
	{
		TWeakObjectPtr<UWaterMaskSubsystem> WeakRetry(this);
		const FWaterMaskBounds RetryBounds = Bounds;
		const FString RetryCacheFile = CacheFile;
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
			[WeakRetry, RaceID, RetryBounds, RetryCacheFile, ServerIndex](float)
			{
				if (WeakRetry.IsValid())
				{
					WeakRetry->SendOverpassRequest(RaceID, RetryBounds, RetryCacheFile, ServerIndex);
				}
				return false;
			}), WaterMask::RetryDelaySeconds);
		return;
	}
	SendOverpassRequest(RaceID, Bounds, CacheFile, ServerIndex);
}

void UWaterMaskSubsystem::SendOverpassRequest(int64 RaceID, const FWaterMaskBounds& Bounds, const FString& CacheFile, int32 ServerIndex)
{
	constexpr int32 NumServers = UE_ARRAY_COUNT(WaterMask::OverpassUrls);

	const FString BBox = FString::Printf(TEXT("(%.4f,%.4f,%.4f,%.4f)"), Bounds.South, Bounds.West, Bounds.North, Bounds.East);
	const FString Query = FString::Printf(TEXT(
		"[out:json][timeout:60];("
		"way[\"natural\"=\"water\"]%s;relation[\"natural\"=\"water\"]%s;"
		"way[\"waterway\"=\"riverbank\"]%s;relation[\"waterway\"=\"riverbank\"]%s;"
		"way[\"landuse\"=\"reservoir\"]%s;relation[\"landuse\"=\"reservoir\"]%s;"
		"way[\"waterway\"~\"^(river|canal)$\"]%s;"
		"way[\"waterway\"=\"stream\"][\"intermittent\"!=\"yes\"]%s;"
		");out geom;"),
		*BBox, *BBox, *BBox, *BBox, *BBox, *BBox, *BBox, *BBox);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(WaterMask::OverpassUrls[ServerIndex % NumServers]);
	Req->SetVerb(TEXT("POST"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
	Req->SetHeader(TEXT("User-Agent"), TEXT("TrailSimulator/1.0 (water mask)"));
	Req->SetContentAsString(TEXT("data=") + FGenericPlatformHttp::UrlEncode(Query));
	Req->SetTimeout(120.0f);

	if (TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>* Previous = ActiveRequests.Find(RaceID))
	{
		(*Previous)->OnProcessRequestComplete().Unbind();
		(*Previous)->CancelRequest();
	}
	ActiveRequests.Add(RaceID, Req);

	TWeakObjectPtr<UWaterMaskSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
	[WeakThis, RaceID, Bounds, CacheFile, ServerIndex](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (!WeakThis.IsValid()) return;
		WeakThis->ActiveRequests.Remove(RaceID);

		if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
			UE_LOG(LogWaterMask, Warning, TEXT("[WaterMask] Race %lld : Overpass %s en echec (Code=%d)"),
				RaceID, WaterMask::OverpassUrls[ServerIndex % NumServers], Response.IsValid() ? Response->GetResponseCode() : -1);
			WeakThis->RequestOverpass(RaceID, Bounds, CacheFile, ServerIndex + 1);
			return;
		}

		FString OverpassJson = Response->GetContentAsString();
		UE::Tasks::Launch(UE_SOURCE_LOCATION,
			[WeakThis, RaceID, CacheFile, OverpassJson = MoveTemp(OverpassJson)]()
			{
				FString Features = WaterMask::OverpassToFeatures(OverpassJson);
				if (!Features.IsEmpty())
				{
					FFileHelper::SaveStringToFile(Features, *CacheFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
				}
				AsyncTask(ENamedThreads::GameThread,
					[WeakThis, RaceID, Features = MoveTemp(Features)]() mutable
					{
						if (!WeakThis.IsValid()) return;
						UE_LOG(LogWaterMask, Log, TEXT("[WaterMask] Race %lld : eau OSM recue d'Overpass (%d caracteres)"), RaceID, Features.Len());
						WeakThis->OnRaceWaterReady(RaceID, MoveTemp(Features));
					});
			},
			UE::Tasks::ETaskPriority::BackgroundNormal);
	});

	if (!Req->ProcessRequest())
	{
		ActiveRequests.Remove(RaceID);
		UE_LOG(LogWaterMask, Error, TEXT("[WaterMask] ProcessRequest() a echoue pour la course %lld"), RaceID);
		RequestOverpass(RaceID, Bounds, CacheFile, ServerIndex + 1);
	}
}

void UWaterMaskSubsystem::OnRaceWaterReady(int64 RaceID, FString&& FeaturesJson)
{
	check(IsInGameThread());
	if (FeaturesJson.IsEmpty())
	{
		RaceFeatures.Remove(RaceID);
	}
	else
	{
		RaceFeatures.Add(RaceID, MoveTemp(FeaturesJson));
	}
	bWaterPolygonsDirty = true;
	ApplyToTilesets();
}

void UWaterMaskSubsystem::RebuildWaterPolygons()
{
	bWaterPolygonsDirty = false;
	WaterPolygons.Reset();

	auto ReadRing = [](const TArray<TSharedPtr<FJsonValue>>& Points, TArray<FVector2D>& Out)
	{
		for (const TSharedPtr<FJsonValue>& P : Points)
		{
			const TArray<TSharedPtr<FJsonValue>>& XY = P->AsArray();
			if (XY.Num() >= 2) Out.Emplace(XY[0]->AsNumber(), XY[1]->AsNumber());
		}
	};
	auto AddPolygon = [this, &ReadRing](const TArray<TSharedPtr<FJsonValue>>& Rings)
	{
		FWaterPolygon& Poly = WaterPolygons.AddDefaulted_GetRef();
		for (const TSharedPtr<FJsonValue>& Ring : Rings)
		{
			ReadRing(Ring->AsArray(), Poly.Rings.AddDefaulted_GetRef());
			for (const FVector2D& P : Poly.Rings.Last()) Poly.Bounds += P;
		}
	};

	for (const TPair<int64, FString>& Pair : RaceFeatures)
	{
		TArray<TSharedPtr<FJsonValue>> Features;
		if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(TEXT("[") + Pair.Value + TEXT("]")), Features)) continue;
		for (const TSharedPtr<FJsonValue>& Feature : Features)
		{
			const TSharedPtr<FJsonObject>* Obj = nullptr;
			const TSharedPtr<FJsonObject>* Geometry = nullptr;
			if (!Feature->TryGetObject(Obj) || !(*Obj)->TryGetObjectField(TEXT("geometry"), Geometry)) continue;
			const FString Type = (*Geometry)->GetStringField(TEXT("type"));
			const TArray<TSharedPtr<FJsonValue>>* Coordinates = nullptr;
			if (!(*Geometry)->TryGetArrayField(TEXT("coordinates"), Coordinates)) continue;
			if (Type == TEXT("Polygon")) AddPolygon(*Coordinates);
			else if (Type == TEXT("MultiPolygon")) for (const TSharedPtr<FJsonValue>& P : *Coordinates) AddPolygon(P->AsArray());
		}
	}
}

bool UWaterMaskSubsystem::IsWaterAt(double Longitude, double Latitude)
{
	if (bWaterPolygonsDirty) RebuildWaterPolygons();
	const FVector2D Point(Longitude, Latitude);
	for (const FWaterPolygon& Poly : WaterPolygons)
	{
		if (!Poly.Bounds.IsInside(Point)) continue;
		// Regle pair-impair sur tous les anneaux : les iles (trous) sont exclues
		bool bInside = false;
		for (const TArray<FVector2D>& Ring : Poly.Rings)
		{
			for (int32 i = 0, j = Ring.Num() - 1; i < Ring.Num(); j = i++)
			{
				if ((Ring[i].Y > Point.Y) != (Ring[j].Y > Point.Y)
					&& Point.X < (Ring[j].X - Ring[i].X) * (Point.Y - Ring[i].Y) / (Ring[j].Y - Ring[i].Y) + Ring[i].X)
				{
					bInside = !bInside;
				}
			}
		}
		if (bInside) return true;
	}
	return false;
}

/**
 * @brief Regroupe l'eau de toutes les courses chargees dans un fichier GeoJSON
 * et le donne a l'overlay "WaterOSM" des tilesets qui portent deja l'overlay JRC
 */
void UWaterMaskSubsystem::ApplyToTilesets()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	TArray<FString> AllFeatures;
	for (const TPair<int64, FString>& Pair : RaceFeatures)
	{
		AllFeatures.Add(Pair.Value);
	}
	const FString GeoJson = TEXT("{\"type\":\"FeatureCollection\",\"features\":[")
		+ FString::Join(AllFeatures, TEXT(",\n")) + TEXT("]}");

	// L'overlay lit le document par URL : on passe par un fichier local
	const FString DocumentFile = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("WaterCache") / TEXT("current_osm_water.geojson"));
	if (!FFileHelper::SaveStringToFile(GeoJson, *DocumentFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogWaterMask, Error, TEXT("[WaterMask] Ecriture impossible : %s"), *DocumentFile);
		return;
	}
	const FString DocumentUrl = TEXT("file:///") + DocumentFile.Replace(TEXT("\\"), TEXT("/"));

	FCesiumVectorStyle DefaultStyle;
	DefaultStyle.PolygonStyle.Fill = true;
	DefaultStyle.PolygonStyle.FillStyle.Color = FColor::White;
	DefaultStyle.PolygonStyle.Outline = false;

	for (TActorIterator<ACesium3DTileset> It(World); It; ++It)
	{
		ACesium3DTileset* Tileset = *It;
		TArray<UCesiumRasterOverlay*> Overlays;
		Tileset->GetComponents(Overlays);

		const bool bHasJrc = Overlays.ContainsByPredicate([](const UCesiumRasterOverlay* Overlay)
		{
			return Overlay->MaterialLayerKey == WaterMask::JrcLayerKey;
		});
		if (!bHasJrc)
		{
			continue;
		}

		UCesiumRasterOverlay** Existing = Overlays.FindByPredicate([](const UCesiumRasterOverlay* Overlay)
		{
			return Overlay->MaterialLayerKey == MaterialLayerKey;
		});
		UCesiumGeoJsonDocumentRasterOverlay* OsmOverlay = Existing ? Cast<UCesiumGeoJsonDocumentRasterOverlay>(*Existing) : nullptr;

		if (!OsmOverlay)
		{
			OsmOverlay = NewObject<UCesiumGeoJsonDocumentRasterOverlay>(Tileset, TEXT("OsmWaterOverlay"));
			OsmOverlay->MaterialLayerKey = MaterialLayerKey;
			OsmOverlay->Source = ECesiumGeoJsonDocumentRasterOverlaySource::FromUrl;
			OsmOverlay->Url = DocumentUrl;
			OsmOverlay->MipLevels = 2;
			OsmOverlay->DefaultStyle = DefaultStyle;
			Tileset->AddInstanceComponent(OsmOverlay);
			OsmOverlay->RegisterComponent(); // auto-activation : ajoute l'overlay au tileset
		}
		else
		{
			OsmOverlay->Url = DocumentUrl;
			OsmOverlay->Refresh();
		}
		UE_LOG(LogWaterMask, Log, TEXT("[WaterMask] Overlay OSM applique a %s (%d course(s))"),
			*Tileset->GetActorNameOrLabel(), RaceFeatures.Num());
	}
}
