// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrailSharedTypes.h"
#include "TeamSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnTeamsGathered,
	int64,
	RaceID,
	FTeams,
	Teams);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnTeamRunnerGathered,
	int64,
	RunnerId,
	FTeamStruct,
	TeamStruct);

/**
 * @brief Subsystem de gestion des Teams
 *
 * Source de vérité unique :
 * RaceID -> FRaceTeamRunnerIndex
 *   - TeamToRunner : TeamID   -> FRunnerStruct
 *   - RunnerToTeam : RunnerID -> TeamID
 */
UCLASS()
class SUBSYSTEMS_API UTeamSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * @brief Rebuild complet de l'index team <-> runner pour une race
	 * à partir d'un snapshot courant de runners.
	 */
	UFUNCTION()
	void RebuildRaceTeamIndex(int64 RaceID, const FRunners& RunnersDatas);

	/**
	 * @brief Vide l'index team <-> runner d'une race.
	 */
	UFUNCTION()
	void ClearRaceTeamIndex(int64 RaceID);

	/**
	 * @brief Affectation ponctuelle d'un runner à une team.
	 * À garder pour des usages ciblés, mais le flux normal doit préférer RebuildRaceTeamIndex().
	 */
	UFUNCTION()
	void SetRunnerToTeam(int64 RaceID, int64 TeamID, const FRunnerStruct& Runner);

	/**
	 * @brief Retourne l'index d'une race.
	 */
	FRaceTeamRunnerIndex* GetTeamsForRace(int64 RaceID);

	/**
	 * @brief Retourne l'index d'une race (const).
	 */
	const FRaceTeamRunnerIndex* GetTeamsForRace(int64 RaceID) const;

	/**
	 * @brief Retourne TeamID pour RaceID + RunnerID.
	 * @return -1 si non trouvé
	 */
	int64 GetTeamByRaceRunner(int64 RaceID, int64 RunnerID) const;

	/**
	 * @brief Retourne le runner courant d'une team pour une race.
	 * @return nullptr si non trouvé
	 */
	const FRunnerStruct* GetRunnerByRaceTeam(int64 RaceID, int64 TeamID) const;

	/**
	 * @brief Retourne l'actor du runner courant d'une team pour une race.
	 * @return nullptr si non trouvé
	 */
	TObjectPtr<AActor> GetRunnerActorByRaceTeam(int64 RaceID, int64 TeamID) const;

	UFUNCTION()
	void DisplayTeam(int64 TeamID, int64 RaceID, bool bDisplay) const;

	UFUNCTION()
	void DisplayTeams(int64 RaceID, bool bDisplay) const;

	UFUNCTION()
	void DisplayFlag(int64 TeamID, int64 RaceID, bool bDisplay) const;

	UFUNCTION()
	void DisplayClub(int64 TeamID, int64 RaceID, bool bDisplay) const;

	UFUNCTION()
	void DisplayPhoto(int64 TeamID, int64 RaceID, bool bDisplay) const;

	UFUNCTION()
	void Teleport(int64 TeamID, int64 RaceID);

	UFUNCTION()
	void Animation(int64 TeamID, int64 RaceID);

	UFUNCTION()
	void StopAnimation(int64 TeamID, int64 RaceID);

private:
	// Race Setup
	UPROPERTY()
	FRaceSetup CurrentRaceSetup;

	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;

	UPROPERTY()
	TObjectPtr<class UHttpGatewaySubsystem> HttpGatewaySubsystem;

	UPROPERTY()
	TObjectPtr<class URunnerSubsystem> RunnerSubsystem;

	UFUNCTION()
	void SetRaceSetup(int64 RaceID, FRaceSetup NewRaceSetup);

private:
	/**
	 * @brief Source de vérité unique :
	 * RaceID -> index bidirectionnel TeamID <-> Runner
	 */
	UPROPERTY()
	TMap<int64, FRaceTeamRunnerIndex> RaceTeamsRunnersMap;
};