// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IHttpRequest.h"
#include "StyleSubsystem.generated.h"

class APostProcessVolume;
class UMaterialInstanceDynamic;
class UMaterialInterface;

/** Diffusé après chaque changement de style (nom du nouveau style). */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStyleChanged, const FString& /*StyleName*/);

/**
 * @brief Handler d'une section de style ("imagery", "sky"...), fourni par le fil qui la possède.
 * @note Appelés sur le game thread, uniquement au changement de style ou de niveau.
 */
struct FStyleSectionHandler
{
	/** Applique la section ; peut être rappelé alors qu'elle est déjà active (réapplication). */
	TFunction<void(UWorld* World, const TSharedRef<FJsonObject>& Section)> Apply;
	/** Revient au rendu d'origine pour ce domaine. */
	TFunction<void(UWorld* World)> Revert;
};

/**
 * @brief Rendu global choisi dans la régie (Réaliste, Noir et blanc, Carto, Cell shading...).
 * @note Un style est un JSON à sections ; chaque section va au handler du même nom.
 *       "realiste" n'a aucune section : rien n'est créé, rien ne coûte. Rien par image.
 *       Contrat complet : Docs/Styles/CONTRAT_STYLES.md.
 */
UCLASS()
class SUBSYSTEMS_API UStyleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString RealisticStyleName;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Applique un style dont le JSON est fourni (commande style set avec data). */
	bool ApplyStyleJson(const FString& StyleName, const FString& StyleJson);

	/** Applique un style connu par son nom : "realiste" tout de suite, sinon lecture sur la régie. */
	void ApplyStyle(const FString& StyleName);

	/** Relit le style courant sur la régie et le réapplique. */
	void ReloadStyle() { ApplyStyle(CurrentStyleName); }

	const FString& GetCurrentStyleName() const { return CurrentStyleName; }

	/** Enregistre le handler d'une section ; il reçoit tout de suite la section du style courant. */
	void RegisterSectionHandler(FName Section, FStyleSectionHandler Handler);

	/** Retire un handler ; son Revert est appelé si sa section était active. */
	void UnregisterSectionHandler(FName Section);

	FOnStyleChanged OnStyleChanged;

private:
	void ApplyParsed(const FString& StyleName, const TSharedPtr<FJsonObject>& Style);
	void ApplySections(UWorld* World);
	static TSharedPtr<FJsonObject> GetSection(const TSharedPtr<FJsonObject>& Style, const FString& Key);
	void OnWorldBeginPlay(UWorld* World);
	UWorld* GetGameWorld() const;

	// Section "postprocess", gérée par le cadre
	void ApplyPostProcess(UWorld* World, const TSharedRef<FJsonObject>& Section);
	void RevertPostProcess(UWorld* World);
	void BuildPostProcess(UWorld* World, const TArray<TSharedPtr<FJsonObject>>& Entries);
	void SetOverlayProtection(UWorld* World, bool bProtect);
	void OnActorSpawned(AActor* Actor);
	static void ProtectActor(AActor* Actor, bool bProtect);

	FString CurrentStyleName;
	TSharedPtr<FJsonObject> CurrentStyle;
	TMap<FName, FStyleSectionHandler> Handlers;
	/** Sections appliquées dans le monde courant (pour savoir quoi défaire). */
	TSet<FName> ActiveSections;

	UPROPERTY(Transient)
	TObjectPtr<APostProcessVolume> StyleVolume;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> StyleMaterials;

	bool bOverlaysProtected = false;
	int32 SavedCustomDepthMode = INDEX_NONE;
	FDelegateHandle ActorSpawnedHandle;
	TWeakObjectPtr<UWorld> SpawnHandleWorld;
	int32 PostProcessGeneration = 0;

	FDelegateHandle PostWorldInitHandle;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ActiveRequest;
};
