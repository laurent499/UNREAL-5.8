// Copyright LTVProd 2026 All Rights Reserved.

#include "StyleSubsystem.h"
#include "SettingsSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogTrailStyle, Log, All);

const FString UStyleSubsystem::RealisticStyleName = TEXT("realiste");

namespace TrailStyle
{
	/** Clés du style qui ne sont pas des sections. */
	static bool IsMetaKey(const FString& Key)
	{
		return Key == TEXT("name") || Key == TEXT("label") || Key == TEXT("description") || Key == TEXT("builtin");
	}

	/** Stencil réservé aux overlays à garder intacts (tracé, bornes, étiquettes). */
	static constexpr int32 OverlayStencil = 200;

	/** Classes (et leurs enfants Blueprint) dont le rendu reste lisible sous un post-process. */
	static const TCHAR* ProtectedClassNames[] = {
		TEXT("Path"), TEXT("Km"), TEXT("Checkpoint"), TEXT("Poi"), TEXT("Runner"),
		TEXT("CinematicTrail"), TEXT("CinematicPanel")
	};

	static bool IsProtectedClass(const UClass* Class)
	{
		for (; Class; Class = Class->GetSuperClass())
		{
			const FString Name = Class->GetName();
			for (const TCHAR* Protected : ProtectedClassNames)
			{
				if (Name == Protected) return true;
			}
		}
		return false;
	}

	static FLinearColor ParseColor(const TSharedPtr<FJsonValue>& Value)
	{
		FString Hex;
		if (Value.IsValid() && Value->TryGetString(Hex))
		{
			return FLinearColor(FColor::FromHex(Hex));
		}
		const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
		if (Value.IsValid() && Value->TryGetArray(Arr) && Arr->Num() >= 3)
		{
			return FLinearColor((*Arr)[0]->AsNumber(), (*Arr)[1]->AsNumber(), (*Arr)[2]->AsNumber(),
				Arr->Num() > 3 ? (*Arr)[3]->AsNumber() : 1.0);
		}
		return FLinearColor::White;
	}
}

void UStyleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentStyleName = RealisticStyleName;

	// Nouveau niveau : le style courant est réappliqué au BeginPlay du monde
	PostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this,
		[this](UWorld* World, const UWorld::InitializationValues)
		{
			if (World && World->IsGameWorld())
			{
				World->OnWorldBeginPlay.AddWeakLambda(this, [this, WeakWorld = TWeakObjectPtr<UWorld>(World)]()
				{
					if (WeakWorld.IsValid()) OnWorldBeginPlay(WeakWorld.Get());
				});
			}
		});
}

void UStyleSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitHandle);
	if (ActiveRequest.IsValid())
	{
		ActiveRequest->OnProcessRequestComplete().Unbind();
		ActiveRequest->CancelRequest();
		ActiveRequest.Reset();
	}
	UWorld* World = GetGameWorld();
	if (World && !World->bIsTearingDown)
	{
		RevertPostProcess(World);
	}
	else if (SavedCustomDepthMode != INDEX_NONE)
	{
		// Monde déjà en démontage : seul le réglage global est à rendre
		if (IConsoleVariable* CustomDepth = IConsoleManager::Get().FindConsoleVariable(TEXT("r.CustomDepth")))
		{
			CustomDepth->Set(SavedCustomDepthMode, ECVF_SetByCode);
		}
		SavedCustomDepthMode = INDEX_NONE;
	}
	if (UWorld* SpawnWorld = SpawnHandleWorld.Get())
	{
		SpawnWorld->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
	}
	Handlers.Empty();
	ActiveSections.Empty();
	Super::Deinitialize();
}

UWorld* UStyleSubsystem::GetGameWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

void UStyleSubsystem::OnWorldBeginPlay(UWorld* World)
{
	if (World != GetGameWorld()) return;
	// L'ancien monde a emporté volume et marquages : on repart de zéro
	StyleVolume = nullptr;
	StyleMaterials.Reset();
	bOverlaysProtected = false;
	ActiveSections.Reset();
	if (CurrentStyle.IsValid())
	{
		ApplySections(World);
	}
}

bool UStyleSubsystem::ApplyStyleJson(const FString& StyleName, const FString& StyleJson)
{
	TSharedPtr<FJsonObject> Style;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(StyleJson);
	if (!FJsonSerializer::Deserialize(Reader, Style) || !Style.IsValid())
	{
		UE_LOG(LogTrailStyle, Warning, TEXT("[Style] %s : JSON illisible, style inchangé (%s)"), *StyleName, *CurrentStyleName);
		return false;
	}
	ApplyParsed(StyleName, Style);
	return true;
}

void UStyleSubsystem::ApplyStyle(const FString& StyleName)
{
	if (StyleName.IsEmpty() || StyleName.Equals(RealisticStyleName, ESearchCase::IgnoreCase))
	{
		ApplyParsed(RealisticStyleName, MakeShared<FJsonObject>());
		return;
	}

	USettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<USettingsSubsystem>() : nullptr;
	if (!Settings || Settings->GetMainURL().IsEmpty())
	{
		UE_LOG(LogTrailStyle, Warning, TEXT("[Style] %s : pas d'URL régie, style inchangé"), *StyleName);
		return;
	}

	if (ActiveRequest.IsValid())
	{
		ActiveRequest->OnProcessRequestComplete().Unbind();
		ActiveRequest->CancelRequest();
	}
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(Settings->GetMainURL() + TEXT("style/") + FGenericPlatformHttp::UrlEncode(StyleName));
	Req->SetVerb(TEXT("GET"));
	Req->SetTimeout(5.f);
	ActiveRequest = Req;
	TWeakObjectPtr<UStyleSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
		[WeakThis, StyleName](FHttpRequestPtr, FHttpResponsePtr Response, bool bOk)
		{
			UStyleSubsystem* Self = WeakThis.Get();
			if (!Self) return;
			Self->ActiveRequest.Reset();
			if (bOk && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
			{
				Self->ApplyStyleJson(StyleName, Response->GetContentAsString());
			}
			else
			{
				UE_LOG(LogTrailStyle, Warning, TEXT("[Style] %s indisponible sur la régie (code %d), style inchangé"),
					*StyleName, Response.IsValid() ? Response->GetResponseCode() : -1);
			}
		});
	if (!Req->ProcessRequest())
	{
		ActiveRequest.Reset();
	}
}

void UStyleSubsystem::ApplyParsed(const FString& StyleName, const TSharedPtr<FJsonObject>& Style)
{
	CurrentStyleName = StyleName;
	CurrentStyle = Style;
	if (UWorld* World = GetGameWorld())
	{
		ApplySections(World);
	}
	UE_LOG(LogTrailStyle, Log, TEXT("[Style] %s appliqué"), *StyleName);
	OnStyleChanged.Broadcast(StyleName);
}

TSharedPtr<FJsonObject> UStyleSubsystem::GetSection(const TSharedPtr<FJsonObject>& Style, const FString& Key)
{
	if (!Style.IsValid()) return nullptr;
	const TSharedPtr<FJsonValue> Value = Style->TryGetField(Key);
	if (!Value.IsValid() || Value->IsNull()) return nullptr;
	if (Value->Type == EJson::Object) return Value->AsObject();
	if (Value->Type == EJson::Array)
	{
		if (Value->AsArray().IsEmpty()) return nullptr;
		TSharedPtr<FJsonObject> Wrapped = MakeShared<FJsonObject>();
		Wrapped->SetField(TEXT("items"), Value);
		return Wrapped;
	}
	return nullptr;
}

void UStyleSubsystem::ApplySections(UWorld* World)
{
	// Sections du nouveau style (hors métadonnées)
	TSet<FName> Wanted;
	if (CurrentStyle.IsValid())
	{
		for (const auto& Pair : CurrentStyle->Values)
		{
			if (!TrailStyle::IsMetaKey(Pair.Key) && GetSection(CurrentStyle, Pair.Key).IsValid())
			{
				Wanted.Add(FName(*Pair.Key));
			}
		}
	}

	// D'abord défaire ce qui n'est plus demandé, puis appliquer
	for (const FName& Active : ActiveSections.Array())
	{
		if (Wanted.Contains(Active)) continue;
		if (Active == TEXT("postprocess")) RevertPostProcess(World);
		else if (const FStyleSectionHandler* Handler = Handlers.Find(Active))
		{
			if (Handler->Revert) Handler->Revert(World);
		}
		ActiveSections.Remove(Active);
	}

	for (const FName& Section : Wanted)
	{
		const TSharedPtr<FJsonObject> Data = GetSection(CurrentStyle, Section.ToString());
		if (Section == TEXT("postprocess"))
		{
			ApplyPostProcess(World, Data.ToSharedRef());
			ActiveSections.Add(Section);
		}
		else if (const FStyleSectionHandler* Handler = Handlers.Find(Section))
		{
			if (Handler->Apply) Handler->Apply(World, Data.ToSharedRef());
			ActiveSections.Add(Section);
		}
		else
		{
			UE_LOG(LogTrailStyle, Warning, TEXT("[Style] %s : section « %s » sans handler, ignorée"),
				*CurrentStyleName, *Section.ToString());
		}
	}
}

void UStyleSubsystem::RegisterSectionHandler(FName Section, FStyleSectionHandler Handler)
{
	if (Section == TEXT("postprocess"))
	{
		UE_LOG(LogTrailStyle, Warning, TEXT("[Style] la section postprocess est gérée par le cadre"));
		return;
	}
	Handlers.Add(Section, MoveTemp(Handler));
	// Handler arrivé après le style : il reçoit sa section tout de suite
	const TSharedPtr<FJsonObject> Data = GetSection(CurrentStyle, Section.ToString());
	UWorld* World = GetGameWorld();
	if (Data.IsValid() && World && World->HasBegunPlay())
	{
		const FStyleSectionHandler& Registered = Handlers[Section];
		if (Registered.Apply) Registered.Apply(World, Data.ToSharedRef());
		ActiveSections.Add(Section);
	}
}

void UStyleSubsystem::UnregisterSectionHandler(FName Section)
{
	FStyleSectionHandler Handler;
	if (!Handlers.RemoveAndCopyValue(Section, Handler)) return;
	if (ActiveSections.Remove(Section) > 0 && Handler.Revert)
	{
		if (UWorld* World = GetGameWorld()) Handler.Revert(World);
	}
}

// ---------------------------------------------------------------------------
// Section "postprocess" : un volume non borné, une instance de matériau par entrée
// ---------------------------------------------------------------------------

void UStyleSubsystem::ApplyPostProcess(UWorld* World, const TSharedRef<FJsonObject>& Section)
{
	TArray<TSharedPtr<FJsonObject>> Entries;
	const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
	if (Section->TryGetArrayField(TEXT("items"), Items))
	{
		for (const TSharedPtr<FJsonValue>& Item : *Items)
		{
			if (Item.IsValid() && Item->Type == EJson::Object) Entries.Add(Item->AsObject());
		}
	}
	else
	{
		Entries.Add(Section);  // une seule entrée écrite comme objet
	}

	// Matériaux chargés en asynchrone : pas d'à-coup au thread de jeu en direct
	TArray<FSoftObjectPath> ToLoad;
	for (const TSharedPtr<FJsonObject>& Entry : Entries)
	{
		FString Path;
		if (Entry->TryGetStringField(TEXT("material"), Path) && !Path.IsEmpty())
		{
			const FSoftObjectPath Soft(Path);
			if (!Soft.ResolveObject()) ToLoad.Add(Soft);
		}
	}

	const int32 Generation = ++PostProcessGeneration;
	TWeakObjectPtr<UStyleSubsystem> WeakThis(this);
	TWeakObjectPtr<UWorld> WeakWorld(World);
	auto Build = [WeakThis, WeakWorld, Entries, Generation]()
	{
		UStyleSubsystem* Self = WeakThis.Get();
		if (!Self || !WeakWorld.IsValid() || Generation != Self->PostProcessGeneration) return;
		Self->BuildPostProcess(WeakWorld.Get(), Entries);
	};

	if (ToLoad.IsEmpty() || !UAssetManager::IsInitialized())
	{
		Build();
	}
	else
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(ToLoad, FStreamableDelegate::CreateLambda(Build));
	}
}

void UStyleSubsystem::BuildPostProcess(UWorld* World, const TArray<TSharedPtr<FJsonObject>>& Entries)
{
	TArray<TObjectPtr<UMaterialInstanceDynamic>> NewMaterials;
	TArray<float> Weights;
	bool bProtect = false;

	for (const TSharedPtr<FJsonObject>& Entry : Entries)
	{
		FString Path;
		Entry->TryGetStringField(TEXT("material"), Path);
		UMaterialInterface* Material = Path.IsEmpty() ? nullptr : Cast<UMaterialInterface>(FSoftObjectPath(Path).TryLoad());
		if (!Material)
		{
			UE_LOG(LogTrailStyle, Warning, TEXT("[Style] matériau de post-process introuvable : %s"), *Path);
			continue;
		}

		// Même matériau qu'avant : on garde l'instance, seuls les paramètres changent
		UMaterialInstanceDynamic* MID = nullptr;
		for (UMaterialInstanceDynamic* Existing : StyleMaterials)
		{
			if (Existing && Existing->Parent == Material && !NewMaterials.Contains(Existing))
			{
				MID = Existing;
				MID->ClearParameterValues();
				break;
			}
		}
		if (!MID)
		{
			MID = UMaterialInstanceDynamic::Create(Material, this);
		}

		const TSharedPtr<FJsonObject>* Scalars = nullptr;
		if (Entry->TryGetObjectField(TEXT("scalars"), Scalars))
		{
			for (const auto& Pair : (*Scalars)->Values)
			{
				MID->SetScalarParameterValue(FName(*Pair.Key), static_cast<float>(Pair.Value->AsNumber()));
			}
		}
		const TSharedPtr<FJsonObject>* Vectors = nullptr;
		if (Entry->TryGetObjectField(TEXT("vectors"), Vectors))
		{
			for (const auto& Pair : (*Vectors)->Values)
			{
				MID->SetVectorParameterValue(FName(*Pair.Key), TrailStyle::ParseColor(Pair.Value));
			}
		}

		double Weight = 1.0;
		Entry->TryGetNumberField(TEXT("weight"), Weight);
		bool bEntryProtect = false;
		if (Entry->TryGetBoolField(TEXT("protectOverlays"), bEntryProtect) && bEntryProtect) bProtect = true;

		NewMaterials.Add(MID);
		Weights.Add(FMath::Clamp(static_cast<float>(Weight), 0.f, 1.f));
	}

	if (NewMaterials.IsEmpty())
	{
		RevertPostProcess(World);
		return;
	}

	if (!IsValid(StyleVolume) || StyleVolume->GetWorld() != World)
	{
		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
#if WITH_EDITOR
		Params.bHideFromSceneOutliner = true;
#endif
		StyleVolume = World->SpawnActor<APostProcessVolume>(Params);
		if (!StyleVolume) return;
		StyleVolume->bUnbound = true;
		StyleVolume->Priority = 1000.f;
		StyleVolume->BlendWeight = 1.f;
	}

	FWeightedBlendables& Blendables = StyleVolume->Settings.WeightedBlendables;
	Blendables.Array.Reset();
	for (int32 i = 0; i < NewMaterials.Num(); ++i)
	{
		Blendables.Array.Add(FWeightedBlendable(Weights[i], NewMaterials[i]));
	}
	StyleVolume->bEnabled = true;
	StyleMaterials = MoveTemp(NewMaterials);

	SetOverlayProtection(World, bProtect);
}

void UStyleSubsystem::RevertPostProcess(UWorld* World)
{
	++PostProcessGeneration;  // annule un chargement en cours
	if (IsValid(StyleVolume))
	{
		StyleVolume->Destroy();
	}
	StyleVolume = nullptr;
	StyleMaterials.Reset();
	SetOverlayProtection(World, false);
}

// ---------------------------------------------------------------------------
// Overlays protégés : stencil 200 sur tracé, bornes, checkpoints, POI, coureurs
// ---------------------------------------------------------------------------

void UStyleSubsystem::ProtectActor(AActor* Actor, bool bProtect)
{
	if (!IsValid(Actor) || !TrailStyle::IsProtectedClass(Actor->GetClass())) return;
	TInlineComponentArray<UPrimitiveComponent*> Prims(Actor);
	for (UPrimitiveComponent* Prim : Prims)
	{
		Prim->SetRenderCustomDepth(bProtect);
		Prim->SetCustomDepthStencilValue(bProtect ? TrailStyle::OverlayStencil : 0);
	}
}

void UStyleSubsystem::OnActorSpawned(AActor* Actor)
{
	ProtectActor(Actor, true);
}

void UStyleSubsystem::SetOverlayProtection(UWorld* World, bool bProtect)
{
	if (bProtect == bOverlaysProtected || !World) return;
	bOverlaysProtected = bProtect;

	if (IConsoleVariable* CustomDepth = IConsoleManager::Get().FindConsoleVariable(TEXT("r.CustomDepth")))
	{
		if (bProtect)
		{
			// Valeur d'origine gardée une seule fois (un changement de niveau ne l'écrase pas)
			if (SavedCustomDepthMode == INDEX_NONE) SavedCustomDepthMode = CustomDepth->GetInt();
			if (CustomDepth->GetInt() < 3) CustomDepth->Set(3, ECVF_SetByCode);
		}
		else if (SavedCustomDepthMode != INDEX_NONE)
		{
			CustomDepth->Set(SavedCustomDepthMode, ECVF_SetByCode);
			SavedCustomDepthMode = INDEX_NONE;
		}
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		ProtectActor(*It, bProtect);
	}

	if (UWorld* Previous = SpawnHandleWorld.Get())
	{
		Previous->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
	}
	ActorSpawnedHandle.Reset();
	SpawnHandleWorld.Reset();
	if (bProtect)
	{
		ActorSpawnedHandle = World->AddOnActorSpawnedHandler(
			FOnActorSpawned::FDelegate::CreateUObject(this, &UStyleSubsystem::OnActorSpawned));
		SpawnHandleWorld = World;
	}
}
