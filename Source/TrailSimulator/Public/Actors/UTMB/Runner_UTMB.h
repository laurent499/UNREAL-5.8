// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Actors/Runner.h"
#include "Runner_UTMB.generated.h"

UCLASS()
class TRAILSIMULATOR_API ARunner_UTMB : public ARunner
{
	GENERATED_BODY()

public:
	ARunner_UTMB();
	virtual void UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup) override;
	
	virtual void ToggleFlag(bool bDisplay) override;
	virtual void TogglePhoto(bool bDisplay) override;
	virtual void UpdateDayNight(bool bIsDay) override;
	
	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;

protected:
	virtual void BeginPlay() override;
	
	// Name
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UUTMB_NameComponent> NameComponent;
	
	// Flag
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> FlagHook;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UUTMB_FlagComponent> FlagComponent;
	
	// Photo
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> PhotoHook;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UPhotoComponent> PhotoComponent;
	
	/** Index */
	// Index Hook
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> IndexBkgHook;
	// Index Bkg
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> IndexBkgComponent;
	// Index Left
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> IndexLeftComponent;
	// Index Barre
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> IndexBarreComponent;
	// Index Right
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> IndexRightComponent;
	
	// UTMB Label 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UTextRenderComponent> UtmbLabelComponent;
	
	// Index Label 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UTextRenderComponent> IndexLabelComponent;
	
	// Index Value
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UTextRenderComponent> IndexValueComponent;
};
