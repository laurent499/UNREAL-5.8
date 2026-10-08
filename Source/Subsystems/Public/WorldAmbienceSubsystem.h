// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HttpRouteHandle.h"
#include "HttpResultCallback.h"
#include "WorldAmbienceSubsystem.generated.h"

class IHttpRouter;
class UMaterialParameterCollection;
class UDirectionalLightComponent;
struct FHttpServerRequest;

/**
 * @brief Reglages du monde vivant, modifiables depuis la regie (/monde/set) et sauvegardes.
 */
USTRUCT(BlueprintType)
struct SUBSYSTEMS_API FWorldAmbienceSettings
{
	GENERATED_BODY()

	// --- Meteo et sol ---
	/** La meteo reelle pilote UDW (branche au lot 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoWeather = false;
	/** Duree de sechage du sol apres la pluie (minutes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DryingMinutes = 20.f;
	/** Ligne de neige forcee (m) ; negative = automatique (lot 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SnowLineOverrideM = -1.f;

	// --- Nuit ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHeadlamps = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeadlampIntensity = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCityLights = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CityLightsIntensity = 1.f;

	// --- Faune / flore ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFauna = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FaunaDensity = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFlora = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FloraDensity = 1.f;
	/** Rayon de la flore autour de la camera (m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FloraRadiusM = 150.f;

	// --- Nuages bas ---
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
	float BenchSettleSeconds = 15.f;
	float BenchMeasureSeconds = 20.f;
	FString BenchCsvPath;
	/** Valeurs d'origine restaurees en fin de banc (georeference, camera, UDS, UDW, gardien) */
	FVector SavedGeoOrigin = FVector::ZeroVector;
	FTransform SavedPawnTransform;
	FRotator SavedControlRotation;
	TMap<FString, FString> SavedSkyValues;
	TMap<FString, FString> SavedWeatherValues;
	bool bSavedPerfGuard = true;

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
