// MeteoPageSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "HttpRouteHandle.h"
#include "MeteoPageSubsystem.generated.h"

class IHttpRouter;
struct FHttpServerRequest;

/**
 * Sert la page de controle meteo (RemoteControl/Meteo.html) sur le port du Remote Control :
 * http://<IP du PC Unreal>:30010/meteo
 * Le fichier est relu a chaque requete : une modification de la page est prise en compte sans recompiler.
 */
UCLASS()
class SUBSYSTEMS_API UMeteoPageSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// Port du serveur Remote Control (Project Settings > Remote Control > Web Server Port)
	static constexpr uint32 ListenPort = 30010;

	TSharedPtr<IHttpRouter> HttpRouter;
	FHttpRouteHandle PageRouteHandle;
	FHttpRouteHandle GeoRouteHandle;
	FHttpRouteHandle GeoSetRouteHandle;

	bool HandlePageRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const;

	// Lieu du georeference Cesium de la partie en cours (lecture PIE ou Standalone)
	// GET /meteo/geo                               -> {"lat":..,"lon":..,"height":..}
	// GET /meteo/geo/set?lat=..&lon=..[&height=..] -> deplace l'origine et renvoie le nouveau lieu
	bool HandleGeoRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const;
	bool HandleGeoSetRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) const;
};
