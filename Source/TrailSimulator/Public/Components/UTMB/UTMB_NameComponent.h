// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/NameComponent.h"
#include "UTMB_NameComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UUTMB_NameComponent : public UNameComponent
{
	GENERATED_BODY()

public:
	UUTMB_NameComponent(const FObjectInitializer& ObjectInitializer);
	virtual void UpdateName(const FString& RunnerName) override;
	void UpdateMesh(FVector NewFirstLocation, FVector NewLastLocation);
	virtual float GetMiddle() const override;
	virtual void UpdateDayNight(bool bIsDay) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<class USplineComponent> SplineComponent;	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Name", meta=(AllowPrivateAccess="true"), Instanced)
	TObjectPtr<class UStaticMeshComponent> RightSlashComponent;
	
	
};
