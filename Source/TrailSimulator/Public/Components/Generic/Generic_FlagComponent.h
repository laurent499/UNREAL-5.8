// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/FlagComponent.h"
#include "Generic_FlagComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UGeneric_FlagComponent : public UFlagComponent
{
	GENERATED_BODY()

public:
	UGeneric_FlagComponent(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> FlagHook;
public:
};
