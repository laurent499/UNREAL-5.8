// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoiInterface.h"
#include "ScaleInterface.h"
#include "BroadcastInterface.h"
#include "CameraControlInterface.h"
#include "SharedTypes/Public/TrailSharedTypes.h"
#include "Poi.generated.h"

UCLASS()
class TRAILSIMULATOR_API APoi : public AActor, public IPoiInterface , public IScaleInterface, public IBroadcastInterface, public ICameraControlInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static void GetBoundsLocation(class USceneComponent* Comp, class USplineComponent* SplineSupport, FVector& StartLocation,
	                              FVector& EndLocation);
	APoi();
	void Tick(float DeltaTime);
	UFUNCTION()
	virtual void StartAnimation(float RotationSpeed) override;
	UFUNCTION()
	virtual void StopAnimation() override;
	UFUNCTION()
	void OnLookAtTimerTick();
	UFUNCTION()
	void UpdatePitch(float NewPitch);
	UFUNCTION()
	void UpdateLength(float NewLength);
	UFUNCTION()
	void UpdateZAnchor(float NewZ);

	UFUNCTION()
	virtual void UpdatePoi(FRacePOI NewRacePoi, FRaceSetup NewRaceSetup) override;
	UFUNCTION()
	virtual void UpdatePoiWeather(FOpenWeatherResponse WeatherDatas) override;
	UFUNCTION()
	virtual void TogglePoiWeather(bool bShow) override;
	
	UFUNCTION()
	virtual void UpdateDayNight(bool bIsDay) override;
	
	// Orbit
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USpringArmComponent> SpringArmComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCineCameraComponent> CameraComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UOWLCaptureComponent> OwlCapture;
	UPROPERTY(VisibleAnywhere)
	FTimerHandle RotHandle;
	UPROPERTY(EditAnywhere, Category="Orbit")
	float OrbitSpeedDegPerSec = 12.f; // 360/10
	UPROPERTY(EditAnywhere, Category="Orbit")
	bool bOrbitEnabled = false;
	UPROPERTY(EditAnywhere, Category="Orbit")
	float OrbitElapsed = 0.f;
	UPROPERTY(EditAnywhere, Category="Orbit") 
	float StartYaw;
	UPROPERTY(EditAnywhere, Category="Orbit") 
	bool bLookAtEnabled = true;
	UPROPERTY(EditAnywhere, Category="Orbit")
	AActor* OriginalViewTarget = nullptr;
	
	// Broadcast
	void SetBroadcastCaptureEnabled_Implementation(bool bEnabled, class UTextureRenderTarget2D* SharedRT) override;
	
	// Contrôles
	void CameraControl_Fwd_Implementation(float Value) override;
	void CameraControl_SaveFwd_Implementation() override;
	void CameraControl_Pitch_Implementation(float Value) override;
	void CameraControl_SavePitch_Implementation() override;
	void CameraControl_Height_Implementation(float Value) override;
	void CameraControl_SaveHeight_Implementation() override;
	void CameraControl_Reset_Implementation() override;
	void CameraControl_GetLength_Implementation() override;
	void CameraControl_GetPitch_Implementation() override;
	void CameraControl_GetHeight_Implementation() override;
	
	UFUNCTION()
	virtual void UpdateGlobeAnchor();

	UFUNCTION()
	FString CapitalizeFirst(const FString& In);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void FetchWeather();
	
	UFUNCTION()
	void SetMinMax(float MinValue, float MaxValue, int64 RaceID);
	UFUNCTION()
	FMinMax GetMinMax() const;

	UPROPERTY()
	FRacePOI PoiDatas;
	UPROPERTY()
	FOpenWeatherResponse PoiWeatherDatas;
	
	UPROPERTY()
	TObjectPtr<class UPoiSubsystem> PoiSubsystem;
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UPROPERTY()
	TObjectPtr<class UWeatherSubsystem> WeatherSubsystem;
	UPROPERTY()
	TObjectPtr<class USettingsSubsystem> SettingsSubsystem;
	UPROPERTY()
	TObjectPtr<class UScaleSubsystem> ScaleSubsystem;
	UPROPERTY()
	TObjectPtr<class UHttpGatewaySubsystem> HttpGatewaySubsystem;
	UPROPERTY()
	TObjectPtr<class UBroadcastCaptureSubsystem> BroadCastSubsystem;
	
	// Root
	UPROPERTY()
	TObjectPtr<class USceneComponent> MainRoot;
	
	// Archi
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> FootComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USceneComponent> FootHook;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USceneComponent> SplineHook;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USplineComponent> SplineComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USplineMeshComponent> NameBkgComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USplineMeshComponent> InfosBkgComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USplineMeshComponent> WeatherBkgComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USplineMeshComponent> Line1Component;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USplineMeshComponent> Line2Component;
	
	// Infos
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextRenderComponent> NameText;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextRenderComponent> AltText;
	
	// Weather
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextRenderComponent> TempText;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextRenderComponent> Weather1Text;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextRenderComponent> Weather2Text;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTextRenderComponent> Weather3Text;
	
	// Pictos
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> MainPicto;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> AltPicto;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> WeatherPicto;
	
private:
	// GobeAnchor
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UCesiumGlobeAnchorComponent> GlobeAnchorComponent;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TSoftObjectPtr<class ACesiumGeoreference> Georeference;
	//Pawn
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<APawn> DynaPawn;
	// Timer
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	FTimerHandle LookAtTimerHandle;
	UPROPERTY()
	FVector InitLocation;
	UPROPERTY()
	bool bShowWeather = false;
	UPROPERTY()
	FTimerHandle WeatherHandle;
	UPROPERTY()
	FMinMax MinMax;
	UPROPERTY()
	float ArmLength;
	UPROPERTY()
	float Pitch;
	UPROPERTY()
	float Height;
};
