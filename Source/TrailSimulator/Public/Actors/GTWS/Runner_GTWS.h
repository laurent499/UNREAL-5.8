// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Runner.h"
#include "Runner_GTWS.generated.h"

UCLASS()
class TRAILSIMULATOR_API ARunner_GTWS : public ARunner
{
	GENERATED_BODY()

public:
	ARunner_GTWS();
	virtual void UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup) override;
	
	virtual void ToggleClub(bool bDisplay) override;
	virtual void ToggleFlag(bool bDisplay) override;
	virtual void TogglePhoto(bool bDisplay) override;
	
	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;
	virtual void UpdateDayNight(bool bIsDay) override;

protected:
	virtual void BeginPlay() override;
	// Photo
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> PhotoHook;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UPhotoComponent> PhotoComponent;
	
	// Name
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UGTWS_NameComponent> NameComponent;
	
	// Flag
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> FlagHook;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UGTWS_FlagComponent> FlagComponent;
	
	// Club
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> ClubHook;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UGTWS_ClubComponent> ClubComponent;
		
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	FVector DefaultClubLocation;
	FVector DefaultNameLocation;
};
