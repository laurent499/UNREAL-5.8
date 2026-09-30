#include "LoadingStatusSubsystem.h"

#include "TrailUtilsLog.h"

static float Clamp01(float V) { return FMath::Clamp(V, 0.f, 1.f); }

void ULoadingStatusSubsystem::RegisterTask(FName Id, const FText& Label)
{
	FLoadingTaskInfo& T = Tasks.FindOrAdd(Id);
	T.Id = Id;
	T.Label = Label;
	T.State = ELoadingTaskState::Queued;
	T.Progress01 = 0.f;
	T.Detail = FText::GetEmpty();
	T.Color = FSlateColor(FLinearColor(0.f,0.5f,1.f,1.f));
	BroadcastChanged();
}

void ULoadingStatusSubsystem::SetRunning(FName Id, const FText& Detail)
{
	if (FLoadingTaskInfo* T = Tasks.Find(Id))
	{
		T->State = ELoadingTaskState::Running;
		T->Progress01 = Clamp01(0.5f);
		T->Color = FSlateColor(FLinearColor(1.f,0.5f,0.f,1.f));
		if (!Detail.IsEmpty()) T->Detail = Detail;
		BroadcastChanged();
	}
}

void ULoadingStatusSubsystem::Update(FName Id, float Progress01, const FText& Detail)
{
	if (FLoadingTaskInfo* T = Tasks.Find(Id))
	{
		// UE_LOG(LogTemp, Warning, TEXT("[Frame %llu] UPDATE %s P=%f"), GFrameCounter, *Id.ToString(), Progress01);
		ensure(IsInGameThread());
		T->State = ELoadingTaskState::Running;
		T->Progress01 = Clamp01(Progress01);
		T->Color = FSlateColor(FLinearColor(1.f,0.5f,0.f,1.f));
		if (!Detail.IsEmpty()) T->Detail = Detail;
		BroadcastChanged();
	}
}

void ULoadingStatusSubsystem::Complete(FName Id, const FText& Detail)
{
	if (FLoadingTaskInfo* T = Tasks.Find(Id))
	{
		T->State = ELoadingTaskState::Success;
		T->Progress01 = 1.f;
		T->Color = FSlateColor(FLinearColor(0.f,1.f,0.f,1.f));
		if (!Detail.IsEmpty()) T->Detail = Detail;
		BroadcastChanged();
	}
}

void ULoadingStatusSubsystem::Fail(FName Id, const FText& Detail)
{
	if (FLoadingTaskInfo* T = Tasks.Find(Id))
	{
		T->State = ELoadingTaskState::Failed;
		T->Progress01 = 1.f;
		T->Color = FSlateColor(FLinearColor(1.f,0.f,0.f,1.f));
		if (!Detail.IsEmpty()) T->Detail = Detail;
		BroadcastChanged();
	}
}

TArray<FLoadingTaskInfo> ULoadingStatusSubsystem::GetSnapshot() const
{
	TArray<FLoadingTaskInfo> Out;
	Tasks.GenerateValueArray(Out);
	Out.Sort([](const FLoadingTaskInfo& A, const FLoadingTaskInfo& B)
	{
		return A.Label.ToString() < B.Label.ToString();
	});	
	return Out;
}

float ULoadingStatusSubsystem::GetOverallProgress01() const
{
	double Sum = 0.0;
	int32 Count = 0;

	for (const auto& It : Tasks)
	{
		const FLoadingTaskInfo& T = It.Value;
		Sum += T.Progress01;
		Count++;
	}
	return (Count > 0) ? float(Sum / double(Count)) : 0.f;
}

bool ULoadingStatusSubsystem::AreAllDoneSuccessfully() const
{
	if (Tasks.Num() == 0) return false;

	for (const auto& It : Tasks)
	{
		const FLoadingTaskInfo& T = It.Value;
		if (T.State != ELoadingTaskState::Success && T.State != ELoadingTaskState::Failed) return false;
	}
	return true;
}

void ULoadingStatusSubsystem::MarkDirty()
{
	bDirty = true;

	// Déjà planifié ? ne rien faire
	if (FlushHandle.IsValid())
		return;

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().SetTimer(FlushHandle, this, &ULoadingStatusSubsystem::Flush, 0.05f, true);
	}
	else
	{
		// fallback
		Flush();
	}
}

void ULoadingStatusSubsystem::Flush()
{
	// UE_LOG(LogTemp, Warning, TEXT("Flushing"));
	FlushHandle.Invalidate();

	if (!bDirty)
		return;

	bDirty = false;
	OnChanged.Broadcast();
}
