// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TrailSharedTypes.h"
#include "Components/SceneComponent.h"
#include "NameComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UNameComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UNameComponent(const FObjectInitializer& ObjectInitializer);
	UFUNCTION()
	virtual void UpdateName(const FString& RunnerName);
	UFUNCTION()
	virtual void UpdateMesh(FVector NewFirstLocation, FVector NewLastLocation);
	UFUNCTION()
	virtual float GetMiddle() const;
	UFUNCTION()
	FVector GetLocalSize() const;
	FVector GetWorldLastPointLocation() const;
	UFUNCTION()
	virtual void UpdateDayNight(bool bIsDay);
	
	UFUNCTION()
	virtual FStartAndEnd GetStartAndEnd() const;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USplineComponent> NameSplineComponent;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USplineMeshComponent> NameBkgComponent;
	
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UTrailTextWidgetComponent> WidgetNameComponent;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UFont> NameFont;
	
	UFUNCTION()
	void ShowWidgetName(bool bIsVisible);
	UFUNCTION()
	void Show3DName(bool bIsVisible);
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UTextRenderComponent> NameTextComponent;
			
	UPROPERTY(EditDefaultsOnly)
	FVector StartLocation;
	UPROPERTY(EditDefaultsOnly)
	FVector LastLocation;
	TObjectPtr<class USplineComponent> GetSplineComponent() const;
	
	UPROPERTY()
	TObjectPtr<class UFont> LabelFont;
	UPROPERTY()
	bool bShow3DText;
	UPROPERTY()
	bool bShowWidgetText;
	
};
