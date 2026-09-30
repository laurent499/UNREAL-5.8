// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/NameComponent.h"
#include "Nike_NameComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UNike_NameComponent : public UNameComponent
{
    GENERATED_BODY()

public:
    UNike_NameComponent(const FObjectInitializer& ObjectInitializer);
    virtual void UpdateName(const FString& RunnerName) override;
    
};
