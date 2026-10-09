// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "SettingsSaveGame.h"
#include "SettingsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFetchUpdate,
	float,
	FetchValue,
	int64,
	RaceID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnZOffsetUpdate,
	float,
	NewZOffset,
	int64,
	RaceID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPathGlowUpdate,
	float,
	GlowIntensity,
	int64,
	RaceID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPulseFrequencyUpdate,
	float,
	PulseFrequency,
	int64, 
	RaceID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPulseGlowSet,
	float,
	PulseGlow);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMinMaxSet,
	float,
	MinValue,
	float,
	MaxValue,
	int64,
	RaceID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMinMaxDirty,
	float,
	MinValue,
	float,
	MaxValue);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnTrailUrlUpdate,
	FString,
	NewURL);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPitchChanged, float, NewPitch);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLengthChanged, float, NewLength);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZAnchorChanged, float, NewZAnchor);



/**
 * @brief Subsystem de gestion des settings
 */
UCLASS()
class SUBSYSTEMS_API USettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UFUNCTION()
	float GetWeatherFrequency() const;
	UFUNCTION()
	void SetCameraPitch(float NewPitch);
	UFUNCTION()
	float GetCameraPitch() const;
	UFUNCTION()
	void SetArmLength(float NewLength);
	UFUNCTION()
	float GetArmLength() const;
	UFUNCTION()
	void SetZAnchor(float NewZAnchor);
	UFUNCTION()
	float GetZAnchor();
	UFUNCTION()
	void CreateTrailSettingsById(int64 NewRaceID, const FSettings& NewTrailSettings);
	UFUNCTION()
	bool DoesSettingsExist(int64 RaceID);
	UFUNCTION()
	FSettings GetTrailSettingsById(int64 RaceID) const;

	/**
	 * @brief Choix de la fréquence de récupération des datas
	 * @param FetchValue Fréquence de récupération
	 * @param RaceID
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	void SetFetchById(float FetchValue, int64 RaceID);
	float GetFetchById(int64 RaceID) const;
	FORCEINLINE float GetFetchFrequency() const{ return 1.0f;};	// temp property
	FOnFetchUpdate OnFetchUpdate;

	/**
	 * @brief Choix du décalage en Z du tracé de la course
	 * @param ZOffset	Valeur du décalage
	 * @param IdRace		Id de la course
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	void SetZOffsetById(float ZOffset, int64 IdRace);
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	float GetZOffsetById(int64 RaceID) const;
	/**
	 * @brief Une seule fois par course : retire du ZOffset l'ecart du geoide desormais ajoute au trace,
	 * pour que le trace reste a la hauteur reglee auparavant
	 * @param GeoidCm	Ecart geoide/ellipsoide au depart de la course (cm)
	 * @return Le ZOffset a utiliser (cm)
	 */
	float ApplyGeoidCorrectionById(int64 RaceID, float GeoidCm);
	FOnZOffsetUpdate OnZOffsetUpdate;

	/**
	 * @brief Choix de la puissance du glow du tracé
	 * @param GlowValue	Valeur de la puissance
	 * @param IdRace		Id de la course
	 * @note Binded in Path
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	void SetGlowById(float GlowValue, int64 IdRace);
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	float GetGlowById(int64 RaceID) const;
	FOnPathGlowUpdate OnPathGlowUpdate;
	
	/**
	 * @brief Choix de la vitesse du Pulse
	 * @param PulseFrequency	Valeur de la vitesse du pulse
	 * @param IdRace		Id de la course
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	void SetPulseFrequencyById(float PulseFrequency, int64 IdRace);
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	void SetPulseGlowById(float NewPulseGlow, int64 RaceID);
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	float GetPulseFrequencyById(int64 RaceID) const;
	
	/**
	 * @brief Choix de la taille min et max d'un Runner pour une course donnée
	 * @param MinValue
	 * @param MaxValue
	 * @param RaceID	int64
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	void SetMinMaxById(float MinValue, float MaxValue, int64 RaceID);
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	FMinMax GetMinMaxById(int64 RaceID) const;
	FOnMinMaxSet OnMinMaxSet;
	FOnMinMaxDirty OnMinMaxDirty;

	/**
	 * @brief Setting the requests URL
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (BlueprintInternalMethod))
	FORCEINLINE FString GetMainURL() const{ return TrailMainURL;};
	void SetMainURL(FString NewUrl);
	FOnTrailUrlUpdate OnTrailUrlUpdate;

	/**
	 * @brief Changing the Cameras pitch for Checkpoints, Runners & Pois
	 */
	FOnPitchChanged OnPitchChanged;
	FOnLengthChanged OnLengthChanged;
	FOnZAnchorChanged OnZAnchorChanged;
	
	UFUNCTION()
	float GetUpdateIntervalSeconds() const;
	UFUNCTION()
	float GetInterpSpeed() const;
	
	// Save
	bool Save();
	bool Load();

private:
	UPROPERTY()
	int64 RaceId;
	UPROPERTY()
	FSettings TrailSettings;
	UPROPERTY()
	FString TrailMainURL = TEXT("https://simulacre.ltvprod.cc/regie/2/");
	UPROPERTY()
	float TrailGlobalPitch = -25.f;
	UPROPERTY()
	float TrailGlobalArmLength = 50000.f;
	UPROPERTY()
	float TrailGlobalZAnchor = -500.f;
	
	// Setting by RaceID
	UPROPERTY()
	TMap<int64, FSettings> TrailSettingsMap;
	
	// LookAt
	UPROPERTY()
	float LookAtIntervalSeconds;
	UPROPERTY()
	float LookAtInterpSpeed;
	
	// UI
	UPROPERTY()
	float UpdateIntervalSeconds;	
	
	UPROPERTY()
	float WeatherFrequency;
	
	// Save
	UPROPERTY()
	TObjectPtr<USettingsSaveGame> CachedSave;

	static constexpr int32 UserIndex = 0;
	inline static const FString SlotName = TEXT("TrailSimulator_Settings");
};
