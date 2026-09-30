#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "SharedTypes/Public/LoadingTasksTypes.h"

class ULoadingStatusSubsystem;
class UFontFace;

class SLoadingOverlay : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS(SLoadingOverlay) {}
		SLATE_ARGUMENT(TWeakObjectPtr<ULoadingStatusSubsystem>, Loading)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
	virtual ~SLoadingOverlay() override;

private:
	TWeakObjectPtr<ULoadingStatusSubsystem> LoadingSubsystem;

	// ListView data
	using FTaskItemPtr = TSharedPtr<FLoadingTaskInfo>;
	TArray<FTaskItemPtr> Items;
	TSharedPtr<SListView<FTaskItemPtr>> ListView;

	// Delegate handle pour se désabonner proprement
	FDelegateHandle ChangedHandle;

	// Rebuild
	void RefreshItemsFromSubsystem();
	void HandleChanged();

	// Bindings
	TOptional<float> GetOverallPercent() const;

	// Row generator
	TSharedRef<ITableRow> OnGenerateRow(FTaskItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	// Helpers UI
	static FText StateToText(ELoadingTaskState State);
	
};
