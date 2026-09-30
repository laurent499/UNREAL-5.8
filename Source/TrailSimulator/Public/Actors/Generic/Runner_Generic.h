// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Runner.h"
#include "Runner_Generic.generated.h"

UCLASS()
class TRAILSIMULATOR_API ARunner_Generic : public ARunner
{
	GENERATED_BODY()

public:
	ARunner_Generic();
	virtual void UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup) override;
	 
	virtual void ToggleFlag(bool bDisplay) override;
	virtual void TogglePhoto(bool bDisplay) override;
	
	virtual void SetIsStacked(bool bInStacked) override;
	
	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;
	
	virtual void UpdateDayNight(bool bIsDay) override;
protected:
	virtual void BeginPlay() override;
	// Name
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UGeneric_NameComponent> NameComponent;
	
	// Flag
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UGeneric_FlagComponent> FlagComponent;
	
	// Photo
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> PhotoHook;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UPhotoComponent> PhotoComponent;
	
	// Slah
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> SlashComponent;
};
