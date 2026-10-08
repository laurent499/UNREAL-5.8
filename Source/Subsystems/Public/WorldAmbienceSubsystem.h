// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HttpRouteHandle.h"
#include "HttpResultCallback.h"
#include "TrailSharedTypes.h"
#include "WorldAmbienceSubsystem.generated.h"

class IHttpRouter;
class UMaterialParameterCollection;
class UDirectionalLightComponent;
class UVolumetricCloudComponent;
struct FHttpServerRequest;

/**
 * @brief Reglages du monde vivant, modifiables depuis la regie (/monde/set) et sauvegardes.
 */
USTRUCT(BlueprintType)
struct SUBSYSTEMS_API FWorldAmbienceSettings
{
	GENERATED_BODY()

	// --- Meteo et sol ---
	/** La meteo reelle (OpenWeather du checkpoint le plus proche de la camera) pilote UDW */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoWeather = false;
	/** Intervalle entre deux releves meteo (minutes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AutoWeatherRefreshMinutes = 10.f;
	/** Duree d'une transition complete vers la nouvelle meteo (minutes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float WeatherSmoothMinutes = 3.f;
	/** Duree de sechage du sol apres la pluie (minutes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DryingMinutes = 20.f;
	/** Ligne de neige forcee (m) ; negative = automatique (lot 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SnowLineOverrideM = -1.f;

	// --- Nuit ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHeadlamps = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeadlampIntensity = 1.f;
	/** Taille de la flaque de lumiere des frontales (multiplicateur du rayon) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeadlampSize = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCityLights = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CityLightsIntensity = 1.f;

	// --- Faune / flore ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFauna = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FaunaDensity = 1.f;
	/** Les oiseaux sont visibles tant que la camera est a moins de cette hauteur au-dessus du sol (m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FaunaMaxHeightM = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFlora = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FloraDensity = 1.f;
	/** Rayon de la flore autour de la camera (m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FloraRadiusM = 150.f;

	// --- Nuages bas ---
	/** Base des nuages volumetriques calculee depuis le releve reel (point de rosee) : nuages accroches aux sommets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLowClouds = false;

	// --- Performance ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bPerfGuard = true;
	/** Au-dessus de ce temps d'image (ms) pendant GuardHoldSeconds, le gardien reduit d'un cran */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float GuardHighMs = 38.f;
	/** En dessous, il retablit d'un cran */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float GuardLowMs = 34.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float GuardHoldSeconds = 2.f;
};

/**
 * @brief Etat du monde calcule a chaque mise a jour, recopie dans MPC_World.
 */
USTRUCT(BlueprintType)
struct SUBSYSTEMS_API FWorldAmbienceState
{
	GENERATED_BODY()

	/** 0 = plein jour, 1 = nuit noire (d'apres la hauteur du soleil) */
	UPROPERTY(BlueprintReadOnly) float Night = 0.f;
	/** Hauteur du soleil au-dessus de l'horizon (degres) */
	UPROPERTY(BlueprintReadOnly) float SunElevationDeg = 45.f;
	/** Valeurs UDW (0-10) */
	UPROPERTY(BlueprintReadOnly) float CloudCoverage = 0.f;
	UPROPERTY(BlueprintReadOnly) float Rain = 0.f;
	UPROPERTY(BlueprintReadOnly) float Snow = 0.f;
	UPROPERTY(BlueprintReadOnly) float Fog = 0.f;
	UPROPERTY(BlueprintReadOnly) float Wind = 0.f;
	/** Sol mouille 0-1, monte avec la pluie et seche avec le temps */
	UPROPERTY(BlueprintReadOnly) float Wetness = 0.f;
	/** Altitude de la ligne de neige (m) */
	UPROPERTY(BlueprintReadOnly) float SnowLineM = 3000.f;

	/** Temps d'image mesure : max(GPU, rendu, jeu), lisse (ms) */
	UPROPERTY(BlueprintReadOnly) float FrameMs = 0.f;
	UPROPERTY(BlueprintReadOnly) float GpuMs = 0.f;
	/** Cran de reduction du gardien : 0 = rien, 1 = flore a moitie, 2 = sans flore, 3 = sans faune, 4 = sans nuages bas */
	UPROPERTY(BlueprintReadOnly) int32 GuardLevel = 0;
};

/**
 * @brief Dernier releve meteo reel retenu et valeurs UDW qui en decoulent (affiche dans la regie)
 */
USTRUCT(BlueprintType)
struct SUBSYSTEMS_API FAutoWeatherInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bHasData = false;
	/** Checkpoint dont la meteo est utilisee */
	UPROPERTY(BlueprintReadOnly) FString Source;
	UPROPERTY(BlueprintReadOnly) int32 Sources = 0;
	UPROPERTY(BlueprintReadOnly) float DistanceKm = 0.f;
	UPROPERTY(BlueprintReadOnly) double SourceLat = 0.0;
	UPROPERTY(BlueprintReadOnly) double SourceLon = 0.0;
	UPROPERTY(BlueprintReadOnly) int32 AgeMinutes = 0;
	UPROPERTY(BlueprintReadOnly) FString Description;
	UPROPERTY(BlueprintReadOnly) float SourceAltitudeM = 0.f;
	UPROPERTY(BlueprintReadOnly) float TempC = 0.f;
	UPROPERTY(BlueprintReadOnly) float CameraAltitudeM = 0.f;
	/** Temperature ramenee a l'altitude de la camera, plafonnee a 1000 m au-dessus du releve (-0,65 degre / 100 m) */
	UPROPERTY(BlueprintReadOnly) float CameraTempC = 0.f;
	UPROPERTY(BlueprintReadOnly) float DewPointC = 0.f;
	/** Base des nuages estimee (m, niveau de la mer) : altitude du releve + 125 m par degre d'ecart temperature / point de rosee */
	UPROPERTY(BlueprintReadOnly) float CloudBaseM = 0.f;
	UPROPERTY(BlueprintReadOnly) int32 Clouds = 0;
	UPROPERTY(BlueprintReadOnly) float Rain1h = 0.f;
	UPROPERTY(BlueprintReadOnly) float Snow1h = 0.f;
	UPROPERTY(BlueprintReadOnly) int32 Visibility = 0;
	UPROPERTY(BlueprintReadOnly) int32 Humidity = 0;
	UPROPERTY(BlueprintReadOnly) float WindMs = 0.f;
	UPROPERTY(BlueprintReadOnly) int32 WindDeg = 0;
	/** Cibles UDW calculees (0-10) */
	UPROPERTY(BlueprintReadOnly) TMap<FString, float> Targets;
	/** Parametres forces a la main depuis la regie (prioritaires sur la meteo reelle) */
	UPROPERTY(BlueprintReadOnly) TMap<FString, float> Forced;
};

/**
 * @brief Un plan du banc de test (RemoteControl/BancTest.json)
 */
struct FBenchShot
{
	FString Name;
	double Lat = 0.0;
	double Lon = 0.0;
	/** Altitude ellipsoidale de la camera (m), ignoree si AboveGroundM >= 0 */
	double HeightM = 1000.0;
	/** Hauteur au-dessus du sol (m) : la camera est posee par un lancer de rayon apres chargement des tuiles */
	double AboveGroundM = -1.0;
	float Pitch = 0.f;
	float Yaw = 0.f;
	/** Heure UDS (0-2400) */
	float TimeOfDay = 1200.f;
	/** Valeurs UDW forcees le temps du plan ("Cloud Coverage", "Rain", ...) */
	TMap<FString, float> Weather;
};

/**
 * @brief Resultat d'un plan du banc de test
 */
struct FBenchResult
{
	FString Name;
	float FrameAvg = 0.f;
	float FrameP99 = 0.f;
	float GpuAvg = 0.f;
	float GpuP99 = 0.f;
	int32 Samples = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldAmbienceChanged, const FWorldAmbienceSettings&, Settings);

/**
 * @brief Cerveau du monde vivant.
 *
 * Calcule l'etat du monde (nuit, meteo UDW, sol mouille, ligne de neige), l'ecrit dans
 * MPC_World pour les materiaux et Niagara, et surveille le temps d'image : au-dela de
 * GuardHighMs il coupe d'abord la flore, puis la faune, puis les nuages bas.
 *
 * Regie : GET /monde/state et GET /monde/set?cle=valeur sur le port du Remote Control.
 */
UCLASS()
class SUBSYSTEMS_API UWorldAmbienceSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintPure, Category = "WorldAmbience")
	const FWorldAmbienceSettings& GetSettings() const { return Settings; }

	UFUNCTION(BlueprintPure, Category = "WorldAmbience")
	const FWorldAmbienceState& GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "WorldAmbience")
	void SetSettings(const FWorldAmbienceSettings& NewSettings);

	/** Change un reglage par son nom (ex. "bCityLights", "1") et l'enregistre ; utilise par la regie /command */
	bool SetSettingByName(const FString& Key, const FString& Value);

	/** Valeurs effectives, apres reglages regie et gardien de performance */
	UFUNCTION(BlueprintPure, Category = "WorldAmbience")
	float GetEffectiveFloraDensity() const;
	UFUNCTION(BlueprintPure, Category = "WorldAmbience")
	float GetEffectiveFaunaDensity() const;
	UFUNCTION(BlueprintPure, Category = "WorldAmbience")
	bool AreLowCloudsAllowed() const;

	/** Diffuse a chaque changement de reglage ou de cran du gardien */
	UPROPERTY(BlueprintAssignable, Category = "WorldAmbience")
	FOnWorldAmbienceChanged OnAmbienceChanged;

private:
	static constexpr uint32 ListenPort = 30010;
	static constexpr float UpdateInterval = 0.25f;

	void UpdateState(float DeltaTime);
	void UpdatePerfGuard(float DeltaTime);
	void PushToMPC() const;
	void ReadWeatherActor();
	UDirectionalLightComponent* FindSunLight();
	void EnsureNightLightsOverlay();
	bool bNightLightsAdded = false;

	void LoadSettings();
	void SaveSettings() const;
	bool ApplySetting(const FString& Key, const FString& Value);
	FString StateJson() const;

	void BindRoutes();
	void UnbindRoutes();
	bool HandleStateRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool HandleSetRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool HandleBenchStartRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool HandleBenchStateRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	// --- Banc de test : place la camera sur chaque plan, attend le chargement, mesure, restaure ---
	enum class EBenchPhase : uint8 { Idle, Settling, Measuring };
	bool StartBench(int32 OnlyShot, FString& OutError);
	void TickBench(float DeltaTime);
	void ApplyBenchShot(const FBenchShot& Shot);
	void PlaceCameraOnGround(const FBenchShot& Shot);
	void FinishBenchShot();
	void EndBench();
	FString BenchJson() const;

	TArray<FBenchShot> BenchShots;
	TArray<FBenchResult> BenchResults;
	TArray<float> BenchFrameSamples;
	TArray<float> BenchGpuSamples;
	EBenchPhase BenchPhase = EBenchPhase::Idle;
	int32 BenchIndex = 0;
	float BenchTimer = 0.f;
	bool bBenchGrounded = false;
	float BenchLoadedTime = 0.f;
	/** Attente maximale du chargement des tuiles par plan */
	float BenchSettleSeconds = 60.f;
	float BenchMeasureSeconds = 20.f;
	FString BenchCsvPath;
	/** Valeurs d'origine restaurees en fin de banc (georeference, camera, UDS, UDW, gardien) */
	FVector SavedGeoOrigin = FVector::ZeroVector;
	FTransform SavedPawnTransform;
	FRotator SavedControlRotation;
	TMap<FString, FString> SavedSkyValues;
	TMap<FString, FString> SavedWeatherValues;
	bool bSavedPerfGuard = true;

	// --- Meteo reelle ---
	struct FWeatherSample
	{
		FString Name;
		double Lat = 0.0;
		double Lon = 0.0;
		float ElevationM = 0.f;
		FOpenWeatherCurrent Current;
		FString Description;
		FDateTime Received;
	};
	void TickAutoWeather(float DeltaTime);
	/** Lance les releves ; renvoie false s'il n'y a encore aucun checkpoint avec meteo */
	bool RequestAutoWeather();
	UFUNCTION()
	void HandleCheckpointsGathered(int64 RaceID, FCheckpoints AllCheckpoints);
	/** Checkpoints de toutes les courses chargees : la meteo du plus proche de la camera l'emporte */
	TMap<int64, FCheckpoints> KnownCheckpoints;
	/** Au-dela, le releve du checkpoint le plus proche n'est pas representatif du lieu filme */
	static constexpr float MaxSourceDistanceKm = 50.f;
	void ComputeAutoWeather();
	bool GetCameraLongLatHeight(FVector& OutLLH) const;
	void WriteWeatherParam(const FString& Key, float Value);
	bool HandleForceRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool HandleReleaseRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	TArray<FWeatherSample> WeatherSamples;
	FAutoWeatherInfo AutoWeather;
	/** Valeurs UDW effectivement appliquees, qui glissent vers les cibles */
	TMap<FString, float> AppliedWeather;
	float WeatherRefreshTimer = 0.f;
	int32 WeatherRequestSerial = 0;
	bool bWasAutoWeather = false;

	// --- Base des nuages ---
	void UpdateCloudBase(float DeltaTime);
	TWeakObjectPtr<UVolumetricCloudComponent> CloudComponent;
	float OriginalCloudBottomKm = -1.f;
	float CurrentCloudBottomKm = -1.f;
	FHttpRouteHandle ForceRouteHandle;
	FHttpRouteHandle ReleaseRouteHandle;

	/** Temps ecoule depuis le BeginPlay : le gardien ignore le chargement initial */
	float TimeSinceBeginPlay = 0.f;

	float LastRawFrameMs = 0.f;
	float LastRawGpuMs = 0.f;

	FWorldAmbienceSettings Settings;
	FWorldAmbienceState State;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> WorldMPC;

	TWeakObjectPtr<UDirectionalLightComponent> SunLight;
	TWeakObjectPtr<AActor> WeatherActor;

	float UpdateAccumulator = 0.f;
	float FindAccumulator = 0.f;
	float GuardHighTime = 0.f;
	float GuardLowTime = 0.f;
	bool bBegunPlay = false;

	TSharedPtr<IHttpRouter> HttpRouter;
	FHttpRouteHandle StateRouteHandle;
	FHttpRouteHandle SetRouteHandle;
	FHttpRouteHandle BenchStartRouteHandle;
	FHttpRouteHandle BenchStateRouteHandle;
};
