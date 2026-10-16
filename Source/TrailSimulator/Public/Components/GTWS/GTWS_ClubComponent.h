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

    /** Couleurs du template : fond et texte du club. Sans effet si elles n'ont pas changé. */
    void SetColors(const FLinearColor& BkgColor, const FColor& InTextColor);

private:
    FLinearColor AppliedBkgColor = FLinearColor::Transparent;
    FColor TextColor = FColor::White;


};
