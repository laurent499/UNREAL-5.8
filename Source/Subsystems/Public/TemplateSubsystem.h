// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "TrailTemplateTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "TemplateSubsystem.generated.h"

/** Diffusé quand un template a été relu sur la régie (commande template/reload). */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTemplateChanged, const FString& /*TemplateName*/);

/**
 * @brief Templates graphiques des courses, lus sur la régie.
 * @note Un template est demandé une fois au chargement de la course, avant l'apparition
 *       des checkpoints, POI et coureurs ; rien ne se recalcule ensuite à chaque frame.
 *       Si la régie ne le connaît pas, les valeurs d'origine (GTWS, Generic) s'appliquent.
 */
UCLASS()
class SUBSYSTEMS_API UTemplateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	/**
	 * @brief Récupère le template sur la régie s'il n'est pas déjà connu.
	 * @param OnDone Appelé sur le game thread, que la requête aboutisse ou non.
	 */
	void RequestTemplate(const FString& TemplateName, TFunction<void()> OnDone);

	/** Relit le template sur la régie puis prévient les acteurs (OnTemplateChanged). */
	void ReloadTemplate(const FString& TemplateName);

	/** Template d'une course : celui de la régie, sinon les valeurs d'origine. */
	FTrailTemplate GetTemplate(const FString& TemplateName) const;

	/** Famille de classes à faire apparaître pour cette course (Generic, GTWS, UTMB, Nike). */
	FString GetBase(const FRaceSetup& RaceSetup) const;

	/** Raccourci pour les acteurs : template de la course via le game instance. */
	static FTrailTemplate GetForRace(const UObject* WorldContext, const FRaceSetup& RaceSetup);

	/** Hauteur du cartouche des coureurs, sans copier le template (appelé pour chaque coureur). */
	static float GetRunnerHeight(const UObject* WorldContext, const FRaceSetup& RaceSetup);

	/**
	 * @brief Template de la course sans copie quand il est en cache (coureurs, à chaque snapshot).
	 * @param Storage Reçoit les valeurs d'origine si le template n'est pas encore en cache.
	 * @note La référence reste valable jusqu'au prochain retour HTTP : ne pas la conserver.
	 */
	static const FTrailTemplate& ResolveForRace(const UObject* WorldContext, const FRaceSetup& RaceSetup,
		TOptional<FTrailTemplate>& Storage);

	/** Impose un template (levels cinema : regie injoignable, ou arrondi des panneaux regle sur place). */
	void SetTemplate(const FString& TemplateName, const FTrailTemplate& Template) { Templates.Add(TemplateName, Template); }

	FOnTemplateChanged OnTemplateChanged;

private:
	void Fetch(const FString& TemplateName, TFunction<void(bool)> OnDone);
	static FTrailTemplate GetBuiltin(const FString& TemplateName);

	UPROPERTY()
	TMap<FString, FTrailTemplate> Templates;

	TMap<FString, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ActiveRequests;
};
