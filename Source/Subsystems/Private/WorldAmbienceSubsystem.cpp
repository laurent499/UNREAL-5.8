// Copyright LTV Prod 2026. All Rights Reserved

#include "WorldAmbienceSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "CheckpointSubsystem.h"
#include "Engine/GameInstance.h"
#include "RaceSubsystem.h"
#include "WeatherSubsystem.h"
#include "Cesium3DTileset.h"
#include "CesiumUrlTemplateRasterOverlay.h"
#include "CesiumGeoreference.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HttpPath.h"
#include "HttpServerModule.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "IHttpRouter.h"
#include "JsonObjectConverter.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RenderTimer.h"
#include "DynamicRHI.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogWorldAmbience, Log, All);

namespace WorldAmbience
{
	const TCHAR* MPCPath = TEXT("/Game/LTVContent/Materials/MPC_World.MPC_World");
	const TCHAR* WeatherClassToken = TEXT("Ultra_Dynamic_Weather");
	const TCHAR* SkyClassToken = TEXT("Ultra_Dynamic_Sky");
	const TCHAR* NightLightsKey = TEXT("NightLights");

	FString SettingsFilePath()
	{
		return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("WorldAmbience.json"));
	}

	/** Lit une variable numerique d'un Blueprint par son nom (ex. "Cloud Coverage") */
	bool ReadNumber(const UObject* Object, const TCHAR* Name, float& Out)
	{
		const FNumericProperty* Prop = CastField<FNumericProperty>(Object->GetClass()->FindPropertyByName(FName(Name)));
		if (!Prop) return false;
		const void* Ptr = Prop->ContainerPtrToValuePtr<void>(Object);
		Out = Prop->IsFloatingPoint() ? static_cast<float>(Prop->GetFloatingPointPropertyValue(Ptr))
		                              : static_cast<float>(Prop->GetSignedIntPropertyValue(Ptr));
		return true;
	}

	/** Lecture / ecriture texte d'une variable de Blueprint par son nom, pour sauvegarder puis restaurer UDS et UDW */
	FString ExportProperty(UObject* Object, const FString& Name)
	{
		const FProperty* Prop = Object->GetClass()->FindPropertyByName(FName(*Name));
		FString Out;
		if (Prop) Prop->ExportTextItem_Direct(Out, Prop->ContainerPtrToValuePtr<void>(Object), nullptr, Object, PPF_None);
		return Out;
	}

	bool ImportProperty(UObject* Object, const FString& Name, const FString& Value)
	{
		const FProperty* Prop = Object->GetClass()->FindPropertyByName(FName(*Name));
		return Prop && Prop->ImportText_Direct(*Value, Prop->ContainerPtrToValuePtr<void>(Object), Object, PPF_None) != nullptr;
	}

	AActor* FindActorByClassToken(UWorld* World, const TCHAR* Token)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->GetClass()->GetName().Contains(Token)) return *It;
		}
		return nullptr;
	}

	float Percentile(TArray<float> Values, float P)
	{
		if (Values.Num() == 0) return 0.f;
		Values.Sort();
		const int32 Index = FMath::Clamp(FMath::CeilToInt(P * Values.Num()) - 1, 0, Values.Num() - 1);
		return Values[Index];
	}

	float Average(const TArray<float>& Values)
	{
		if (Values.Num() == 0) return 0.f;
		double Sum = 0.0;
		for (float V : Values) Sum += V;
		return static_cast<float>(Sum / Values.Num());
	}

	/** Interpolation lineaire par morceaux sur des points (x croissants) */
	float Piecewise(float X, std::initializer_list<FVector2f> Points)
	{
		const FVector2f* Prev = nullptr;
		for (const FVector2f& P : Points)
		{
			if (X <= P.X) return Prev ? FMath::GetMappedRangeValueClamped(FVector2f(Prev->X, P.X), FVector2f(Prev->Y, P.Y), X) : P.Y;
			Prev = &P;
		}
		return Prev ? Prev->Y : 0.f;
	}

	/** Cles des parametres UDW pilotes par la meteo reelle et la regie */
	const TCHAR* WeatherKeys[] = { TEXT("Cloud Coverage"), TEXT("Rain"), TEXT("Snow"), TEXT("Fog"), TEXT("Thunder/Lightning"), TEXT("Wind Intensity") };

	TUniquePtr<FHttpServerResponse> JsonResponse(const FString& Json)
	{
		TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(Json, TEXT("application/json"));
		Response->Headers.Add(TEXT("Access-Control-Allow-Origin"), { TEXT("*") });
		return Response;
	}
}

bool UWorldAmbienceSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UWorldAmbienceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettings();
	BindRoutes();
}

void UWorldAmbienceSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UCheckpointSubsystem* Checkpoints = GI->GetSubsystem<UCheckpointSubsystem>())
		{
			Checkpoints->OnCheckpointsDatasGathered.RemoveDynamic(this, &UWorldAmbienceSubsystem::HandleCheckpointsGathered);
		}
	}
	UnbindRoutes();
	Super::Deinitialize();
}

void UWorldAmbienceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	WorldMPC = LoadObject<UMaterialParameterCollection>(nullptr, WorldAmbience::MPCPath);
	if (!WorldMPC)
	{
		UE_LOG(LogWorldAmbience, Warning, TEXT("%s introuvable : les materiaux ne recevront pas l'etat du monde"), WorldAmbience::MPCPath);
	}
	if (UGameInstance* GI = InWorld.GetGameInstance())
	{
		if (UCheckpointSubsystem* Checkpoints = GI->GetSubsystem<UCheckpointSubsystem>())
		{
			Checkpoints->OnCheckpointsDatasGathered.AddUniqueDynamic(this, &UWorldAmbienceSubsystem::HandleCheckpointsGathered);
		}
	}

	bBegunPlay = true;
	UpdateState(0.f);
}

TStatId UWorldAmbienceSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWorldAmbienceSubsystem, STATGROUP_Tickables);
}

void UWorldAmbienceSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bBegunPlay) return;

	TimeSinceBeginPlay += DeltaTime;

	// Le gardien suit chaque image ; l'etat du monde n'a pas besoin de plus de 4 mises a jour par seconde
	UpdatePerfGuard(DeltaTime);
	TickBench(DeltaTime);

	UpdateAccumulator += DeltaTime;
	if (UpdateAccumulator >= UpdateInterval)
	{
		UpdateState(UpdateAccumulator);
		UpdateAccumulator = 0.f;
	}
}

// ---------------------------------------------------------------------------
// Etat du monde
// ---------------------------------------------------------------------------

void UWorldAmbienceSubsystem::UpdateState(float DeltaTime)
{
	// Les acteurs UDS/UDW peuvent arriver apres le BeginPlay (streaming, changement de map) : on les recherche de temps en temps
	FindAccumulator -= DeltaTime;
	if ((!SunLight.IsValid() || !WeatherActor.IsValid() || !bNightLightsAdded) && FindAccumulator <= 0.f)
	{
		FindAccumulator = 2.f;
		EnsureNightLightsOverlay();
		FindSunLight();
		if (!WeatherActor.IsValid())
		{
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				if (It->GetClass()->GetName().Contains(WorldAmbience::WeatherClassToken))
				{
					WeatherActor = *It;
					break;
				}
			}
		}
	}

	if (const UDirectionalLightComponent* Sun = SunLight.Get())
	{
		// Pres de l'origine Cesium, Z est la verticale locale
		const FVector LightDir = Sun->GetComponentTransform().GetUnitAxis(EAxis::X);
		State.SunElevationDeg = FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(-LightDir.Z, -1.0, 1.0)));
		// Plein jour au-dessus de +6 deg, nuit noire sous -8 deg (fin du crepuscule civil vers -6)
		State.Night = 1.f - FMath::SmoothStep(-8.f, 6.f, State.SunElevationDeg);
	}

	TickAutoWeather(DeltaTime);
	UpdateCloudBase(DeltaTime);
	ReadWeatherActor();

	// Sol mouille : monte en 2 a 6 minutes selon l'intensite, seche en DryingMinutes
	const float Precip = FMath::Max(State.Rain, State.Snow);
	if (Precip > 0.5f && DeltaTime > 0.f)
	{
		const float WetSeconds = 360.f / FMath::Clamp(Precip / 3.f, 1.f, 3.f);
		State.Wetness = FMath::Min(1.f, State.Wetness + DeltaTime / WetSeconds);
	}
	else if (DeltaTime > 0.f)
	{
		State.Wetness = FMath::Max(0.f, State.Wetness - DeltaTime / (FMath::Max(Settings.DryingMinutes, 0.1f) * 60.f));
	}

	// Ligne de neige : forcee par la regie, sinon isotherme 0 degre deduit du releve reel moins 300 m
	// (la neige tient un peu sous l'isotherme), sinon 3000 m par defaut
	if (Settings.SnowLineOverrideM >= 0.f)
	{
		State.SnowLineM = Settings.SnowLineOverrideM;
	}
	else if (AutoWeather.bHasData)
	{
		const float FreezingLevelM = AutoWeather.SourceAltitudeM + AutoWeather.TempC / 0.0065f;
		State.SnowLineM = FMath::Clamp(FreezingLevelM - 300.f, 0.f, 6000.f);
	}
	else
	{
		State.SnowLineM = 3000.f;
	}

	PushToMPC();
}

void UWorldAmbienceSubsystem::EnsureNightLightsOverlay()
{
	// Lumieres des villes : NASA Black Marble (VIIRS 2016), lu par le materiau des tuiles sous la cle "NightLights".
	// Ajoute aux tilesets qui portent deja le masque d'eau, c'est-a-dire ceux qui utilisent M_CesiumGlobalWater.
	bool bAny = false;
	for (TActorIterator<ACesium3DTileset> It(GetWorld()); It; ++It)
	{
		TArray<UCesiumRasterOverlay*> Overlays;
		It->GetComponents(Overlays);
		const bool bHasWater = Overlays.ContainsByPredicate([](const UCesiumRasterOverlay* O) { return O->MaterialLayerKey == TEXT("Water"); });
		if (!bHasWater) continue;
		bAny = true;
		if (Overlays.ContainsByPredicate([](const UCesiumRasterOverlay* O) { return O->MaterialLayerKey == WorldAmbience::NightLightsKey; })) continue;

		UCesiumUrlTemplateRasterOverlay* Overlay = NewObject<UCesiumUrlTemplateRasterOverlay>(*It, TEXT("NightLightsOverlay"));
		Overlay->MaterialLayerKey = WorldAmbience::NightLightsKey;
		Overlay->TemplateUrl = TEXT("https://gibs.earthdata.nasa.gov/wmts/epsg3857/best/VIIRS_Black_Marble/default/2016-01-01/GoogleMapsCompatible_Level8/{z}/{reverseY}/{x}.png");
		Overlay->Projection = ECesiumUrlTemplateRasterOverlayProjection::WebMercator;
		Overlay->MaximumLevel = 8;
		It->AddInstanceComponent(Overlay);
		Overlay->RegisterComponent(); // auto-activation : ajoute l'overlay au tileset
		UE_LOG(LogWorldAmbience, Log, TEXT("Overlay NightLights (NASA Black Marble) ajoute a %s"), *It->GetActorNameOrLabel());
	}
	bNightLightsAdded = bAny;
}

UDirectionalLightComponent* UWorldAmbienceSubsystem::FindSunLight()
{
	// Le soleil est la lumiere directionnelle d'atmosphere d'indice 0, de preference celle d'Ultra Dynamic Sky
	UDirectionalLightComponent* Best = nullptr;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		TInlineComponentArray<UDirectionalLightComponent*> Lights(*It);
		for (UDirectionalLightComponent* Light : Lights)
		{
			if (!Light->IsUsedAsAtmosphereSunLight() || Light->GetAtmosphereSunLightIndex() != 0) continue;
			Best = Light;
			if (It->GetClass()->GetName().Contains(WorldAmbience::SkyClassToken))
			{
				SunLight = Best;
				return Best;
			}
		}
	}
	SunLight = Best;
	return Best;
}

void UWorldAmbienceSubsystem::ReadWeatherActor()
{
	const AActor* Weather = WeatherActor.Get();
	if (!Weather) return;

	WorldAmbience::ReadNumber(Weather, TEXT("Cloud Coverage"), State.CloudCoverage);
	WorldAmbience::ReadNumber(Weather, TEXT("Rain"), State.Rain);
	WorldAmbience::ReadNumber(Weather, TEXT("Snow"), State.Snow);
	WorldAmbience::ReadNumber(Weather, TEXT("Fog"), State.Fog);
	WorldAmbience::ReadNumber(Weather, TEXT("Wind Intensity"), State.Wind);
}

void UWorldAmbienceSubsystem::PushToMPC() const
{
	if (!WorldMPC) return;
	UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(WorldMPC);
	if (!Instance) return;

	auto Set = [Instance](const TCHAR* Name, float Value) { Instance->SetScalarParameterValue(FName(Name), Value); };
	Set(TEXT("Night"), State.Night);
	Set(TEXT("SunElevation"), State.SunElevationDeg);
	Set(TEXT("CloudCoverage"), State.CloudCoverage);
	Set(TEXT("Rain"), State.Rain);
	Set(TEXT("Snow"), State.Snow);
	Set(TEXT("Fog"), State.Fog);
	Set(TEXT("Wind"), State.Wind);
	Set(TEXT("Wetness"), State.Wetness);
	Set(TEXT("SnowLine"), State.SnowLineM * 100.f); // cm, unite du monde
	Set(TEXT("HeadlampIntensity"), Settings.bHeadlamps ? Settings.HeadlampIntensity : 0.f);
	Set(TEXT("CityLightsIntensity"), Settings.bCityLights ? Settings.CityLightsIntensity : 0.f);
	Set(TEXT("FloraDensity"), GetEffectiveFloraDensity());
	Set(TEXT("FaunaDensity"), GetEffectiveFaunaDensity());
	// Altitude de l'origine Cesium (cm) : le materiau des tuiles en deduit l'altitude reelle de chaque pixel
	if (const ACesiumGeoreference* Geo = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(GetWorld(), ACesiumGeoreference::StaticClass())))
	{
		Set(TEXT("OriginHeight"), static_cast<float>(Geo->GetOriginHeight() * 100.0));
	}
}

// ---------------------------------------------------------------------------
// Gardien de performance
// ---------------------------------------------------------------------------

void UWorldAmbienceSubsystem::UpdatePerfGuard(float DeltaTime)
{
	const float GpuMs = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	const float RenderMs = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	const float GameMs = FPlatformTime::ToMilliseconds(GGameThreadTime);
	const float RawMs = FMath::Max3(GpuMs, RenderMs, GameMs);
	LastRawFrameMs = RawMs;
	LastRawGpuMs = GpuMs;

	// Lissage sur environ une demi-seconde pour ignorer une image isolee
	const float Alpha = FMath::Clamp(DeltaTime / 0.5f, 0.f, 1.f);
	State.GpuMs = FMath::Lerp(State.GpuMs, GpuMs, Alpha);
	State.FrameMs = FMath::Lerp(State.FrameMs, RawMs, Alpha);

	const int32 PreviousLevel = State.GuardLevel;
	if (TimeSinceBeginPlay < 15.f)
	{
		// Chargement de la map et des premieres tuiles : images tres longues sans rapport avec le cout du rendu
		GuardHighTime = GuardLowTime = 0.f;
	}
	else if (!Settings.bPerfGuard)
	{
		State.GuardLevel = 0;
		GuardHighTime = GuardLowTime = 0.f;
	}
	else if (State.FrameMs > Settings.GuardHighMs)
	{
		GuardLowTime = 0.f;
		GuardHighTime += DeltaTime;
		if (GuardHighTime >= Settings.GuardHoldSeconds && State.GuardLevel < 4)
		{
			++State.GuardLevel;
			GuardHighTime = 0.f;
		}
	}
	else if (State.FrameMs < Settings.GuardLowMs)
	{
		GuardHighTime = 0.f;
		GuardLowTime += DeltaTime;
		// Retablir lentement pour ne pas osciller entre deux crans
		if (GuardLowTime >= Settings.GuardHoldSeconds * 5.f && State.GuardLevel > 0)
		{
			--State.GuardLevel;
			GuardLowTime = 0.f;
		}
	}
	else
	{
		GuardHighTime = GuardLowTime = 0.f;
	}

	if (State.GuardLevel != PreviousLevel)
	{
		UE_LOG(LogWorldAmbience, Log, TEXT("Gardien de performance : cran %d -> %d (%.1f ms)"), PreviousLevel, State.GuardLevel, State.FrameMs);
		OnAmbienceChanged.Broadcast(Settings);
	}
}

float UWorldAmbienceSubsystem::GetEffectiveFloraDensity() const
{
	if (!Settings.bFlora || State.GuardLevel >= 2) return 0.f;
	return Settings.FloraDensity * (State.GuardLevel >= 1 ? 0.5f : 1.f);
}

float UWorldAmbienceSubsystem::GetEffectiveFaunaDensity() const
{
	return (Settings.bFauna && State.GuardLevel < 3) ? Settings.FaunaDensity : 0.f;
}

bool UWorldAmbienceSubsystem::AreLowCloudsAllowed() const
{
	return Settings.bLowClouds && State.GuardLevel < 4;
}

// ---------------------------------------------------------------------------
// Reglages
// ---------------------------------------------------------------------------

void UWorldAmbienceSubsystem::SetSettings(const FWorldAmbienceSettings& NewSettings)
{
	Settings = NewSettings;
	SaveSettings();
	PushToMPC();
	OnAmbienceChanged.Broadcast(Settings);
}

void UWorldAmbienceSubsystem::LoadSettings()
{
	FString Json;
	if (FFileHelper::LoadFileToString(Json, *WorldAmbience::SettingsFilePath()))
	{
		FJsonObjectConverter::JsonObjectStringToUStruct(Json, &Settings, 0, 0);
	}
}

void UWorldAmbienceSubsystem::SaveSettings() const
{
	FString Json;
	if (FJsonObjectConverter::UStructToJsonObjectString(Settings, Json))
	{
		FFileHelper::SaveStringToFile(Json, *WorldAmbience::SettingsFilePath());
	}
}

bool UWorldAmbienceSubsystem::ApplySetting(const FString& Key, const FString& Value)
{
	// Les noms de FName ne tiennent pas compte de la casse : "faunaDensity" et "FaunaDensity" vont au meme champ
	FProperty* Prop = FWorldAmbienceSettings::StaticStruct()->FindPropertyByName(FName(*Key));
	if (!Prop) return false;

	void* Ptr = Prop->ContainerPtrToValuePtr<void>(&Settings);
	if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
	{
		BoolProp->SetPropertyValue(Ptr, Value == TEXT("1") || Value.Equals(TEXT("true"), ESearchCase::IgnoreCase));
		return true;
	}
	if (const FNumericProperty* NumProp = CastField<FNumericProperty>(Prop))
	{
		if (!Value.IsNumeric()) return false;
		if (NumProp->IsFloatingPoint()) NumProp->SetFloatingPointPropertyValue(Ptr, FCString::Atod(*Value));
		else NumProp->SetIntPropertyValue(Ptr, FCString::Atoi64(*Value));
		return true;
	}
	return false;
}

FString UWorldAmbienceSubsystem::StateJson() const
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetBoolField(TEXT("ok"), true);
	Root->SetObjectField(TEXT("settings"), FJsonObjectConverter::UStructToJsonObject(Settings));
	Root->SetObjectField(TEXT("state"), FJsonObjectConverter::UStructToJsonObject(State));
	Root->SetObjectField(TEXT("weather"), FJsonObjectConverter::UStructToJsonObject(AutoWeather));

	TSharedRef<FJsonObject> Effective = MakeShared<FJsonObject>();
	Effective->SetNumberField(TEXT("flora"), GetEffectiveFloraDensity());
	Effective->SetNumberField(TEXT("fauna"), GetEffectiveFaunaDensity());
	Effective->SetBoolField(TEXT("lowClouds"), AreLowCloudsAllowed());
	Effective->SetBoolField(TEXT("mpc"), WorldMPC != nullptr);
	Effective->SetBoolField(TEXT("sun"), SunLight.IsValid());
	Effective->SetBoolField(TEXT("weather"), WeatherActor.IsValid());
	if (const UVolumetricCloudComponent* Cloud = CloudComponent.Get())
	{
		Effective->SetNumberField(TEXT("cloudBottomKm"), Cloud->LayerBottomAltitude);
	}
	Root->SetObjectField(TEXT("effective"), Effective);

	FString Out;
	FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Out));
	return Out;
}

// ---------------------------------------------------------------------------
// Routes regie : /monde/state, /monde/set?cle=valeur&...
// ---------------------------------------------------------------------------

void UWorldAmbienceSubsystem::BindRoutes()
{
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	HttpRouter = HttpServerModule.GetHttpRouter(ListenPort, /*bFailOnBindFailure=*/true);
	if (!HttpRouter.IsValid())
	{
		UE_LOG(LogWorldAmbience, Warning, TEXT("Port %d indisponible : routes /monde non servies"), ListenPort);
		return;
	}

	StateRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/monde/state")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UWorldAmbienceSubsystem::HandleStateRequest));
	SetRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/monde/set")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UWorldAmbienceSubsystem::HandleSetRequest));

	ForceRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/monde/force")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UWorldAmbienceSubsystem::HandleForceRequest));
	ReleaseRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/monde/release")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UWorldAmbienceSubsystem::HandleReleaseRequest));
	BenchStartRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/monde/bench/start")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UWorldAmbienceSubsystem::HandleBenchStartRequest));
	BenchStateRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/monde/bench")), EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &UWorldAmbienceSubsystem::HandleBenchStateRequest));

	if (!StateRouteHandle.IsValid() || !SetRouteHandle.IsValid())
	{
		UE_LOG(LogWorldAmbience, Warning, TEXT("Routes /monde deja prises (un autre monde de jeu est ouvert ?)"));
	}
	HttpServerModule.StartAllListeners();
}

void UWorldAmbienceSubsystem::UnbindRoutes()
{
	if (HttpRouter.IsValid())
	{
		for (FHttpRouteHandle* Handle : { &StateRouteHandle, &SetRouteHandle, &ForceRouteHandle, &ReleaseRouteHandle, &BenchStartRouteHandle, &BenchStateRouteHandle })
		{
			if (Handle->IsValid()) HttpRouter->UnbindRoute(*Handle);
			Handle->Reset();
		}
	}
	HttpRouter.Reset();
}

bool UWorldAmbienceSubsystem::HandleStateRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	OnComplete(WorldAmbience::JsonResponse(StateJson()));
	return true;
}

bool UWorldAmbienceSubsystem::HandleSetRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	TArray<FString> Unknown;
	for (const TPair<FString, FString>& Param : Request.QueryParams)
	{
		if (!ApplySetting(Param.Key, Param.Value)) Unknown.Add(Param.Key);
	}

	if (Unknown.Num() > 0)
	{
		OnComplete(WorldAmbience::JsonResponse(FString::Printf(
			TEXT("{\"ok\":false,\"error\":\"Reglage inconnu ou valeur invalide : %s\"}"), *FString::Join(Unknown, TEXT(", ")))));
		return true;
	}

	SetSettings(Settings);
	OnComplete(WorldAmbience::JsonResponse(StateJson()));
	return true;
}

// ---------------------------------------------------------------------------
// Banc de test : /monde/bench/start[?shot=N], /monde/bench
// Plans decrits dans RemoteControl/BancTest.json, resultats dans Saved/BancTest/*.csv
// ---------------------------------------------------------------------------

bool UWorldAmbienceSubsystem::HandleBenchStartRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	const FString* ShotParam = Request.QueryParams.Find(TEXT("shot"));
	const int32 OnlyShot = (ShotParam && ShotParam->IsNumeric()) ? FCString::Atoi(**ShotParam) : -1;

	FString Error;
	if (!StartBench(OnlyShot, Error))
	{
		OnComplete(WorldAmbience::JsonResponse(FString::Printf(TEXT("{\"ok\":false,\"error\":\"%s\"}"), *Error.ReplaceCharWithEscapedChar())));
		return true;
	}
	OnComplete(WorldAmbience::JsonResponse(BenchJson()));
	return true;
}

bool UWorldAmbienceSubsystem::HandleBenchStateRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	OnComplete(WorldAmbience::JsonResponse(BenchJson()));
	return true;
}

bool UWorldAmbienceSubsystem::StartBench(int32 OnlyShot, FString& OutError)
{
	if (BenchPhase != EBenchPhase::Idle) { OutError = TEXT("Banc deja en cours"); return false; }

	const FString File = FPaths::Combine(FPaths::ProjectDir(), TEXT("RemoteControl"), TEXT("BancTest.json"));
	FString Json;
	TSharedPtr<FJsonObject> Root;
	if (!FFileHelper::LoadFileToString(Json, *File) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
	{
		OutError = TEXT("RemoteControl/BancTest.json introuvable ou illisible");
		return false;
	}

	Root->TryGetNumberField(TEXT("settleSeconds"), BenchSettleSeconds);
	Root->TryGetNumberField(TEXT("measureSeconds"), BenchMeasureSeconds);

	BenchShots.Reset();
	const TArray<TSharedPtr<FJsonValue>>* Shots = nullptr;
	if (Root->TryGetArrayField(TEXT("shots"), Shots))
	{
		for (int32 i = 0; i < Shots->Num(); ++i)
		{
			if (OnlyShot >= 0 && i != OnlyShot) continue;
			const TSharedPtr<FJsonObject> S = (*Shots)[i]->AsObject();
			if (!S.IsValid()) continue;
			FBenchShot Shot;
			S->TryGetStringField(TEXT("name"), Shot.Name);
			S->TryGetNumberField(TEXT("lat"), Shot.Lat);
			S->TryGetNumberField(TEXT("lon"), Shot.Lon);
			S->TryGetNumberField(TEXT("height"), Shot.HeightM);
			S->TryGetNumberField(TEXT("aboveGround"), Shot.AboveGroundM);
			S->TryGetNumberField(TEXT("pitch"), Shot.Pitch);
			double Heading = 0.0;
			S->TryGetNumberField(TEXT("heading"), Heading);
			// Repere Cesium : +X est, +Y sud. Cap 0 = nord = lacet -90
			Shot.Yaw = static_cast<float>(Heading) - 90.f;
			S->TryGetNumberField(TEXT("time"), Shot.TimeOfDay);
			const TSharedPtr<FJsonObject>* Weather = nullptr;
			if (S->TryGetObjectField(TEXT("weather"), Weather))
			{
				for (const auto& Pair : (*Weather)->Values) Shot.Weather.Add(FString(Pair.Key),static_cast<float>(Pair.Value->AsNumber()));
			}
			BenchShots.Add(MoveTemp(Shot));
		}
	}
	if (BenchShots.Num() == 0) { OutError = TEXT("Aucun plan dans BancTest.json"); return false; }

	UWorld* World = GetWorld();
	ACesiumGeoreference* Geo = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(World, ACesiumGeoreference::StaticClass()));
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Geo || !Pawn) { OutError = TEXT("Georeference Cesium ou camera du joueur introuvable"); return false; }

	// Sauvegarde de tout ce que le banc va toucher
	SavedGeoOrigin = Geo->GetOriginLongitudeLatitudeHeight();
	SavedPawnTransform = Pawn->GetActorTransform();
	SavedControlRotation = PC->GetControlRotation();
	SavedSkyValues.Reset();
	SavedWeatherValues.Reset();
	if (AActor* Sky = WorldAmbience::FindActorByClassToken(World, WorldAmbience::SkyClassToken))
	{
		for (const TCHAR* Name : { TEXT("TimeOfDay"), TEXT("Use System Time"), TEXT("Animate Time of Day") })
			SavedSkyValues.Add(Name, WorldAmbience::ExportProperty(Sky, Name));
	}
	if (AActor* Weather = WeatherActor.Get())
	{
		for (const FBenchShot& Shot : BenchShots)
		{
			for (const auto& Pair : Shot.Weather)
			{
				const FString Override = Pair.Key + TEXT(" - Manual Override");
				if (!SavedWeatherValues.Contains(Pair.Key)) SavedWeatherValues.Add(Pair.Key, WorldAmbience::ExportProperty(Weather, Pair.Key));
				if (!SavedWeatherValues.Contains(Override)) SavedWeatherValues.Add(Override, WorldAmbience::ExportProperty(Weather, Override));
			}
		}
	}

	// Mesure brute : le gardien ne doit rien couper pendant le banc
	bSavedPerfGuard = Settings.bPerfGuard;
	Settings.bPerfGuard = false;

	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("BancTest"));
	IFileManager::Get().MakeDirectory(*Dir, true);
	BenchCsvPath = FPaths::Combine(Dir, FString::Printf(TEXT("banc_%s.csv"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
	FFileHelper::SaveStringToFile(TEXT("plan;image_moy_ms;image_p99_ms;gpu_moy_ms;gpu_p99_ms;echantillons\n"), *BenchCsvPath);

	BenchResults.Reset();
	BenchIndex = 0;
	ApplyBenchShot(BenchShots[0]);
	UE_LOG(LogWorldAmbience, Log, TEXT("Banc de test : %d plan(s), resultats dans %s"), BenchShots.Num(), *BenchCsvPath);
	return true;
}

void UWorldAmbienceSubsystem::ApplyBenchShot(const FBenchShot& Shot)
{
	UWorld* World = GetWorld();
	if (ACesiumGeoreference* Geo = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(World, ACesiumGeoreference::StaticClass())))
	{
		// En mode "au sol", l'origine est a une altitude provisoire ; la camera est posee par lancer de rayon une fois les tuiles chargees
		Geo->SetOriginLongitudeLatitudeHeight(FVector(Shot.Lon, Shot.Lat, Shot.AboveGroundM >= 0.0 ? 2000.0 : Shot.HeightM));
	}

	const FRotator Rotation(Shot.Pitch, Shot.Yaw, 0.f);
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		PC->SetControlRotation(Rotation);
		if (APawn* Pawn = PC->GetPawn()) Pawn->SetActorLocationAndRotation(FVector::ZeroVector, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (AActor* Sky = WorldAmbience::FindActorByClassToken(World, WorldAmbience::SkyClassToken))
	{
		WorldAmbience::ImportProperty(Sky, TEXT("Use System Time"), TEXT("False"));
		WorldAmbience::ImportProperty(Sky, TEXT("Animate Time of Day"), TEXT("False"));
		WorldAmbience::ImportProperty(Sky, TEXT("TimeOfDay"), FString::SanitizeFloat(Shot.TimeOfDay));
	}
	if (AActor* Weather = WeatherActor.Get())
	{
		for (const auto& Pair : Shot.Weather)
		{
			WorldAmbience::ImportProperty(Weather, Pair.Key + TEXT(" - Manual Override"), TEXT("True"));
			WorldAmbience::ImportProperty(Weather, Pair.Key, FString::SanitizeFloat(Pair.Value));
		}
	}

	BenchPhase = EBenchPhase::Settling;
	BenchTimer = 0.f;
	bBenchGrounded = false;
	BenchLoadedTime = 0.f;
	BenchFrameSamples.Reset();
	BenchGpuSamples.Reset();
}

void UWorldAmbienceSubsystem::PlaceCameraOnGround(const FBenchShot& Shot)
{
	UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn) return;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WorldAmbienceBench), false, Pawn);
	const FVector Start(0.0, 0.0, 800000.0);
	const FVector End(0.0, 0.0, -800000.0);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		Pawn->SetActorLocation(Hit.ImpactPoint + FVector(0.0, 0.0, Shot.AboveGroundM * 100.0), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		UE_LOG(LogWorldAmbience, Warning, TEXT("Banc : sol introuvable sous le plan \"%s\" (tuiles sans collision ?)"), *Shot.Name);
	}
}

void UWorldAmbienceSubsystem::TickBench(float DeltaTime)
{
	if (BenchPhase == EBenchPhase::Idle || !BenchShots.IsValidIndex(BenchIndex)) return;
	const FBenchShot& Shot = BenchShots[BenchIndex];
	BenchTimer += DeltaTime;

	if (BenchPhase == EBenchPhase::Settling)
	{
		// On attend que toutes les tuiles Cesium soient chargees et le restent 3 s (BenchSettleSeconds = attente maximale)
		float Progress = 100.f;
		for (TActorIterator<ACesium3DTileset> It(GetWorld()); It; ++It) Progress = FMath::Min(Progress, It->GetLoadProgress());
		BenchLoadedTime = (Progress >= 99.5f && BenchTimer >= 3.f) ? BenchLoadedTime + DeltaTime : 0.f;
		const bool bTimeout = BenchTimer >= BenchSettleSeconds;

		// Plan au sol : la camera est posee une fois le relief charge (collision comprise), puis on attend le detail proche
		if (Shot.AboveGroundM >= 0.0 && !bBenchGrounded)
		{
			if (BenchLoadedTime >= 1.f || BenchTimer >= BenchSettleSeconds * 0.5f)
			{
				PlaceCameraOnGround(Shot);
				bBenchGrounded = true;
				BenchLoadedTime = 0.f;
			}
			return;
		}
		if (BenchLoadedTime >= 3.f || bTimeout)
		{
			if (bTimeout) UE_LOG(LogWorldAmbience, Warning, TEXT("Banc \"%s\" : tuiles chargees a %.0f %% seulement"), *Shot.Name, Progress);
			BenchPhase = EBenchPhase::Measuring;
			BenchTimer = 0.f;
		}
		return;
	}

	BenchFrameSamples.Add(LastRawFrameMs);
	BenchGpuSamples.Add(LastRawGpuMs);
	if (BenchTimer >= BenchMeasureSeconds) FinishBenchShot();
}

void UWorldAmbienceSubsystem::FinishBenchShot()
{
	FBenchResult Result;
	Result.Name = BenchShots[BenchIndex].Name;
	Result.FrameAvg = WorldAmbience::Average(BenchFrameSamples);
	Result.FrameP99 = WorldAmbience::Percentile(BenchFrameSamples, 0.99f);
	Result.GpuAvg = WorldAmbience::Average(BenchGpuSamples);
	Result.GpuP99 = WorldAmbience::Percentile(BenchGpuSamples, 0.99f);
	Result.Samples = BenchFrameSamples.Num();
	BenchResults.Add(Result);

	const FString Line = FString::Printf(TEXT("%s;%.2f;%.2f;%.2f;%.2f;%d\n"),
		*Result.Name, Result.FrameAvg, Result.FrameP99, Result.GpuAvg, Result.GpuP99, Result.Samples);
	FFileHelper::SaveStringToFile(Line, *BenchCsvPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	UE_LOG(LogWorldAmbience, Log, TEXT("Banc \"%s\" : image %.1f ms (p99 %.1f), GPU %.1f ms (p99 %.1f)"),
		*Result.Name, Result.FrameAvg, Result.FrameP99, Result.GpuAvg, Result.GpuP99);

	if (++BenchIndex < BenchShots.Num()) ApplyBenchShot(BenchShots[BenchIndex]);
	else EndBench();
}

void UWorldAmbienceSubsystem::EndBench()
{
	BenchPhase = EBenchPhase::Idle;
	UWorld* World = GetWorld();

	if (ACesiumGeoreference* Geo = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(World, ACesiumGeoreference::StaticClass())))
		Geo->SetOriginLongitudeLatitudeHeight(SavedGeoOrigin);
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		PC->SetControlRotation(SavedControlRotation);
		if (APawn* Pawn = PC->GetPawn()) Pawn->SetActorTransform(SavedPawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}
	if (AActor* Sky = WorldAmbience::FindActorByClassToken(World, WorldAmbience::SkyClassToken))
	{
		for (const auto& Pair : SavedSkyValues) WorldAmbience::ImportProperty(Sky, Pair.Key, Pair.Value);
	}
	if (AActor* Weather = WeatherActor.Get())
	{
		for (const auto& Pair : SavedWeatherValues) WorldAmbience::ImportProperty(Weather, Pair.Key, Pair.Value);
	}
	Settings.bPerfGuard = bSavedPerfGuard;
	UE_LOG(LogWorldAmbience, Log, TEXT("Banc de test termine, scene restauree"));
}

FString UWorldAmbienceSubsystem::BenchJson() const
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetBoolField(TEXT("ok"), true);
	Root->SetBoolField(TEXT("running"), BenchPhase != EBenchPhase::Idle);
	Root->SetStringField(TEXT("phase"), BenchPhase == EBenchPhase::Settling ? TEXT("chargement") : BenchPhase == EBenchPhase::Measuring ? TEXT("mesure") : TEXT("arret"));
	Root->SetNumberField(TEXT("shot"), BenchIndex);
	Root->SetNumberField(TEXT("shots"), BenchShots.Num());
	Root->SetStringField(TEXT("csv"), BenchCsvPath);

	TArray<TSharedPtr<FJsonValue>> Results;
	for (const FBenchResult& R : BenchResults)
	{
		TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
		O->SetStringField(TEXT("name"), R.Name);
		O->SetNumberField(TEXT("frameAvg"), R.FrameAvg);
		O->SetNumberField(TEXT("frameP99"), R.FrameP99);
		O->SetNumberField(TEXT("gpuAvg"), R.GpuAvg);
		O->SetNumberField(TEXT("gpuP99"), R.GpuP99);
		O->SetNumberField(TEXT("samples"), R.Samples);
		Results.Add(MakeShared<FJsonValueObject>(O));
	}
	Root->SetArrayField(TEXT("results"), Results);

	FString Out;
	FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Out));
	return Out;
}

// ---------------------------------------------------------------------------
// Meteo reelle -> UDW
// Les releves OpenWeather des checkpoints de la course sont relus toutes les AutoWeatherRefreshMinutes ;
// celui du checkpoint le plus proche de la camera donne les cibles UDW, corrigees de l'altitude.
// Les parametres forces depuis la regie (/monde/force) restent prioritaires.
// ---------------------------------------------------------------------------

void UWorldAmbienceSubsystem::TickAutoWeather(float DeltaTime)
{
	AutoWeather.Sources = WeatherSamples.Num();
	// Le banc de test impose sa propre meteo
	if (BenchPhase != EBenchPhase::Idle) return;
	if (!Settings.bAutoWeather)
	{
		bWasAutoWeather = false;
		return;
	}

	if (!bWasAutoWeather)
	{
		// Activation : on part des valeurs UDW courantes pour que la transition soit douce
		bWasAutoWeather = true;
		WeatherRefreshTimer = 0.f;
		AppliedWeather.Reset();
		if (const AActor* Weather = WeatherActor.Get())
		{
			for (const TCHAR* Key : WorldAmbience::WeatherKeys)
			{
				float Value = 0.f;
				if (WorldAmbience::ReadNumber(Weather, Key, Value)) AppliedWeather.Add(Key, Value);
			}
		}
	}

	WeatherRefreshTimer -= DeltaTime;
	if (WeatherRefreshTimer <= 0.f)
	{
		// Tant qu'aucune course n'est chargee, on reessaie toutes les 30 s
		WeatherRefreshTimer = RequestAutoWeather() ? FMath::Max(Settings.AutoWeatherRefreshMinutes, 1.f) * 60.f : 30.f;
	}
	if (WeatherSamples.Num() == 0) return;

	ComputeAutoWeather();

	// Pleine echelle (0 -> 10) parcourue en WeatherSmoothMinutes ; un forcage regie s'applique tout de suite
	const float Rate = 10.f / (FMath::Max(Settings.WeatherSmoothMinutes, 0.05f) * 60.f);
	for (const TCHAR* Key : WorldAmbience::WeatherKeys)
	{
		const float* Forced = AutoWeather.Forced.Find(Key);
		const float* Target = Forced ? Forced : AutoWeather.Targets.Find(Key);
		if (!Target) continue;
		float& Current = AppliedWeather.FindOrAdd(Key, *Target);
		Current = Forced ? *Forced : FMath::FInterpConstantTo(Current, *Target, DeltaTime, Rate);
		WriteWeatherParam(Key, Current);
	}

	// Direction du vent : meteo = d'ou il vient (0 = nord) ; repere Cesium +X est, +Y sud.
	// Lacet de la direction vers laquelle il souffle = cap + 180 - 90
	if (AActor* Weather = WeatherActor.Get())
	{
		WorldAmbience::ImportProperty(Weather, TEXT("Wind Direction"), FString::SanitizeFloat(FMath::Fmod(AutoWeather.WindDeg + 90.f, 360.f)));
	}
}

void UWorldAmbienceSubsystem::HandleCheckpointsGathered(int64 RaceID, FCheckpoints AllCheckpoints)
{
	KnownCheckpoints.Add(RaceID, MoveTemp(AllCheckpoints));
}

bool UWorldAmbienceSubsystem::RequestAutoWeather()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	URaceSubsystem* Race = GI ? GI->GetSubsystem<URaceSubsystem>() : nullptr;
	UCheckpointSubsystem* Checkpoints = GI ? GI->GetSubsystem<UCheckpointSubsystem>() : nullptr;
	UWeatherSubsystem* Weather = GI ? GI->GetSubsystem<UWeatherSubsystem>() : nullptr;
	if (!Weather) return false;

	// Checkpoints de toutes les courses vues passer, plus ceux de la course courante (chargee avant nous)
	TArray<const FRaceCheckpoint*> All;
	for (const TPair<int64, FCheckpoints>& Pair : KnownCheckpoints)
		for (const FRaceCheckpoint& C : Pair.Value.Checkpoints) All.Add(&C);
	if (const FCheckpoints* Current = (Race && Checkpoints) ? Checkpoints->GetCheckpointsByRaceId(Race->GetCurrentRaceId()) : nullptr)
		for (const FRaceCheckpoint& C : Current->Checkpoints) All.Add(&C);

	TSet<FString> Seen;
	TWeakObjectPtr<UWorldAmbienceSubsystem> WeakThis(this);
	for (const FRaceCheckpoint* CheckpointPtr : All)
	{
		const FRaceCheckpoint& Checkpoint = *CheckpointPtr;
		if (Checkpoint.weather.IsEmpty() || Seen.Contains(Checkpoint.weather)) continue;
		Seen.Add(Checkpoint.weather);

		Weather->PerformHttpRequestForWeather(
			FString::Printf(TEXT("WorldAmbience_%lld"), Checkpoint.checkpointId),
			Checkpoint.weather,
			[WeakThis, Name = Checkpoint.name, AltitudeM = Checkpoint.altitude](FWeatherResult&& Result)
			{
				if (!WeakThis.IsValid() || !Result.bSuccess) return;
				FWeatherSample* Sample = WeakThis->WeatherSamples.FindByPredicate([&Name](const FWeatherSample& S) { return S.Name == Name; });
				if (!Sample) Sample = &WeakThis->WeatherSamples.AddDefaulted_GetRef();
				Sample->Name = Name;
				Sample->Lat = Result.Data.lat;
				Sample->Lon = Result.Data.lon;
				Sample->ElevationM = AltitudeM;
				Sample->Current = Result.Data.current;
				Sample->Description = Result.Data.current.weather.Num() > 0 ? Result.Data.current.weather[0].description : FString();
				Sample->Received = FDateTime::UtcNow();
			});
	}

	if (Seen.Num() == 0)
	{
		AutoWeather.Source = FString::Printf(TEXT("Aucun checkpoint avec meteo (%d checkpoints connus) : course pas encore chargee ?"), All.Num());
		return false;
	}
	return true;
}

bool UWorldAmbienceSubsystem::GetCameraLongLatHeight(FVector& OutLLH) const
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	const ACesiumGeoreference* Geo = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(GetWorld(), ACesiumGeoreference::StaticClass()));
	if (!PC || !PC->PlayerCameraManager || !Geo) return false;
	OutLLH = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(PC->PlayerCameraManager->GetCameraLocation());
	return true;
}

void UWorldAmbienceSubsystem::ComputeAutoWeather()
{
	using WorldAmbience::Piecewise;

	FVector Camera(0.0);
	const bool bHasCamera = GetCameraLongLatHeight(Camera);

	// Checkpoint le plus proche de la camera (distance equirectangulaire, suffisante a l'echelle d'une course)
	const FWeatherSample* Best = &WeatherSamples[0];
	double BestKm = 0.0;
	if (bHasCamera)
	{
		BestKm = TNumericLimits<double>::Max();
		for (const FWeatherSample& S : WeatherSamples)
		{
			const double X = FMath::DegreesToRadians(S.Lon - Camera.X) * FMath::Cos(FMath::DegreesToRadians((S.Lat + Camera.Y) * 0.5));
			const double Y = FMath::DegreesToRadians(S.Lat - Camera.Y);
			const double Km = 6371.0 * FMath::Sqrt(X * X + Y * Y);
			if (Km < BestKm) { BestKm = Km; Best = &S; }
		}
	}

	FAutoWeatherInfo& W = AutoWeather;
	W.SourceLat = Best->Lat;
	W.SourceLon = Best->Lon;
	// Releve trop loin de la camera (autre course, terrain deplace) : on laisse UDW tel quel
	if (bHasCamera && BestKm > MaxSourceDistanceKm)
	{
		W.bHasData = false;
		W.Targets.Reset();
		W.Source = FString::Printf(TEXT("Aucun checkpoint a moins de %.0f km de la camera (le plus proche : %s, %.0f km)"), MaxSourceDistanceKm, *Best->Name, BestKm);
		return;
	}

	const FOpenWeatherCurrent& C = Best->Current;
	W.bHasData = true;
	W.Source = Best->Name;
	W.DistanceKm = static_cast<float>(BestKm);
	W.AgeMinutes = static_cast<int32>((FDateTime::UtcNow() - Best->Received).GetTotalMinutes());
	W.Description = Best->Description;
	W.SourceAltitudeM = Best->ElevationM;
	W.TempC = C.temp;
	W.CameraAltitudeM = bHasCamera ? static_cast<float>(Camera.Z) : Best->ElevationM;
	// Temperature la ou tombent les precipitations visibles : a la camera, mais pas plus de 1000 m au-dessus
	// du releve (en plan aerien, la pluie qu'on voit tombe sur le relief, pas a l'altitude de l'avion)
	const float PrecipAltitudeM = FMath::Min(W.CameraAltitudeM, Best->ElevationM + 1000.f);
	W.CameraTempC = C.temp - 0.0065f * (PrecipAltitudeM - Best->ElevationM);
	W.DewPointC = C.dew_point;
	W.CloudBaseM = Best->ElevationM + 125.f * FMath::Max(0.f, C.temp - C.dew_point);
	W.Clouds = C.clouds;
	W.Rain1h = C.rain_1h;
	W.Snow1h = C.snow_1h;
	W.Visibility = C.visibility;
	W.Humidity = C.humidity;
	W.WindMs = C.wind_speed;
	W.WindDeg = C.wind_deg;

	const int32 Id = C.weather.Num() > 0 ? C.weather[0].id : 800;
	const int32 Group = Id / 100;

	// Precipitations : cumul horaire en priorite, code meteo sinon
	const std::initializer_list<FVector2f> PrecipCurve = { {0.f, 0.f}, {0.5f, 2.f}, {2.f, 5.f}, {8.f, 9.f}, {20.f, 10.f} };
	float Rain = Piecewise(C.rain_1h, PrecipCurve);
	float Snow = Piecewise(C.snow_1h, PrecipCurve);
	if (Rain <= 0.f && Snow <= 0.f)
	{
		const int32 Sub = Id % 100;
		if (Group == 3) Rain = 1.5f;                                                           // bruine
		else if (Group == 5) Rain = Sub == 0 ? 2.f : Sub == 1 ? 4.f : Sub == 2 ? 7.f : Sub <= 4 ? 9.f : 4.f;
		else if (Group == 6) Snow = Sub == 0 ? 2.f : Sub == 1 ? 4.f : Sub == 2 ? 7.f : 3.f;
		else if (Group == 2) Rain = 5.f;
	}

	// Pluie ou neige selon la temperature a l'altitude de la camera
	const float T = W.CameraTempC;
	if (T <= 0.5f) { Snow += Rain; Rain = 0.f; }
	else if (T < 2.f) { const float Half = Rain * 0.5f; Snow += Half; Rain -= Half; }
	else if (T >= 3.f) { Rain += Snow; Snow = 0.f; }

	// Orage
	float Thunder = 0.f;
	if (Group == 2)
	{
		Thunder = (Id == 202 || Id == 232 || Id == 212) ? 9.f : (Id == 211 || Id == 201 || Id == 231) ? 7.f : 5.f;
	}

	// Brouillard : visibilite, codes brume/brouillard, air sature sans vent
	float Fog = C.visibility > 0 ? Piecewise(static_cast<float>(C.visibility), { {200.f, 9.f}, {1000.f, 6.f}, {5000.f, 2.f}, {10000.f, 0.f} }) : 0.f;
	if (Id == 741) Fog = FMath::Max(Fog, 7.f);
	else if (Id == 701 || Id == 721) Fog = FMath::Max(Fog, 3.f);
	if (C.humidity >= 95 && C.wind_speed < 2.f) Fog = FMath::Max(Fog, 2.f);

	float Clouds = C.clouds / 10.f;
	if (Rain > 0.f || Snow > 0.f) Clouds = FMath::Max(Clouds, 7.f);
	if (Thunder > 0.f) Clouds = FMath::Max(Clouds, 9.f);

	W.Targets.Add(TEXT("Cloud Coverage"), Clouds);
	W.Targets.Add(TEXT("Rain"), FMath::Min(Rain, 10.f));
	W.Targets.Add(TEXT("Snow"), FMath::Min(Snow, 10.f));
	W.Targets.Add(TEXT("Fog"), Fog);
	W.Targets.Add(TEXT("Thunder/Lightning"), Thunder);
	W.Targets.Add(TEXT("Wind Intensity"), FMath::Clamp(C.wind_speed * 0.6f, 0.f, 10.f));
}

void UWorldAmbienceSubsystem::UpdateCloudBase(float DeltaTime)
{
	UVolumetricCloudComponent* Cloud = CloudComponent.Get();
	if (!Cloud)
	{
		AActor* Sky = WorldAmbience::FindActorByClassToken(GetWorld(), WorldAmbience::SkyClassToken);
		Cloud = Sky ? Sky->FindComponentByClass<UVolumetricCloudComponent>() : nullptr;
		if (!Cloud) return;
		CloudComponent = Cloud;
		OriginalCloudBottomKm = CurrentCloudBottomKm = Cloud->LayerBottomAltitude;
	}

	// Sans nuages bas (reglage ou gardien) ou sans releve : la base reste celle d'Ultra Dynamic Sky
	float TargetKm = OriginalCloudBottomKm;
	const ACesiumGeoreference* Geo = Cast<ACesiumGeoreference>(UGameplayStatics::GetActorOfClass(GetWorld(), ACesiumGeoreference::StaticClass()));
	if (AreLowCloudsAllowed() && Settings.bAutoWeather && AutoWeather.bHasData && Geo)
	{
		// La planete d'UDS a son sommet a l'origine du monde, donc a l'altitude de l'origine Cesium
		TargetKm = FMath::Clamp((AutoWeather.CloudBaseM - static_cast<float>(Geo->GetOriginHeight())) / 1000.f, 0.2f, OriginalCloudBottomKm);
	}

	// Montee ou descente de 0,5 km par minute au plus
	CurrentCloudBottomKm = FMath::FInterpConstantTo(CurrentCloudBottomKm, TargetKm, DeltaTime, 0.5f / 60.f);
	if (!FMath::IsNearlyEqual(Cloud->LayerBottomAltitude, CurrentCloudBottomKm, 0.005f))
	{
		Cloud->SetLayerBottomAltitude(CurrentCloudBottomKm);
	}
}

void UWorldAmbienceSubsystem::WriteWeatherParam(const FString& Key, float Value)
{
	AActor* Weather = WeatherActor.Get();
	if (!Weather) return;
	WorldAmbience::ImportProperty(Weather, Key + TEXT(" - Manual Override"), TEXT("True"));
	WorldAmbience::ImportProperty(Weather, Key, FString::SanitizeFloat(Value));
}

bool UWorldAmbienceSubsystem::HandleForceRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	// /monde/force?key=Rain&value=6 force un parametre ; sans value, le rend a la meteo reelle
	const FString* Key = Request.QueryParams.Find(TEXT("key"));
	const FString* Value = Request.QueryParams.Find(TEXT("value"));
	bool bKnown = false;
	for (const TCHAR* K : WorldAmbience::WeatherKeys) bKnown |= (Key && *Key == K);
	if (!bKnown || (Value && !Value->IsNumeric()))
	{
		OnComplete(WorldAmbience::JsonResponse(TEXT("{\"ok\":false,\"error\":\"key attendu parmi les parametres UDW, value numerique\"}")));
		return true;
	}

	if (Value)
	{
		const float V = FCString::Atof(**Value);
		AutoWeather.Forced.Add(*Key, V);
		WriteWeatherParam(*Key, V);
	}
	else
	{
		AutoWeather.Forced.Remove(*Key);
	}
	OnComplete(WorldAmbience::JsonResponse(StateJson()));
	return true;
}

bool UWorldAmbienceSubsystem::HandleReleaseRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	AutoWeather.Forced.Reset();
	// Sans meteo auto, on rend la main au prereglage UDS de la map comme avant
	if (!Settings.bAutoWeather)
	{
		if (AActor* Weather = WeatherActor.Get())
		{
			for (const TCHAR* Key : WorldAmbience::WeatherKeys)
				WorldAmbience::ImportProperty(Weather, FString(Key) + TEXT(" - Manual Override"), TEXT("False"));
		}
	}
	OnComplete(WorldAmbience::JsonResponse(StateJson()));
	return true;
}
