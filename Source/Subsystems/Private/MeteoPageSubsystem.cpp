// MeteoPageSubsystem.cpp
#include "MeteoPageSubsystem.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpPath.h"
#include "HttpServerResponse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CesiumGeoreference.h"

namespace
{
	// Monde de la partie en cours : lecture dans l'editeur en priorite, sinon jeu Standalone
	UWorld* FindPlayWorld()
	{
		if (!GEngine) return nullptr;
		UWorld* Game = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (!World) continue;
			if (Context.WorldType == EWorldType::PIE) return World;
			if (Context.WorldType == EWorldType::Game) Game = World;
		}
		return Game;
	}

	ACesiumGeoreference* FindGeoreference()
	{
		UWorld* World = FindPlayWorld();
		return World ? Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(World, ACesiumGeoreference::StaticClass())) : nullptr;
	}

	TUniquePtr<FHttpServerResponse> JsonResponse(const FString& Json)
	{
		TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(Json, TEXT("application/json"));
		Response->Headers.Add(TEXT("Access-Control-Allow-Origin"), { TEXT("*") });
		return Response;
	}

	FString GeoJson(const ACesiumGeoreference* Geo)
	{
		// "level" : chemin du niveau joue (UEDPIE_0_MainMap en lecture dans l'editeur, MainMap en Standalone),
		// pour que la regie pilote les acteurs de la partie en cours et pas ceux de la map ouverte dans l'editeur
		const UWorld* World = Geo->GetWorld();
		const FString Level = World ? World->GetPathName() + TEXT(":PersistentLevel.") : FString();
		return FString::Printf(TEXT("{\"ok\":true,\"lat\":%.8f,\"lon\":%.8f,\"height\":%.2f,\"level\":\"%s\"}"),
			Geo->GetOriginLatitude(), Geo->GetOriginLongitude(), Geo->GetOriginHeight(), *Level);
	}

	const TCHAR* NoGameJson = TEXT("{\"ok\":false,\"error\":\"Aucune partie en cours (lance la lecture ou le Standalone)\"}");
}

void UMeteoPageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Meme listener que le Remote Control : GetHttpRouter renvoie le routeur existant s'il est deja ouvert
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	HttpRouter = HttpServerModule.GetHttpRouter(ListenPort, /*bFailOnBindFailure=*/true);
	if (!HttpRouter.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Meteo] Port %d indisponible : page meteo non servie"), ListenPort);
		return;
	}

	PageRouteHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/meteo")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UMeteoPageSubsystem::HandlePageRequest));

	if (!PageRouteHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Meteo] Impossible de monter la route /meteo"));
		HttpRouter.Reset();
		return;
	}

	GeoRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/meteo/geo")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UMeteoPageSubsystem::HandleGeoRequest));
	GeoSetRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/meteo/geo/set")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UMeteoPageSubsystem::HandleGeoSetRequest));

	HttpServerModule.StartAllListeners();
	UE_LOG(LogTemp, Log, TEXT("[Meteo] Page meteo servie sur http://<IP>:%d/meteo"), ListenPort);
}

void UMeteoPageSubsystem::Deinitialize()
{
	if (HttpRouter.IsValid())
	{
		for (FHttpRouteHandle* Handle : { &PageRouteHandle, &GeoRouteHandle, &GeoSetRouteHandle })
		{
			if (Handle->IsValid()) HttpRouter->UnbindRoute(*Handle);
			Handle->Reset();
		}
	}
	HttpRouter.Reset();

	Super::Deinitialize();
}

bool UMeteoPageSubsystem::HandlePageRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const
{
	const FString FilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("RemoteControl"), TEXT("Meteo.html"));

	FString Html;
	TUniquePtr<FHttpServerResponse> Response;
	if (FFileHelper::LoadFileToString(Html, *FilePath))
	{
		Response = FHttpServerResponse::Create(Html, TEXT("text/html"));
	}
	else
	{
		Response = FHttpServerResponse::Create(
			FString::Printf(TEXT("Page meteo introuvable : %s"), *FPaths::ConvertRelativePathToFull(FilePath)),
			TEXT("text/plain"));
		Response->Code = EHttpServerResponseCodes::NotFound;
	}

	OnComplete(MoveTemp(Response));
	return true;
}

bool UMeteoPageSubsystem::HandleGeoRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const
{
	const ACesiumGeoreference* Geo = FindGeoreference();
	OnComplete(JsonResponse(Geo ? GeoJson(Geo) : FString(NoGameJson)));
	return true;
}

bool UMeteoPageSubsystem::HandleGeoSetRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const
{
	ACesiumGeoreference* Geo = FindGeoreference();
	if (!Geo)
	{
		OnComplete(JsonResponse(NoGameJson));
		return true;
	}

	const FString* Lat = Request.QueryParams.Find(TEXT("lat"));
	const FString* Lon = Request.QueryParams.Find(TEXT("lon"));
	const FString* Height = Request.QueryParams.Find(TEXT("height"));
	if (!Lat || !Lon || !Lat->IsNumeric() || !Lon->IsNumeric() || (Height && !Height->IsNumeric()))
	{
		OnComplete(JsonResponse(TEXT("{\"ok\":false,\"error\":\"Parametres attendus : lat, lon (et height optionnel), en degres decimaux\"}")));
		return true;
	}

	const double NewLat = FCString::Atod(**Lat);
	const double NewLon = FCString::Atod(**Lon);
	const double NewHeight = Height ? FCString::Atod(**Height) : Geo->GetOriginHeight();
	if (FMath::Abs(NewLat) > 90.0 || FMath::Abs(NewLon) > 180.0)
	{
		OnComplete(JsonResponse(TEXT("{\"ok\":false,\"error\":\"Latitude entre -90 et 90, longitude entre -180 et 180\"}")));
		return true;
	}

	Geo->SetOriginLongitudeLatitudeHeight(FVector(NewLon, NewLat, NewHeight));
	UE_LOG(LogTemp, Log, TEXT("[Meteo] Origine Cesium deplacee : lat %.6f lon %.6f h %.1f"), NewLat, NewLon, NewHeight);
	OnComplete(JsonResponse(GeoJson(Geo)));
	return true;
}
