// All Rights Reserved


#include "TeamGroupSubsystem.h"
#include "RaceSubsystem.h"
#include "TeamSubsystem.h"

void UTeamGroupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(URaceSubsystem::StaticClass());
	if (GetGameInstance())
	{
		TeamSubsystem = GetGameInstance()->GetSubsystem<UTeamSubsystem>();
		RaceSubsystem = GetGameInstance()->GetSubsystem<URaceSubsystem>();
		if (RaceSubsystem){
			RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &UTeamGroupSubsystem::SetRaceSetup);
		}
	}
	
}

void UTeamGroupSubsystem::AddTeamToGroup(int64 RaceID, int64 GroupID, int64 TeamID)
{
	// Find retourne un *
	// FindOrAdd retourne une &
	// Add(Key) retourne une &
	auto& GroupMap = RaceGroupTeamMap.FindOrAdd(RaceID);
	auto& Teams = GroupMap.FindOrAdd(GroupID);
	Teams.AddUnique(TeamID);
	
}

void UTeamGroupSubsystem::RemoveTeamFromGroup(int64 RaceID, int64 GroupID, int64 TeamID)
{
	auto* GroupMap = RaceGroupTeamMap.Find(RaceID);
	if (!GroupMap) return;
	auto* Teams = GroupMap->Find(GroupID);
	if (!Teams) return;

	Teams->RemoveSingle(TeamID);
	if (Teams->Num() == 0) GroupMap->Remove(GroupID);
	if (GroupMap->Num() == 0) RaceGroupTeamMap.Remove(RaceID);
}
TArray<int64> UTeamGroupSubsystem::GetTeamsInGroup(int64 RaceID, int64 GroupID)
{
	if (const auto* GroupMap = RaceGroupTeamMap.Find(RaceID))
	{
		if (const auto* Teams = GroupMap->Find(GroupID))
		{
			return *Teams;
		}
	}
	return TArray<int64>();
}

void UTeamGroupSubsystem::ToggleGroup(int64 RaceID, int64 GroupID, bool bDisplay)
{
	TArray<int64> Group = GetTeamsInGroup(RaceID, GroupID);
	if (Group.Num() == 0) return;
	for (int64 TeamID : Group)
	{
		TeamSubsystem->DisplayTeam(TeamID, RaceID, bDisplay);
	}
}

void UTeamGroupSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup)
{
	CurrentRaceSetup = NewRaceSetup;
}
