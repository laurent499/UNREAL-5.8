#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SharedTypes/Public/LoadingTasksTypes.h"

class ULoadingStatusSubsystem;
class SVerticalBox;
class STextBlock;

/**
 * Ecran « Initialisation » affiche pendant le chargement des courses.
 * Une carte par course avec ses etapes (reglages, trace, coureurs, POI, checkpoints),
 * une barre de progression globale et le resume des taches terminees / en erreur.
 */
class SLoadingOverlay : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLoadingOverlay) {}
		SLATE_ARGUMENT(TWeakObjectPtr<ULoadingStatusSubsystem>, Loading)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SLoadingOverlay() override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TWeakObjectPtr<ULoadingStatusSubsystem> LoadingSubsystem;

	// Delegate handle pour se désabonner proprement
	FDelegateHandle ChangedHandle;

	// Le subsystem notifie a chaque coureur spawne : on reconstruit au plus toutes les 100 ms
	bool bDirty = true;
	double LastRebuildTime = 0.0;

	TSharedPtr<SVerticalBox> CardsBox;
	TSharedPtr<STextBlock> SummaryText;
	TSharedPtr<STextBlock> PercentText;

	void HandleChanged();
	void Rebuild();

	// Bindings
	TOptional<float> GetOverallPercent() const;
};
