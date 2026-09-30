// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/NameComponent.h"
#include "Generic_NameComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UGeneric_NameComponent : public UNameComponent
{
	GENERATED_BODY()

public:
	UGeneric_NameComponent(const FObjectInitializer& ObjectInitializer);
	virtual void UpdateName(const FString& RunnerName) override;
	virtual float GetMiddle() const override;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UStaticMeshComponent> NameLeftComponent;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UStaticMeshComponent> NameRightComponent;
};