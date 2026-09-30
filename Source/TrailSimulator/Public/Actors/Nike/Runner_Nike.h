// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Runner.h"
#include "Runner_Nike.generated.h"

UCLASS()
class TRAILSIMULATOR_API ARunner_Nike : public ARunner
{
    GENERATED_BODY()

public:
    ARunner_Nike();
    virtual void UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup) override;
	
protected:	
    // Name
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
    TObjectPtr<class UNike_NameComponent> NameComponent;
	
	// Club
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> ClubHook;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UNike_ClubComponent> ClubComponent;
	
	// Flag
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UNike_FlagComponent> FlagComponent;
	
	UPROPERTY()
	TObjectPtr<class UFont> LabelFont;
	UPROPERTY()
	TObjectPtr<class UFont> ClubFont;
	
	
	
};
