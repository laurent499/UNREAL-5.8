// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ClubComponent.h"
#include "Nike_ClubComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UNike_ClubComponent : public UClubComponent
{
    GENERATED_BODY()

public:
    UNike_ClubComponent(const FObjectInitializer& ObjectInitializer);
    virtual void UpdateClub(const FRunnerStruct& RunnerDatas) override;
};
