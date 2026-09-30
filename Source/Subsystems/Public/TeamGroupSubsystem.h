// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "TeamGroupSubsystem.generated.h"

UCLASS()
class SUBSYSTEMS_API UTeamGroupSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void AddTeamToGroup(int64 RaceID, int64 GroupID, int64 TeamID);
	void RemoveTeamFromGroup(int64 RaceID, int64 GroupID, int64 TeamID);
	void ToggleGroup(int64 RaceID, int64 GroupID, bool bDisplay);
	TArray<int64> GetTeamsInGroup(int64 RaceID, int64 GroupID);
	
private:
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UPROPERTY()
	TObjectPtr<class UTeamSubsystem> TeamSubsystem;
	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup);
	
	// idrace - idgroup/idteam
	TMap<int64, TMap<int64, TArray<int64>>> RaceGroupTeamMap;
};
