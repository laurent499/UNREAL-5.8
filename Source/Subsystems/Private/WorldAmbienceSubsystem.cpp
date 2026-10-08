// Copyright LTV Prod 2026. All Rights Reserved

#include "WorldAmbienceSubsystem.h"
#include "CesiumGeoreference.h"
#include "Components/DirectionalLightComponent.h"
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
	if ((!SunLight.IsValid() || !WeatherActor.IsValid()) && FindAccumulator <= 0.f)
	{
		FindAccumulator = 2.f;
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

	// Ligne de neige : forcee par la regie, sinon valeur par defaut en attendant le calcul automatique (lot 1)
	State.SnowLineM = Settings.SnowLineOverrideM >= 0.f ? Settings.SnowLineOverrideM : 3000.f;

	PushToMPC();
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
	if (!Settings.bPerfGuard)
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

	TSharedRef<FJsonObject> Effective = MakeShared<FJsonObject>();
	Effective->SetNumberField(TEXT("flora"), GetEffectiveFloraDensity());
	Effective->SetNumberField(TEXT("fauna"), GetEffectiveFaunaDensity());
	Effective->SetBoolField(TEXT("lowClouds"), AreLowCloudsAllowed());
	Effective->SetBoolField(TEXT("mpc"), WorldMPC != nullptr);
	Effective->SetBoolField(TEXT("sun"), SunLight.IsValid());
	Effective->SetBoolField(TEXT("weather"), WeatherActor.IsValid());
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
		for (FHttpRouteHandle* Handle : { &StateRouteHandle, &SetRouteHandle, &BenchStartRouteHandle, &BenchStateRouteHandle })
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
		// Pose au sol aux deux tiers de l'attente : les tuiles proches ont eu le temps d'arriver avec leur collision
		if (Shot.AboveGroundM >= 0.0 && !bBenchGrounded && BenchTimer >= BenchSettleSeconds * 0.66f)
		{
			PlaceCameraOnGround(Shot);
			bBenchGrounded = true;
		}
		if (BenchTimer >= BenchSettleSeconds)
		{
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
