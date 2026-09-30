// All Rights Reserved

#include "TeamSubsystem.h"

#include "CesiumFlyToComponent.h"
#include "HttpGatewaySubsystem.h"
#include "RaceSubsystem.h"
#include "RunnerInterface.h"
#include "RunnerSubsystem.h"
#include "SlateNotificationsBFL.h"
#include "Kismet/GameplayStatics.h"

class UCesiumFlyToComponent;

void UTeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(URaceSubsystem::StaticClass());
	Collection.InitializeDependency(UHttpGatewaySubsystem::StaticClass());
	Collection.InitializeDependency(URunnerSubsystem::StaticClass());

	if (UGameInstance* GI = GetGameInstance())
	{
		RaceSubsystem        = GI->GetSubsystem<URaceSubsystem>();
		HttpGatewaySubsystem = GI->GetSubsystem<UHttpGatewaySubsystem>();
		RunnerSubsystem      = GI->GetSubsystem<URunnerSubsystem>();

		if (RaceSubsystem)
		{
			RaceSubsystem->OnRaceSetupDatasGathered.AddDynamic(this, &UTeamSubsystem::SetRaceSetup);
		}

		if (HttpGatewaySubsystem)
		{
			HttpGatewaySubsystem->OnDisplayTeam.AddUObject(this, &UTeamSubsystem::DisplayTeam);
			HttpGatewaySubsystem->OnDisplayFlag.AddUObject(this, &UTeamSubsystem::DisplayFlag);
			HttpGatewaySubsystem->OnDisplayClub.AddUObject(this, &UTeamSubsystem::DisplayClub);
			HttpGatewaySubsystem->OnDisplayPhoto.AddUObject(this, &UTeamSubsystem::DisplayPhoto);
			HttpGatewaySubsystem->OnTeleportToTeam.AddUObject(this, &UTeamSubsystem::Teleport);
			// HttpGatewaySubsystem->OnFollowTeam.AddUObject(this, &UTeamSubsystem::Follow);
		}
	}
}

void UTeamSubsystem::Deinitialize()
{
	if (RaceSubsystem)
	{
		RaceSubsystem->OnRaceSetupDatasGathered.RemoveDynamic(this, &UTeamSubsystem::SetRaceSetup);
	}

	if (HttpGatewaySubsystem)
	{
		HttpGatewaySubsystem->OnDisplayTeam.RemoveAll(this);
		HttpGatewaySubsystem->OnDisplayFlag.RemoveAll(this);
		HttpGatewaySubsystem->OnDisplayClub.RemoveAll(this);
		HttpGatewaySubsystem->OnDisplayPhoto.RemoveAll(this);
		HttpGatewaySubsystem->OnTeleportToTeam.RemoveAll(this);
		HttpGatewaySubsystem->OnFollowTeam.RemoveAll(this);
	}

	RaceTeamsRunnersMap.Empty();

	RunnerSubsystem = nullptr;
	HttpGatewaySubsystem = nullptr;
	RaceSubsystem = nullptr;

	Super::Deinitialize();
}

void UTeamSubsystem::ClearRaceTeamIndex(int64 RaceID)
{
	if (FRaceTeamRunnerIndex* Idx = RaceTeamsRunnersMap.Find(RaceID))
	{
		Idx->TeamToRunner.Reset();
		Idx->RunnerToTeam.Reset();
	}
}

void UTeamSubsystem::RebuildRaceTeamIndex(int64 RaceID, const FRunners& RunnersDatas)
{
	FRaceTeamRunnerIndex& Idx = RaceTeamsRunnersMap.FindOrAdd(RaceID);

	// Reset garde la capacité mémoire, pratique si on rebuild souvent.
	Idx.TeamToRunner.Reset();
	Idx.RunnerToTeam.Reset();

	for (const FRunnerStruct& Runner : RunnersDatas.Runners)
	{
		if (Runner.runnerId <= 0 || Runner.canalId <= 0)
		{
			continue;
		}

		// Snapshot courant :
		// si plusieurs runners ont le même canalId dans le payload,
		// le dernier rencontré gagne.
		Idx.TeamToRunner.Add(Runner.canalId, Runner);
		Idx.RunnerToTeam.Add(Runner.runnerId, Runner.canalId);
	}
}

void UTeamSubsystem::SetRunnerToTeam(int64 RaceID, int64 TeamID, const FRunnerStruct& Runner)
{
	FRaceTeamRunnerIndex& Idx = RaceTeamsRunnersMap.FindOrAdd(RaceID);

	// Si ce runner appartenait déjà à une autre team, on nettoie l'ancienne relation
	if (const int64* OldTeamID = Idx.RunnerToTeam.Find(Runner.runnerId))
	{
		if (*OldTeamID != TeamID)
		{
			Idx.TeamToRunner.Remove(*OldTeamID);
		}
	}

	// Si cette team avait déjà un autre runner, on nettoie l'ancienne relation inverse
	if (const FRunnerStruct* OldRunner = Idx.TeamToRunner.Find(TeamID))
	{
		if (OldRunner->runnerId != Runner.runnerId)
		{
			Idx.RunnerToTeam.Remove(OldRunner->runnerId);
		}
	}

	Idx.TeamToRunner.Add(TeamID, Runner);
	Idx.RunnerToTeam.Add(Runner.runnerId, TeamID);
}

FRaceTeamRunnerIndex* UTeamSubsystem::GetTeamsForRace(int64 RaceID)
{
	return RaceTeamsRunnersMap.Find(RaceID);
}

const FRaceTeamRunnerIndex* UTeamSubsystem::GetTeamsForRace(int64 RaceID) const
{
	return RaceTeamsRunnersMap.Find(RaceID);
}

const FRunnerStruct* UTeamSubsystem::GetRunnerByRaceTeam(int64 RaceID, int64 TeamID) const
{
	if (const FRaceTeamRunnerIndex* Idx = RaceTeamsRunnersMap.Find(RaceID))
	{
		return Idx->TeamToRunner.Find(TeamID);
	}

	return nullptr;
}

// TObjectPtr<AActor> UTeamSubsystem::GetRunnerActorByRaceTeam(int64 RaceID, int64 TeamID) const
// {
// 	if (!RunnerSubsystem)
// 	{
// 		return nullptr;
// 	}
//
// 	if (const FRunnerStruct* Runner = GetRunnerByRaceTeam(RaceID, TeamID))
// 	{
// 		// return RunnerSubsystem->GetRunnerActor(RaceID, Runner->runnerId);
// 		return RunnerSubsystem->GetRunnerActorByTeam(RaceID, TeamID);
// 	}
//
// 	return nullptr;
// }
TObjectPtr<AActor> UTeamSubsystem::GetRunnerActorByRaceTeam(int64 RaceID, int64 TeamID) const
{
	if (!RunnerSubsystem)
	{
		return nullptr;
	}

	if (!GetRunnerByRaceTeam(RaceID, TeamID))
	{
		return nullptr;
	}

	return RunnerSubsystem->GetRunnerActorByTeam(RaceID, TeamID);
}


int64 UTeamSubsystem::GetTeamByRaceRunner(int64 RaceID, int64 RunnerID) const
{
	if (const FRaceTeamRunnerIndex* Idx = RaceTeamsRunnersMap.Find(RaceID))
	{
		if (const int64* TeamID = Idx->RunnerToTeam.Find(RunnerID))
		{
			return *TeamID;
		}
	}

	return -1;
}

void UTeamSubsystem::SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup)
{
	CurrentRaceSetup = NewRaceSetup;
}

// void UTeamSubsystem::DisplayTeam(int64 TeamID, int64 RaceID, bool bDisplay) const
// {
// 	if (!RunnerSubsystem)
// 	{
// 		USlateNotificationsBFL::SlateNotify(
// 			FText::FromString(TEXT("[DisplayTeam] RunnerSubsystem is null")),
// 			EMessageType::Error);
// 		return;
// 	}
//
// 	if (const FRunnerStruct* Runner = GetRunnerByRaceTeam(RaceID, TeamID))
// 	{
// 		// RunnerSubsystem->ToggleRunner(RaceID, Runner->runnerId, bDisplay);
// 		RunnerSubsystem->ToggleRunnerByTeam(RaceID, TeamID, bDisplay);
// 		return;
// 	}
//
// 	USlateNotificationsBFL::SlateNotify(
// 		FText::FromString(FString::Printf(TEXT("[DisplayTeam] Team %lld not found in Race %lld"), TeamID, RaceID)),
// 		EMessageType::Error);
// }

void UTeamSubsystem::DisplayTeam(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	if (!RunnerSubsystem)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(TEXT("[DisplayTeam] RunnerSubsystem is null")),
			EMessageType::Error);
		return;
	}

	if (GetRunnerByRaceTeam(RaceID, TeamID))
	{
		RunnerSubsystem->ToggleRunnerByTeam(RaceID, TeamID, bDisplay);
		return;
	}

	USlateNotificationsBFL::SlateNotify(
		FText::FromString(FString::Printf(TEXT("[DisplayTeam] Team %lld not found in Race %lld"), TeamID, RaceID)),
		EMessageType::Error);
}


void UTeamSubsystem::DisplayTeams(int64 RaceID, bool bDisplay) const
{
	if (!RunnerSubsystem)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(TEXT("[DisplayTeams] RunnerSubsystem is null")),
			EMessageType::Error);
		return;
	}

	RunnerSubsystem->ToggleRunners(RaceID, bDisplay);
}

void UTeamSubsystem::DisplayFlag(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActorByRaceTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(TEXT("[DisplayFlag] Team %lld not found in Race %lld"), TeamID, RaceID)),
			EMessageType::Error);
		return;
	}

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->ToggleFlag(bDisplay);
	}
}

void UTeamSubsystem::DisplayClub(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActorByRaceTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(TEXT("[DisplayClub] Team %lld not found in Race %lld"), TeamID, RaceID)),
			EMessageType::Error);
		return;
	}

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->ToggleClub(bDisplay);
	}
}

void UTeamSubsystem::DisplayPhoto(int64 TeamID, int64 RaceID, bool bDisplay) const
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActorByRaceTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(TEXT("[DisplayPhoto] Team %lld not found in Race %lld"), TeamID, RaceID)),
			EMessageType::Error);
		return;
	}

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->TogglePhoto(bDisplay);
	}
}

void UTeamSubsystem::Teleport(int64 TeamID, int64 RaceID)
{
	const FRunnerStruct* RunnerStruct = GetRunnerByRaceTeam(RaceID, TeamID);
	if (!RunnerStruct)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(TEXT("[Teleport] Cancelled: Team %lld not found in Race %lld"), TeamID, RaceID)),
			EMessageType::Error);
		return;
	}

	TObjectPtr<AActor> RunnerActor = GetRunnerActorByRaceTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(
				TEXT("[Teleport] Cancelled: Team %lld Runner %lld not found in Race %lld"),
				TeamID,
				RunnerStruct->runnerId,
				RaceID)),
			EMessageType::Error);
		return;
	}

	USlateNotificationsBFL::SlateNotify(
		FText::FromString(FString::Printf(
			TEXT("Teleport To Team %lld for Runner %lld - %s"),
			RunnerStruct->canalId,
			RunnerStruct->runnerId,
			*RunnerStruct->nom)),
		EMessageType::Success);

	APawn* DynaPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!DynaPawn)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(TEXT("[Teleport] PlayerPawn not found")),
			EMessageType::Error);
		return;
	}

	UCesiumFlyToComponent* FlyComp =
		Cast<UCesiumFlyToComponent>(DynaPawn->GetComponentByClass(UCesiumFlyToComponent::StaticClass()));

	if (!FlyComp)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(TEXT("[Teleport] CesiumFlyToComponent not found")),
			EMessageType::Error);
		return;
	}

	const FVector Destination = RunnerActor->GetActorLocation();
	FlyComp->FlyToLocationUnreal(Destination + FVector(0.f, 0.f, 50000.f), 0.f, 0.f, false);
}

void UTeamSubsystem::Animation(int64 TeamID, int64 RaceID)
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActorByRaceTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(TEXT("[Animation] Team %lld not found in Race %lld"), TeamID, RaceID)),
			EMessageType::Error);
		return;
	}

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->StartAnimation(100.f);
	}
}

void UTeamSubsystem::StopAnimation(int64 TeamID, int64 RaceID)
{
	TObjectPtr<AActor> RunnerActor = GetRunnerActorByRaceTeam(RaceID, TeamID);
	if (!RunnerActor)
	{
		USlateNotificationsBFL::SlateNotify(
			FText::FromString(FString::Printf(TEXT("[StopAnimation] Team %lld not found in Race %lld"), TeamID, RaceID)),
			EMessageType::Error);
		return;
	}

	if (IRunnerInterface* RunnerInterface = Cast<IRunnerInterface>(RunnerActor))
	{
		RunnerInterface->StopAnimation();
	}
}