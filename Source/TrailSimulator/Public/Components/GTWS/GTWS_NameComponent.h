// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/NameComponent.h"
#include "GTWS_NameComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UGTWS_NameComponent : public UNameComponent
{
	GENERATED_BODY()

public:
	UGTWS_NameComponent(const FObjectInitializer& ObjectInitializer);
	virtual void UpdateName(const FString& RunnerName) override;
	virtual float GetMiddle() const override;
	virtual FStartAndEnd GetStartAndEnd() const override;
	UFUNCTION()
	float GetSplineLength() const;
	UFUNCTION()
	FVector GetSplineLastPoint() const;
	virtual void UpdateDayNight(bool bIsDay) override;
};
