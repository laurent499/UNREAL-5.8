// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FlagComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UFlagComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UFlagComponent(const FObjectInitializer& ObjectInitializer);
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> FlagSMC;
	UFUNCTION()
	virtual void UpdateFlag(const FString& Country);
	virtual void ToggleFlag(bool bShow) const;
	UFUNCTION()
	virtual void UpdateDayNight(bool bIsDay);

protected:
	// Applique la luminosité correspondant à l'état jour/nuit courant
	void ApplyDayNightBrightness() const;

	// Pays actuellement affiché : évite de recharger la texture à chaque fetch
	UPROPERTY()
	FString CurrentCountry;
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> FlagMID;
	UPROPERTY()
	bool bIsDayCached = true;
};
