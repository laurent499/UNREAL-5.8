// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TrailSharedTypes.h"
#include "ClubComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UClubComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UClubComponent(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UTextRenderComponent> ClubTextComponent;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USplineComponent> ClubSplineComponent;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USplineMeshComponent> ClubBkgComponent;
	
	virtual void UpdateClub(const FRunnerStruct& RunnerDatas);
	UFUNCTION()
	FVector GetLocalSize() const;
	UFUNCTION()
	void UpdateMesh(FVector NewFirstLocation, FVector NewLastLocation);
	UFUNCTION()
	FStartAndEnd GetStartAndEnd() const;
	UFUNCTION()
	float GetMiddle() const;
	virtual void ToggleClub(bool bShow) const;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UTrailTextWidgetComponent> WidgetClubComponent;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UFont> ClubFont;
	
	UFUNCTION()
	void ShowWidgetName(bool bIsVisible);
	UFUNCTION()
	void Show3DName(bool bIsVisible);
	
	UFUNCTION()
	virtual void UpdateDayNight(bool bIsDay) const;
	
	UPROPERTY()
	bool bShow3DText;
	UPROPERTY()
	bool bShowWidgetText;
	UPROPERTY(EditDefaultsOnly)
	FVector StartLocation;
	UPROPERTY(EditDefaultsOnly)
	FVector LastLocation;
private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	
	
	
	
};
