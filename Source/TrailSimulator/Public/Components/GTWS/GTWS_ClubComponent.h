// Copyright LTVProd 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ClubComponent.h"
#include "GTWS_ClubComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UGTWS_ClubComponent : public UClubComponent
{
    GENERATED_BODY()

public:
    UGTWS_ClubComponent(const FObjectInitializer& ObjectInitializer);
    virtual void UpdateClub(const FRunnerStruct& RunnerDatas) override;


};
