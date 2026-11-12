// Copyright LTVProd 2026 All Rights Reserved.

#include "CartoImagerySubsystem.h"
#include "StyleSubsystem.h"
#include "Cesium3DTileset.h"
#include "CesiumGeoreference.h"
#include "CesiumIonServer.h"
#include "CesiumUrlTemplateRasterOverlay.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogCartoImagery, Log, All);

namespace CartoImagery
{
	static const FName SectionName(TEXT("imagery"));
	/** Tag du relief créé par le style : Path et les autres outils ne doivent pas le prendre pour le sol de référence. */
	static const FName TerrainTag(TEXT("LTV_StyleTerrain"));
	/** Couche lue par le matériau par défaut de Cesium (MI_CesiumThreeOverlaysAndClipping). */
	static const TCHAR* OverlayKey = TEXT("Overlay0");
	/** Cesium World Terrain. */
	static constexpr int64 WorldTerrainAssetId = 1;

	/** Clé Mapbox locale (jamais loguée) : variable LTV_MAPBOX_TOKEN, sinon %LOCALAPPDATA%/LTV/mapbox_token.txt. */
	static FString ReadMapboxToken()
	{
		FString Token = FPlatformMisc::GetEnvironmentVariable(TEXT("LTV_MAPBOX_TOKEN")).TrimStartAndEnd();
		// Puis Saved/Secrets du projet (ignoré par git), puis %LOCALAPPDATA%/LTV (commun à toutes les copies du projet)
		const FString Files[] = {
			FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Secrets"), TEXT("mapbox_token.txt")),
			FPaths::Combine(FPlatformProcess::UserSettingsDir(), TEXT("LTV"), TEXT("mapbox_token.txt")),
		};
		for (const FString& File : Files)
		{
			if (!Token.IsEmpty()) break;
			if (FFileHelper::LoadFileToString(Token, *File)) Token = Token.TrimStartAndEnd();
		}
		return Token;
	}

	static bool IsSafeStyleId(const FString& Style)
	{
		if (Style.IsEmpty() || !Style.Contains(TEXT("/"))) return false;
		for (const TCHAR C : Style)
		{
			if (!FChar::IsAlnum(C) && C != TEXT('_') && C != TEXT('-') && C != TEXT('.') && C != TEXT('/')) return false;
		}
		return true;
	}

	static FString GetString(const TSharedRef<FJsonObject>& Section, const TCHAR* Key, const FString& Default)
	{
		FString Value;
		return Section->TryGetStringField(Key, Value) && !Value.IsEmpty() ? Value : Default;
	}

	static double GetNumber(const TSharedRef<FJsonObject>& Section, const TCHAR* Key, double Default)
	{
		double Value = Default;
		return Section->TryGetNumberField(Key, Value) ? Value : Default;
	}

	static bool GetBool(const TSharedRef<FJsonObject>& Section, const TCHAR* Key, bool Default)
	{
		bool Value = Default;
		return Section->TryGetBoolField(Key, Value) ? Value : Default;
	}

	/** Réglages de l'overlay demandés par la section. */
	struct FMapSource
	{
		FString Url;          // contient la clé : ne jamais loguer
		FString Description;  // lisible, sans secret
		int32 TileSize = 256;
		int32 MaximumLevel = 19;
	};

	/** Construit l'URL XYZ de la section ; false (avec raison) si elle est inutilisable. */
	static bool BuildSource(const TSharedRef<FJsonObject>& Section, FMapSource& Out, FString& OutError)
	{
		const FString Source = GetString(Section, TEXT("source"), TEXT("mapbox")).ToLower();
		if (Source == TEXT("mapbox"))
		{
			const FString Style = GetString(Section, TEXT("style"), TEXT("mapbox/outdoors-v12"));
			if (!IsSafeStyleId(Style))
			{
				OutError = FString::Printf(TEXT("style Mapbox invalide '%s' (attendu : compte/style, ex. mapbox/outdoors-v12)"), *Style);
				return false;
			}
			const FString Token = ReadMapboxToken();
			if (Token.IsEmpty())
			{
				OutError = TEXT("clé Mapbox absente : variable LTV_MAPBOX_TOKEN ou %LOCALAPPDATA%/LTV/mapbox_token.txt");
				return false;
			}
			// Tuiles 512 px ; "retina" : @2x (1024 px), plus net mais 4 fois plus lourd
			const bool bRetina = GetBool(Section, TEXT("retina"), false);
			Out.TileSize = bRetina ? 1024 : 512;
			Out.MaximumLevel = FMath::Clamp(int32(GetNumber(Section, TEXT("maxLevel"), 22)), 1, 22);
			// {y} de Cesium part du sud : serveur XYZ => {reverseY}
			Out.Url = FString::Printf(TEXT("https://api.mapbox.com/styles/v1/%s/tiles/512/{z}/{x}/{reverseY}%s?access_token=%s"),
				*Style, bRetina ? TEXT("@2x") : TEXT(""), *Token);
			Out.Description = FString::Printf(TEXT("Mapbox %s"), *Style);
			return true;
		}
		if (Source == TEXT("osm"))
		{
			// Serveur communautaire OSM : usage léger seulement (politique tile.openstreetmap.org)
			Out.Url = TEXT("https://tile.openstreetmap.org/{z}/{x}/{reverseY}.png");
			Out.TileSize = 256;
			Out.MaximumLevel = FMath::Clamp(int32(GetNumber(Section, TEXT("maxLevel"), 19)), 1, 19);
			Out.Description = TEXT("OpenStreetMap");
			return true;
		}
		if (Source == TEXT("url"))
		{
			FString Url = GetString(Section, TEXT("url"), FString());
			if (!Url.StartsWith(TEXT("https://")) && !Url.StartsWith(TEXT("http://")))
			{
				OutError = TEXT("source url sans 'url' http(s)");
				return false;
			}
			Out.Description = Url;  // avant l'insertion d'une éventuelle clé
			// XYZ (origine nord) par défaut ; "yUp": true pour un serveur TMS
			if (!GetBool(Section, TEXT("yUp"), false)) Url.ReplaceInline(TEXT("{y}"), TEXT("{reverseY}"));
			if (Url.Contains(TEXT("{mapboxToken}")))
			{
				const FString Token = ReadMapboxToken();
				if (Token.IsEmpty())
				{
					OutError = TEXT("l'URL demande {mapboxToken} mais la clé Mapbox locale est absente");
					return false;
				}
				Url.ReplaceInline(TEXT("{mapboxToken}"), *Token);
			}
			Out.Url = Url;
			Out.TileSize = FMath::Clamp(int32(GetNumber(Section, TEXT("tileSize"), 256)), 64, 1024);
			Out.MaximumLevel = FMath::Clamp(int32(GetNumber(Section, TEXT("maxLevel"), 19)), 1, 25);
			return true;
		}
		OutError = FString::Printf(TEXT("source inconnue '%s' (mapbox, osm, url)"), *Source);
		return false;
	}
}

void UCartoImagerySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UStyleSubsystem* Styles = Collection.InitializeDependency<UStyleSubsystem>();
	if (!Styles) return;

	FStyleSectionHandler Handler;
	TWeakObjectPtr<UCartoImagerySubsystem> WeakThis(this);
	Handler.Apply = [WeakThis](UWorld* World, const TSharedRef<FJsonObject>& Section)
	{
		if (UCartoImagerySubsystem* Self = WeakThis.Get()) Self->ApplySection(World, Section);
	};
	Handler.Revert = [WeakThis](UWorld* World)
	{
		if (UCartoImagerySubsystem* Self = WeakThis.Get()) Self->RevertSection(World);
	};
	Styles->RegisterSectionHandler(CartoImagery::SectionName, MoveTemp(Handler));
}

void UCartoImagerySubsystem::Deinitialize()
{
	if (UStyleSubsystem* Styles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStyleSubsystem>() : nullptr)
	{
		Styles->UnregisterSectionHandler(CartoImagery::SectionName);
	}
	StopWaiting();
	Super::Deinitialize();
}

void UCartoImagerySubsystem::ApplySection(UWorld* World, const TSharedRef<FJsonObject>& Section)
{
	using namespace CartoImagery;
	if (!World) return;

	// Nouveau niveau : l'ancien monde a emporté relief et tilesets, on repart de zéro
	if (StyledWorld.Get() != World)
	{
		StopWaiting();
		Terrain.Reset();
		MapOverlay.Reset();
		HiddenPhoto.Reset();
		StyledWorld = World;
	}

	FMapSource Map;
	FString Error;
	if (!BuildSource(Section, Map, Error))
	{
		// Style inutilisable : on garde (ou on rend) le rendu photo plutôt qu'un globe vide
		UE_LOG(LogCartoImagery, Warning, TEXT("[Carto] %s : rendu photo conservé"), *Error);
		RevertSection(World);
		return;
	}

	// Tileset photo de référence (géoréférence, serveur ion) : Google de préférence
	ACesium3DTileset* PhotoReference = nullptr;
	for (TActorIterator<ACesium3DTileset> It(World); It; ++It)
	{
		if (It->ActorHasTag(TerrainTag)) continue;
		if (!PhotoReference || It->GetActorNameOrLabel().Contains(TEXT("Google"))) PhotoReference = *It;
	}

	const int64 AssetId = int64(GetNumber(Section, TEXT("terrainAssetId"), double(WorldTerrainAssetId)));
	ACesium3DTileset* Relief = EnsureTerrain(World, PhotoReference, AssetId);
	if (!Relief)
	{
		UE_LOG(LogCartoImagery, Warning, TEXT("[Carto] pas de géoréférence Cesium dans ce niveau : rendu photo conservé"));
		return;
	}

	// Décalage vertical du relief (m) : World Terrain est plus lisse que Google, le tracé drapé peut s'y enfoncer sur les crêtes
	const double OffsetM = GetNumber(Section, TEXT("terrainOffsetM"), 0.0);
	const FVector Base = PhotoReference ? PhotoReference->GetActorLocation() : FVector::ZeroVector;
	const FVector Wanted = Base + FVector::UpVector * OffsetM * 100.0;
	if (!Relief->GetActorLocation().Equals(Wanted, 1.0)) Relief->SetActorLocation(Wanted);

	// Carte : même réglages => rien à faire (réapplication) ; sinon on recharge l'overlay
	UCesiumUrlTemplateRasterOverlay* Overlay = MapOverlay.Get();
	const double OverlaySse = FMath::Clamp(GetNumber(Section, TEXT("maximumScreenSpaceError"), 2.0), 0.5, 16.0);
	const bool bChanged = !Overlay || Overlay->TemplateUrl != Map.Url || Overlay->TileWidth != Map.TileSize
		|| Overlay->MaximumLevel != Map.MaximumLevel;
	if (!Overlay)
	{
		Overlay = NewObject<UCesiumUrlTemplateRasterOverlay>(Relief, TEXT("StyleMapOverlay"), RF_Transient);
		Overlay->MaterialLayerKey = OverlayKey;
		Overlay->Projection = ECesiumUrlTemplateRasterOverlayProjection::WebMercator;
		Overlay->bAutoActivate = false;
		Overlay->TemplateUrl = Map.Url;
		Overlay->TileWidth = Map.TileSize;
		Overlay->TileHeight = Map.TileSize;
		Overlay->MaximumLevel = Map.MaximumLevel;
		Overlay->SetMaximumScreenSpaceError(OverlaySse);
		Relief->AddInstanceComponent(Overlay);
		Overlay->RegisterComponent();
		Overlay->Activate(true);  // ajoute la carte au relief
		MapOverlay = Overlay;
	}
	else
	{
		// Chaque setter Cesium recharge la carte : on ne touche qu'à ce qui change
		if (bChanged)
		{
			Overlay->TemplateUrl = Map.Url;
			Overlay->TileWidth = Map.TileSize;
			Overlay->TileHeight = Map.TileSize;
			Overlay->MaximumLevel = Map.MaximumLevel;
		}
		if (!FMath::IsNearlyEqual(Overlay->GetMaximumScreenSpaceError(), OverlaySse)) Overlay->SetMaximumScreenSpaceError(OverlaySse);
		else if (bChanged) Overlay->Refresh();
	}

	bSuspendPhoto = GetString(Section, TEXT("photoTiles"), TEXT("hide")).Equals(TEXT("suspend"), ESearchCase::IgnoreCase);
	WaitTimeout = FMath::Clamp(float(GetNumber(Section, TEXT("swapTimeoutS"), 8.0)), 0.f, 30.f);

	const FString Attribution = GetString(Section, TEXT("attribution"), FString());
	UE_LOG(LogCartoImagery, Log, TEXT("[Carto] %s sur relief ion %lld%s"), *Map.Description, AssetId,
		Attribution.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" (mention : %s)"), *Attribution));

	if (!HiddenPhoto.IsEmpty() || !Relief->IsHidden())
	{
		// Déjà en carte : seul l'état de suspension des photos peut changer
		for (const TPair<TWeakObjectPtr<ACesium3DTileset>, bool>& Entry : HiddenPhoto)
		{
			if (ACesium3DTileset* Photo = Entry.Key.Get()) Photo->SuspendUpdate = bSuspendPhoto || Entry.Value;
		}
		return;
	}

	// Première bascule : le relief charge masqué, les photos restent à l'antenne jusqu'à ce qu'il soit prêt
	StopWaiting();
	WaitElapsed = 0.f;
	if (WaitTimeout <= 0.f)
	{
		SwapToTerrain();
		return;
	}
	WaitHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UCartoImagerySubsystem::TickWaitTerrain), 0.2f);
}

ACesium3DTileset* UCartoImagerySubsystem::EnsureTerrain(UWorld* World, ACesium3DTileset* PhotoReference, int64 AssetId)
{
	using namespace CartoImagery;
	if (ACesium3DTileset* Existing = Terrain.Get())
	{
		if (Existing->GetIonAssetID() != AssetId) Existing->SetIonAssetID(AssetId);
		return Existing;
	}

	ACesiumGeoreference* Georef = PhotoReference ? PhotoReference->ResolveGeoreference() : ACesiumGeoreference::GetDefaultGeoreference(World);
	if (!Georef) return nullptr;

	FActorSpawnParameters Params;
	Params.Name = MakeUniqueObjectName(World->PersistentLevel, ACesium3DTileset::StaticClass(), TEXT("StyleCartoTerrain"));
	Params.ObjectFlags |= RF_Transient;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform Xf = PhotoReference ? PhotoReference->GetActorTransform() : FTransform::Identity;
	ACesium3DTileset* Relief = World->SpawnActor<ACesium3DTileset>(ACesium3DTileset::StaticClass(), Xf, Params);
	if (!Relief) return nullptr;

	Relief->Tags.Add(TerrainTag);
	// Masqué tant qu'il charge : Cesium sélectionne et charge les tuiles d'un tileset masqué
	Relief->SetActorHiddenInGame(true);
	Relief->SetGeoreference(Georef);
	UCesiumIonServer* Server = PhotoReference ? PhotoReference->GetCesiumIonServer() : nullptr;
	Relief->SetCesiumIonServer(Server ? Server : UCesiumIonServer::GetServerForNewObjects());
	if (PhotoReference) Relief->SetIonAccessToken(PhotoReference->GetIonAccessToken());
	Relief->SetTilesetSource(ETilesetSource::FromCesiumIon);
	Relief->SetIonAssetID(AssetId);
	// Les collisions restent celles des photos (masquées mais présentes) : rien à construire ici
	Relief->SetCreatePhysicsMeshes(false);
	Relief->SetCreateNavCollision(false);
	Terrain = Relief;
	return Relief;
}

bool UCartoImagerySubsystem::TickWaitTerrain(float DeltaTime)
{
	ACesium3DTileset* Relief = Terrain.Get();
	if (!Relief)
	{
		WaitHandle.Reset();
		return false;
	}
	WaitElapsed += DeltaTime;
	const float Progress = Relief->GetLoadProgress();
	if (Progress < WaitProgress && WaitElapsed < WaitTimeout) return true;

	if (Progress < WaitProgress)
	{
		UE_LOG(LogCartoImagery, Warning, TEXT("[Carto] relief chargé à %.0f %% après %.1f s : bascule quand même"), Progress, WaitElapsed);
	}
	WaitHandle.Reset();
	SwapToTerrain();
	return false;
}

void UCartoImagerySubsystem::SwapToTerrain()
{
	ACesium3DTileset* Relief = Terrain.Get();
	UWorld* World = StyledWorld.Get();
	if (!Relief || !World) return;

	for (TActorIterator<ACesium3DTileset> It(World); It; ++It)
	{
		ACesium3DTileset* Photo = *It;
		if (Photo == Relief || Photo->ActorHasTag(CartoImagery::TerrainTag) || Photo->IsHidden()) continue;
		HiddenPhoto.Emplace(Photo, Photo->SuspendUpdate);
		Photo->SetActorHiddenInGame(true);
		// "suspend" : plus aucun chargement photo (bande passante), mais le retour en Réaliste recharge la vue
		if (bSuspendPhoto) Photo->SuspendUpdate = true;
	}
	Relief->SetActorHiddenInGame(false);
	UE_LOG(LogCartoImagery, Log, TEXT("[Carto] carte à l'antenne (%d tileset(s) photo masqué(s))"), HiddenPhoto.Num());
}

void UCartoImagerySubsystem::RestorePhotoTilesets()
{
	for (const TPair<TWeakObjectPtr<ACesium3DTileset>, bool>& Entry : HiddenPhoto)
	{
		if (ACesium3DTileset* Photo = Entry.Key.Get())
		{
			Photo->SuspendUpdate = Entry.Value;
			Photo->SetActorHiddenInGame(false);
		}
	}
	HiddenPhoto.Reset();
}

void UCartoImagerySubsystem::RevertSection(UWorld* World)
{
	StopWaiting();
	if (StyledWorld.Get() != World)
	{
		// Monde déjà détruit ou autre monde : rien à défaire ici
		Terrain.Reset();
		MapOverlay.Reset();
		HiddenPhoto.Reset();
		return;
	}
	// Photos d'abord (tuiles encore en cache en mode "hide" : retour instantané), puis le relief disparaît
	RestorePhotoTilesets();
	if (ACesium3DTileset* Relief = Terrain.Get())
	{
		Relief->Destroy();
	}
	Terrain.Reset();
	MapOverlay.Reset();
	UE_LOG(LogCartoImagery, Log, TEXT("[Carto] retour au rendu photo"));
}

void UCartoImagerySubsystem::StopWaiting()
{
	if (WaitHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(WaitHandle);
		WaitHandle.Reset();
	}
}
