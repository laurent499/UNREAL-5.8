// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TrailSharedTypes.h"
#include "RunnerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URunnerInterface : public UInterface
{
	GENERATED_BODY()
};

class TRAILINTERFACES_API IRunnerInterface
{
	GENERATED_BODY()

public:
	virtual void AssignRunnerToTeam(int64 IdTeam) = 0;
	virtual void UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup) = 0;
	
	virtual void ToggleFlag(bool bDisplay) = 0;
	virtual void ToggleClub(bool bDisplay) = 0;
	virtual void TogglePhoto(bool bDisplay) = 0;
	virtual bool IsPhotoVisible() = 0;
	virtual bool IsClubVisible() = 0;
	virtual bool IsFlagVisible() = 0;
	
	virtual FMinMax GetRunnerMinMax() const = 0;
	virtual float GetRunnerVDelta() const = 0;
	
	virtual void UpdateRunnerLocation(FRunnerStruct RunnerStruct, TObjectPtr<class APath> CurrentPath) = 0;
	virtual void TriggerUpdateAfterHidden(int64 RunnerId) = 0;
	
	virtual void StartAnimation(float RotationSpeed) = 0;
	virtual void StopAnimation() = 0;
	virtual void MarkDirty() = 0; // ou bDirty=true 
	
	virtual USceneComponent* GetFootHook() const = 0;
	virtual UStaticMeshComponent* GetFootMesh() const = 0;
	
	virtual void UpdateDayNight(bool bIsDay) = 0;
};
	
