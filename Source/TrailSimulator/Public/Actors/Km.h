// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KmInterface.h"
#include "Km.generated.h"

UCLASS()
class TRAILSIMULATOR_API AKm : public AActor, public IKmInterface
{
	GENERATED_BODY()

public:
	AKm();
	UFUNCTION()
	virtual void UpdateKm(int64 RaceID, const FText& NewLabel) const override;
	UFUNCTION()
	virtual void UpdateGlobeAnchor();
	UFUNCTION()
	virtual void UpdateDayNight(bool bIsDay) override;
	
	UPROPERTY()
	TObjectPtr<class ARaceManager> RaceManager;
protected:
	virtual void BeginPlay() override;
	void OnLookAtTimerTick();
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UStaticMeshComponent> StaticMeshComp;

private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USceneComponent> MainRoot;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UTextRenderComponent> LabelComp;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USettingsSubsystem> SettingsSubsystem;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UCesiumGlobeAnchorComponent> GlobeAnchorComponent;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TSoftObjectPtr<class ACesiumGeoreference> Georeference;
		
	//Pawn
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<APawn> DynaPawn;
	
	// Timer
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	FTimerHandle LookAtTimerHandle;
};