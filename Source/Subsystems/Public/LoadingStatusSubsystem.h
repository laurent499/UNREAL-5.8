// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoadingTasksTypes.h"
#include "LoadingStatusSubsystem.generated.h"


/**
 * @brief Subsystem in charge of the loading screen
 */
UCLASS()
class SUBSYSTEMS_API ULoadingStatusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// Delegate "Slate-friendly" (non-dynamic, pas Blueprint)
	DECLARE_MULTICAST_DELEGATE(FOnChangedNative);
	FOnChangedNative OnChanged;

	void RegisterTask(FName Id, const FText& Label);
	void SetRunning(FName Id, const FText& Detail = FText::GetEmpty());
	void Update(FName Id, float Progress01, const FText& Detail = FText::GetEmpty());
	void Complete(FName Id, const FText& Detail = FText::GetEmpty());
	void Fail(FName Id, const FText& Detail = FText::GetEmpty());

	TArray<FLoadingTaskInfo> GetSnapshot() const;
	float GetOverallProgress01() const;
	bool AreAllDoneSuccessfully() const;
	
	bool bDirty = false;
	FTimerHandle FlushHandle;
	
	void MarkDirty();
	void Flush();

private:
	TMap<FName, FLoadingTaskInfo> Tasks;

	FORCEINLINE void BroadcastChanged() { OnChanged.Broadcast(); }
	
};
