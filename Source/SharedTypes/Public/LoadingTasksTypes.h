#pragma once

#include "CoreMinimal.h"

enum class ELoadingTaskState : uint8
{
	Queued,
	Running,
	Success,
	Failed
};

struct FLoadingTaskInfo{
	
	FName Id;
	FText Label;
	float Progress01 = 0.f;           // 0..1
	ELoadingTaskState State = ELoadingTaskState::Queued;
	FText Detail;                     // ex: "1200/1600 points"
	FSlateColor Color;
};
