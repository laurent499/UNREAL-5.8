// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/FlagComponent.h"
#include "Nike_FlagComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UNike_FlagComponent : public UFlagComponent
{
    GENERATED_BODY()

public:
    UNike_FlagComponent(const FObjectInitializer& ObjectInitializer);

};
