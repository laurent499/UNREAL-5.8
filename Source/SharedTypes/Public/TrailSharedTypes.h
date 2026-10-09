// Source/SharedTypes/Public/TrailSharedTypes.h
#pragma once

#include "CoreMinimal.h"
#include "TrailSharedTypes.generated.h"

/**
 * @brief Types de données utilisés pour décrire les objets JSON
 */ 

/**
 * @brief Etat Stacked/Unstacked d'un runner
 */
UENUM(BlueprintType)
enum class EStackState : uint8 {
	ESS_Unstacked UMETA(DisplayName = "Unstacked"),
	ESS_Stacked UMETA(DisplayName = "Stacked")
};

/**
 * @brief Template types
 */
UENUM(BlueprintType)
enum class ERunnerTemplate : uint8
{
	ERT_UTMB	UMETA(DisplayName = "UTMB"),
	ERT_GTWS	UMETA(DisplayName = "GTWS"),
	ERT_GEN		UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FMinMax
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FVector Min = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FVector Max = FVector();
	
	FMinMax() = default;
	FMinMax(FVector Min, FVector Max) : Min(Min), Max(Max){}
	
	FString ToString() const
	{
		return FString::Printf(TEXT("Min: %s - Max: %s"), *Min.ToString(), *Max.ToString());
	}
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int64 RaceID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString RaceURL = TEXT("https://simulacre.ltvprod.cc/regie/1/");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float FetchFrequency = 0.0f;	// sec
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float ZOffset = 0.0f;			// cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float PulseFrequency = 0.0f;	// sec
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float GlowValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float PulseGlowValue = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 RegieNumber = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FMinMax MinMax = FMinMax();
	float Pitch = -25.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float Length = 50000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float ZAnchor = -500.0f;
	/** Le trace ajoute l'ecart du geoide (EGM96) a l'altitude GPS ; true une fois le ZOffset ajuste en consequence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bGeoidCorrected = false;
	
	FSettings() = default;
	FSettings(int64 NewRaceID,
			  const FString& NewRaceUrl,
			  float NewFetchFrequency,
			  float NewZOffset,
			  float NewPulseFrequency,
			  float NewGlowValue,
			  int32 NewRegieNumber,
			  const FMinMax& NewMinMax,
			  float NewPitch,
			  float NewLength,
			  float NewZAnchor) :	
	RaceID(NewRaceID),
	RaceURL(NewRaceUrl),
	FetchFrequency(NewFetchFrequency),
	ZOffset(NewZOffset),
	PulseFrequency(NewPulseFrequency),
	GlowValue(NewGlowValue),
	RegieNumber(NewRegieNumber),
	MinMax(NewMinMax),
	Pitch(NewPitch),
	Length(NewLength),
	ZAnchor(NewZAnchor)
	{}
	
	FString ToString() const
	{
		return FString::Printf(
		TEXT("RaceID: %lld - RaceURL: %s - Fetch: %f - ZOffset: %f - Pulse: %f - Glow: %f - Regie: %i - MinMax: %s - Pitch: %f - ArmLength: %f - ZAnchor: %f"),
		RaceID, *RaceURL, FetchFrequency, ZOffset, PulseFrequency, GlowValue, RegieNumber, *MinMax.ToString(), Pitch, Length, ZAnchor);
	}
};

/**
 * @brief Structure du Setup d'une course
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRaceSetup
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "raceId"))
	int64 raceId = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "raceName"))
	FString raceName = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "color"))
	FString color = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "elevationGain"))
	FString elevationGain = ""; // "6050 M+ 
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "distance"))
	FString distance= "";      // "176 KM "
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "description"))
	FString description = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "cat"))
	FString cat = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "departureCity"))
	FString departureCity = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "arrivalCity"))
	FString arrivalCity = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "departureCountry"))
	FString departureCountry = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "arrivalCountry"))
	FString arrivalCountry = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "departureCountryFlag"))
	FString departureCountryFlag = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "arrivalCountryFlag"))
	FString arrivalCountryFlag = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "templateName"))
	FString templateName = "";
	
	FRaceSetup() = default;
	FRaceSetup(
		int64 NewraceId,
		FString NewraceName,
		FString Newcolor,
		FString NewelevationGain,
		FString Newdistance,
		FString Newdescription,
		FString Newcat,
		FString NewdepartureCity,
		FString NewarrivalCity,
		FString NewdepartureCountry,
		FString NewarrivalCountry,
		FString NewdepartureCountryFlag,
		FString NewarrivalCountryFlag,
		FString NewtemplateName) :
		raceId(NewraceId),
		raceName(NewraceName),
		color(Newcolor),
		elevationGain(NewelevationGain),
		distance(Newdistance),
		description(Newdescription),
		cat(Newcat),
		departureCity(NewdepartureCity),
		arrivalCity(NewarrivalCity),
		departureCountry(NewdepartureCountry),
		arrivalCountry(NewarrivalCountry),
		departureCountryFlag(NewdepartureCountryFlag),
		arrivalCountryFlag(NewarrivalCountryFlag),
		templateName(NewtemplateName){}
};

USTRUCT(BlueprintType)
struct FRaceSetupRoot
{
	GENERATED_BODY()
	UPROPERTY(meta = (JsonProperty = "setup"))
	FRaceSetup Setup = FRaceSetup();
	
	FRaceSetupRoot() = default;
};

/**
 * @brief Structure du POI
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRacePOI
{
    GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "poiId"))
    int64 poiId = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "name"))
    FString name = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "type"))
    FString type = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "lat"))
    float lat = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "lon"))
    float lon = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "elevation"))
    float elevation = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "weather"))
    FString weather = "";
	
	FRacePOI() = default;
	FRacePOI(int64 NewpoiId,
			FString Newname,
			FString Newtype,
			float Newlat,
			float Newlon,
			float Newelevation,
			FString Newweather) :
	poiId(NewpoiId),
	name(Newname),
	type(Newtype),
	lat(Newlat),
	lon(Newlon),
	elevation(Newelevation),
	weather(Newweather){}
	
};

/**
 * @brief Structure du tableau des POIs
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FPOIs
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)	
	TArray<FRacePOI> POIs;
	
	FPOIs() = default;
};

/**
 * @brief Structure du Checkpoint
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRaceCheckpoint
{
    GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "checkpointId"))
	int64 checkpointId = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "name"))
    FString name = "";
    UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "distance"))
    float distance = 0.0;
    UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "elevation"))
    float elevation = 0.0;
    UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "altitude"))
	float altitude = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "type"))
    FString type = "";
    UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "weather"))
	FString weather = "";
	
	FRaceCheckpoint() = default;
	FRaceCheckpoint(int64 NewcheckpointId,
					FString Newname,
					float Newdistance,
					float Newelevation,
					float Newaltitude,
					FString Newtype,
					FString Newweather) : 
	checkpointId(NewcheckpointId),
	name(Newname),
	distance(Newdistance),
	elevation(Newelevation),
	altitude(Newaltitude),
	type(Newtype),
	weather(Newweather){}
};

/**
 * @brief Structure du tableau de checkpoints
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FCheckpoints
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)	
	TArray<FRaceCheckpoint> Checkpoints;
	FCheckpoints() = default;
};

/**
 * @brief Structure d'un point du tracé
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRacePathPoint
{
    GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "ele"))
    float ele = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "lat"))
    float lat = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "lon"))
    float lon = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "datas"))
    FRaceCheckpoint datas = FRaceCheckpoint();
	
	FRacePathPoint() = default;
	FRacePathPoint(float Newele,
					float Newlat,
					float Newlon,
					FRaceCheckpoint Newdatas) : 
	ele(Newele),
	lat(Newlat),
	lon(Newlon),
	datas(Newdatas){}
};

/**
 * @brief Structure du tracé
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRacePath
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	TArray<FRacePathPoint> Points;
	
	FRacePath() = default;
};

/**
 * @brief Structure d'un Runner
 * @endpoint /race/4/runner/IdRunner
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRunnerStruct
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "runnerId"))
	int64 runnerId = 0;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "canalId"))
	int64 canalId = 0;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "canal"))
	FString canal = "";
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "dossard"))
	int32 dossard = 0;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "nom"))
	FString nom = "";
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "pays"))
	FString pays = "";
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "position"))
	FString position = "";
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "indexM"))
	int32 IndexM = 0;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "club"))
	FString club = "";
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "photo"))
	FString photo = "";
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "rank"))
	int32 rank = 0;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "ranksex"))
	int32 ranksex = 0;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "raceName"))
	FString raceName = "";
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "lat"))
	float lat=0.f;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "lon"))
	float lon=0.f;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "elevation"))
	float elevation=0.f;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "kmActuel"))
	float kmActuel = 0.f;
	UPROPERTY(BlueprintReadOnly, meta=(JsonProperty = "speed"))
	float speed = 0.f;
	UPROPERTY(BlueprintReadOnly)
	FMinMax MinMax = FMinMax();
	UPROPERTY(BlueprintReadOnly)
	float VDelta = 0.f;
	
	FRunnerStruct() = default;	
	FRunnerStruct(	int64 NewrunnerId,
					int64 NewcanalId,
					FString Newcanal,
					int32 Newdossard,
					FString Newnom,
					FString Newpays,
					FString Newposition,
					int32 NewIndexM,
					FString Newclub,
					FString Newphoto,
					int32 Newrank,
					int32 Newranksex,
					FString NewraceName,
					float Newlat,
					float Newlon,
					float Newelevation,
					float NewkmActuel,
					float Newspeed,
					FMinMax NewMinMax,
					float NewVDelta) : 
	runnerId(NewrunnerId),
	canalId(NewcanalId),
	canal(Newcanal),
	dossard(Newdossard),
	nom(Newnom),
	pays(Newpays),
	position(Newposition),
	IndexM(NewIndexM),
	club(Newclub),
	photo(Newphoto),
	rank(Newrank),
	ranksex(Newranksex),
	raceName(NewraceName),
	lat(Newlat),
	lon(Newlon),
	elevation(Newelevation),
	kmActuel(NewkmActuel),
	speed(Newspeed),
	MinMax(NewMinMax),
	VDelta(NewVDelta){}
};

/**
 * @brief Structure tu tableau des Runners
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRunners
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	TArray<FRunnerStruct> Runners;
	
	FRunners() = default;
};

/**
 * @brief Structure d'une course
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRaceStruct
{
    GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	int64 raceId = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "setup"))
    FRaceSetup setup = FRaceSetup();
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "poi"))
    FPOIs poi = FPOIs();
	UPROPERTY(BlueprintReadOnly)
	FCheckpoints Checkpoints = FCheckpoints();
	UPROPERTY(BlueprintReadOnly)
	FRunners RaceRunners = FRunners();
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "path"))
    FRacePath path = FRacePath();
	
	FRaceStruct() = default;
	FRaceStruct(int64 NewraceId,
				FRaceSetup Newsetup,
				FPOIs Newpoi,
				FCheckpoints NewCheckpoints,
				FRunners NewRaceRunners,
				FRacePath Newpath) :
	raceId(NewraceId),
	setup(Newsetup),
	poi(Newpoi),
	Checkpoints(NewCheckpoints),
	RaceRunners(NewRaceRunners),
	path(Newpath){}
};

/* RaceEntry Structure */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRaceEntry
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "raceId"))
	int64 raceId = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "name"))
	FString name = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "templateName"))
	FString templateName = "";
	
	FRaceEntry() = default;
	FRaceEntry(	int64 raceId,
				FString name,
				FString templateName) : raceId(raceId), name(name), templateName(templateName){}
};

/**
 * @brief Structure de l'endpoint /races
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRaceEntries
{
    GENERATED_BODY()
UPROPERTY(BlueprintReadOnly)
    TArray<FRaceEntry> RacesEntries;
	
	FRaceEntries() = default;
};

/**
 * @brief Structure d'une Team/Canal
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FTeamStruct
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "canalId"))
	int64 canalId = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "canal"))
	FString canal = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "runnerId"))
	int64 runnerId = 0;
	UPROPERTY(BlueprintReadOnly)
	int64 RaceId = 0;
	
	FTeamStruct() = default;
	FTeamStruct(int64 NewcanalId,
				FString Newcanal,
				int64 NewrunnerId,
				int64 NewRaceId) : 
	canalId(NewcanalId),
	canal(Newcanal),
	runnerId(NewrunnerId),
	RaceId(NewRaceId){}
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FTeams
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	int64 RaceId = 0;
	UPROPERTY(BlueprintReadOnly)
	TArray<FTeamStruct> Teams;
	
	FTeams() = default;
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API  FRaceTeamRunnerIndex
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	TMap<int64, FRunnerStruct> TeamToRunner; // TeamID -> Runner struct
	UPROPERTY(BlueprintReadOnly)
	TMap<int64, int64>       RunnerToTeam; // RunnerID -> TeamID
};

/**
 * @brief Association Team/Runner
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FTeamRunner
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	int64 RunnerId = 0;
	UPROPERTY(BlueprintReadOnly)
	int64 TeamId = 0;
	
	FTeamRunner() = default;
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FRaceTeamsRunners
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	int64 RaceId = 0;
	UPROPERTY(BlueprintReadOnly)
	TArray<FTeamRunner> TeamRunners;
	FRaceTeamsRunners() = default;
};
	
/**
 * @brief Structures de la météo
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FOpenWeatherCondition
{
    GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "id"))
    int32 id = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "main"))
    FString main = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "description"))
    FString description = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "icon"))
    FString icon = "";
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FOpenWeatherCurrent
{
    GENERATED_BODY()

    // Timestamp Unix de la mesure (cle JSON "dt", le nom du champ doit la reprendre pour etre lu)
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "dt"))
    int64 dt = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "sunrise"))
    int64 sunrise = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "sunset"))
    int64 sunset = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "temp"))
    float temp = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "feels_like"))
    float feels_like = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "pressure"))
    int32 pressure = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "humidity"))
    int32 humidity = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "dew_point"))
    float dew_point = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "uvi"))
    float uvi = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "clouds"))
    int32 clouds = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "visibility"))
    int32 visibility = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "wind_speed"))
    float wind_speed = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "wind_deg"))
    int32 wind_deg = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "weather"))
    TArray<FOpenWeatherCondition> weather;
	// Precipitations de la derniere heure en mm (cles JSON "rain" / "snow" -> "1h").
	// Le convertisseur JSON ne sait pas lire une cle "1h" : remplis a la main par UWeatherSubsystem.
	UPROPERTY(BlueprintReadOnly)
    float rain_1h = 0.0;
	UPROPERTY(BlueprintReadOnly)
    float snow_1h = 0.0;
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FOpenWeatherResponse
{
    GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "lat"))
    float lat = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "lon"))
    float lon = 0.0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "timezone"))
    FString timezone = "";
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "timezone_offset"))
    int32 timezone_offset = 0;
	UPROPERTY(BlueprintReadOnly, meta = (JsonProperty = "current"))
    FOpenWeatherCurrent current = FOpenWeatherCurrent();
	
	FOpenWeatherResponse() = default;
};

/**
 * @brief Slope segment
 */
USTRUCT(BlueprintType)
struct FPointIndexSegment
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	int32 StartIndex = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 EndIndex   = 0;
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> Indices;
	UPROPERTY(BlueprintReadOnly)
	float StartDistanceCm = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float EndDistanceCm   = 0.f;
	
	FPointIndexSegment() = default;
};

/**
 * @brief TeamGroups
 */
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FGroupStruct
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	int64 groupId = 0;
	UPROPERTY(BlueprintReadOnly)
	TMap<int64, FRunnerStruct> Teams;
	
	FGroupStruct() = default;
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FGroups
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	int64 RaceId = 0;
	UPROPERTY(BlueprintReadOnly)
	TArray<FGroupStruct> Groups;
	
	FGroups() = default;
};

// Scale 
USTRUCT(BlueprintType)
struct SHAREDTYPES_API FDistanceScaleConfig
{
	GENERATED_BODY()

	// Min/Max voulus par le client (par défaut Min=5, Max=40)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale")
	// FMinMax ScaleMinMax = FMinMax(FVector(10.f), FVector(70.f));
	FMinMax ScaleMinMax = FMinMax();

	// Distance (cm) à laquelle on est à ScaleMin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale", meta=(ClampMin="0.0"))
	float NearDistanceCm = 0.f;

	// Distance (cm) à laquelle on revient à ScaleMax
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale", meta=(ClampMin="0.0"))
	float FarDistanceCm = 0.f; 

	// Souvent préférable dans un monde "globe" : ne considérer que XY
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale")
	bool bUseXYOnly = true;

	// Courbe douce (smoothstep) au lieu de linéaire
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale")
	bool bSmoothStep = true;

	// Utiliser le scale d'origine de l'actor comme "Max" (40 chez toi)
	// Si false, on prendra ScaleMinMax.Max
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale")
	bool bUseActorOriginalScaleAsMax = true;

	// Optionnel : lisser les variations (sinon "snap" au target à chaque update)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale")
	bool bInterp = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale", meta=(EditCondition="bInterp", ClampMin="0.0"))
	float InterpSpeed = 8.f;

	// Fréquence de mise à jour (pas de tick)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perf", meta=(ClampMin="0.01"))
	float UpdateIntervalSec = 0.015f;

	// Evite de spam SetActorScale3D() si le delta est microscopique
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perf", meta=(ClampMin="0.0"))
	float ScaleEpsilon = 0.02f;
};

USTRUCT(BlueprintType)
struct SHAREDTYPES_API FEntry
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> Actor;
	FVector OriginalScale = FVector(1.f);
	FDistanceScaleConfig OverrideConfig;
	FMinMax CurrentMinMax; 
};

// Stacking
USTRUCT(BlueprintType)
struct FRunnerStackingConfig
{
	GENERATED_BODY()

	// Rayon quand on est "près" (scale petit) ⇒ on stacke moins
	UPROPERTY(EditAnywhere, Category="Stacking")
	float RadiusNearCm = 1.f; // 1m

	// Rayon quand on est "loin" (scale proche du scale d'origine) => on stacke plus
	UPROPERTY(EditAnywhere, Category="Stacking")
	float RadiusFarCm = 200000.f; // 2km

	// Courbe de réponse : 1 = linéaire, >1 = transition plus progressive
	UPROPERTY(EditAnywhere, Category="Stacking")
	float Gamma = 2.0f;

	// Timer du subsystem (pas de tick par runner)
	UPROPERTY(EditAnywhere, Category="Perf", meta=(ClampMin="0.02"))
	float UpdateIntervalSec = 0.05f;

	// Hystérésis pour éviter le flicker autour du seuil (en cm)
	UPROPERTY(EditAnywhere, Category="Stacking", meta=(ClampMin="0.0"))
	float HysteresisCm = 10.f;
};

USTRUCT()
struct FStackEntry
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> Runner;

	// Vérité "track" (indépendante du visuel)
	FTransform TrackTransform = FTransform::Identity;
	float TrackDistanceMeters = 0.f;

	// Pour rayon dynamique via scale
	float OriginalScaleX = 1.f;

	// Hauteur du pied (hook au sommet) en cm, pour empiler
	float HookHeightCm = 100.f;
	
	FVector HookLocalInAttach;

	bool bPhotoVisible = true;
	bool bClubVisible = true;
	
	// État courant
	TWeakObjectPtr<AActor> CurrentBase; // null => pas stacké
	int32 CurrentOrder = -1;            // 0…n-1 pour les enfants (ordre dans le stack)
};

USTRUCT(BlueprintType)
struct FStartAndEnd
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;
	UPROPERTY()
	FVector EndLocation = FVector::ZeroVector;
	
	FStartAndEnd() = default;
	FStartAndEnd(FVector NewStartLocation, FVector NewEndLocation):
		StartLocation(NewStartLocation),
		EndLocation(NewEndLocation){}
};

// TextWidget
USTRUCT(BlueprintType)
struct FTrailLabelStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere)
	FVector2D DrawSize = FVector2D(256.f, 64.f);
};

// Infos des courses chargées
USTRUCT(BlueprintType)
struct FRaceLoaded
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int64 RaceID;
	UPROPERTY(EditAnywhere)
	FString RaceName;
	UPROPERTY(EditAnywhere)
	FString RaceTemplate;
	
};