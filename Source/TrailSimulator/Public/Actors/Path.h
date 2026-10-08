// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BroadcastInterface.h"
#include "GameFramework/Actor.h"
#include "TrailSharedTypes.h"
#include "PathInterface.h"
#include "Path.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPathEndDrawing,
	int64,
	RaceID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGeoRefLocation,
	int64,
	RaceID,
	FVector,
	GeorefLocation);

UCLASS()
class TRAILSIMULATOR_API APath : public AActor, public IPathInterface, public IBroadcastInterface
{
	GENERATED_BODY()

public:
	APath();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void BeginPlay() override;
	UFUNCTION()
	TSubclassOf<AActor> GetCheckpointClassForRace(const FRaceSetup& RaceSetup) const;

	UFUNCTION()
	void SetRacePath(const FRacePath& RacePath);
	
	UFUNCTION()
	void StartPulse() const;
	UFUNCTION()
	void StopPulse() const;
	UFUNCTION()
	void UpdatePulseSpeed(float NewSpeed) const;

	UFUNCTION()
	virtual void ChangePathVisibility(bool bShowPath) override;
	UFUNCTION()
	virtual void ChangeSlopeVisibility(bool bShowPath) override;
	UFUNCTION()
	void UpdatePathGlow(float NewGlow, int64 RaceID);
	UFUNCTION()
	void UpdatePulseGlow(float NewPulseGlow) const;

	UFUNCTION()
	void ChangeKmsVisibility(bool bShowKm);
	
	UFUNCTION()
	virtual FVector GetClosestSplineLocation(FVector RunnerLocation) const override;
	UFUNCTION()
	virtual FVector GetLocationAtDistance(float Distance) const override;
	UFUNCTION()
	virtual float GetDistanceAlongSpline(FVector RunnerLocation) const;
	
	UFUNCTION()
	void StartTravelForward();
	UFUNCTION()
	void StartTravelBackward();
	UFUNCTION()
	void StopTravel();
	
	/**
	 * @brief Draws the Path & the Slope of the race
	 */
	void DrawPath(int64 RaceID, FRacePath RacePathDatas);
	FOnPathEndDrawing OnPathEndDrawing;
	FOnGeoRefLocation OnGeoRefLocation;
	
	// Checkpoints
	UFUNCTION()
	void HandleCheckpointsDatasGathered(int64 RaceID, FCheckpoints CheckpointsDatas);
	UPROPERTY()
	int32 NbCheckpoints;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USpringArmComponent> SpringArmComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UCineCameraComponent> CameraComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UOWLCaptureComponent> OwlCapture;
	
	UPROPERTY(EditAnywhere, Category="Orbit")
	AActor* OriginalViewTarget = nullptr;
	
	// Broadcast
	void SetBroadcastCaptureEnabled_Implementation(bool bEnabled, class UTextureRenderTarget2D* SharedRT) override;
	
private:
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class USceneComponent> Root;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class USplineComponent> SplinePath;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class USplineComponent> SlopePath;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class USplineComponent> TravelPath;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UCesiumGlobeAnchorComponent> GlobeAnchorComponent;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TSoftObjectPtr<class ACesiumGeoreference> Georeference;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UStaticMesh> SplineStaticMesh;
	
		
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* TrailHaloMID;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* TrailMID;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* GreenMat;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* GreenHaloMat;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* YellowMat;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* YellowHaloMat;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* OrangeMat;
	UPROPERTY(Transient) 
	UMaterialInstanceDynamic* OrangeHaloMat;
	UPROPERTY(Transient) 
	UMaterialInstanceDynamic* RedMat;
	UPROPERTY(Transient) 
	UMaterialInstanceDynamic* RedHaloMat;
	UPROPERTY(Transient) 
	UMaterialInstanceDynamic* PurpleMat;
	UPROPERTY(Transient) 
	UMaterialInstanceDynamic* PurpleHaloMat;
	UPROPERTY(Transient) 
	UMaterialInstanceDynamic* GreyMat;
	UPROPERTY(Transient) 
	UMaterialInstanceDynamic* GreyHaloMat;
		
	UPROPERTY(meta=(allowPrivateAccess=true))
	float ZOffset;
	
	UPROPERTY()
	TObjectPtr<class ULoadingStatusSubsystem> LoadingSubsystem;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class USettingsSubsystem> SettingsSubsystem;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UCheckpointSubsystem> CheckpointSubsystem;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UWeatherSubsystem> WeatherSubsystem;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UPathSubsystem> PathSubsystem;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UHttpGatewaySubsystem> HttpGatewaySubsystem;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TObjectPtr<class UBroadcastCaptureSubsystem> BroadCastSubsystem;
		
	/** Recalage du trace sur les tuiles Cesium : demande l'altitude des tuiles sous chaque point */
	void StartDrape(int64 RaceID);
	void HandleDrapeHeights(int64 RaceID, int32 Serial, const TArray<FVector>& Query, const TArray<bool>& bSuccess, int32 NumPathPoints, const TArray<int32>& SubSegment, const TArray<float>& SubAlpha);
	/** Construit splines, checkpoints, kms et meshes a partir de RacePath et des hauteurs (m, ellipsoide) */
	void BuildPathGeometry(int64 RaceID, const TArray<double>& HeightsM);
	void DrapeTimedOut(int64 RaceID, int32 Serial);
	class ACesium3DTileset* FindTerrainTileset() const;
	FString GetDrapeCachePath(int64 RaceID, uint32 Hash) const;

	/** Hauteur du centre du tube au-dessus de la surface des tuiles (m) */
	UPROPERTY(EditAnywhere, Category="Drape")
	float DrapeClearanceM = 3.f;
	/** Ecart max entre deux echantillons de relief sur un troncon (m) : sert a detecter les cretes coupees */
	UPROPERTY(EditAnywhere, Category="Drape")
	float DrapeSampleSpacingM = 20.f;
	/** Ecart geoide/ellipsoide utilise si les tuiles ne repondent pas (m, ~50 dans les Alpes) */
	UPROPERTY(EditAnywhere, Category="Drape")
	float DrapeFallbackGeoidM = 50.f;
	UPROPERTY(EditAnywhere, Category="Drape")
	float DrapeTimeoutSeconds = 60.f;
	/** Decalage du trace vers la camera, en fraction de la distance (CPD 17 de M_Glow / M_MasterPC) */
	UPROPERTY(EditAnywhere, Category="Drape")
	float CameraBiasRatio = 0.02f;

	int32 DrapeSerial = 0;
	bool bDrapePending = false;
	FTimerHandle DrapeTimeoutHandle;

	UFUNCTION()
	void RebuildPathSplineMeshes(int64 RaceID);
	UFUNCTION()
	void RebuildSlopeSplineMeshes(int64 RaceID);
	UFUNCTION()
	TSubclassOf<AActor> GetKmClassForRace(const FRaceSetup& RaceSetup) const;
	UFUNCTION()
	bool BuildKms(int64 RaceID);
	
	UPROPERTY()
	TArray<FVector> QueryLLH;	// sampling path points
	UPROPERTY()
	TArray<FVector> KmLLH;	// sampling path points
	
	UPROPERTY()
	int64 RaceId;
	UPROPERTY(meta=(allowPrivateAccess=true))
	FRacePath RacePath;
	UPROPERTY()	
	TArray<TObjectPtr<class AKm>> KmsArray;
	TMap<int64, TArray<TObjectPtr<class AKm>>> Kms;
	UPROPERTY()
	TArray<TObjectPtr<class USplineMeshComponent>> PathSplineMeshes;
	UPROPERTY()
	TArray<TObjectPtr<class USplineMeshComponent>> SlopeSplineMeshes;
	UPROPERTY()
	TArray<FVector> LockedPoints;
	
	UPROPERTY(EditAnywhere)
	float ToleranceMeters = 50.f; // Facteur de simplification
	UPROPERTY(EditAnywhere)
	float SampleStepMeters = 1000.f; // Distance du sample (rater ou pas des virages)
	UPROPERTY(EditAnywhere)
	float SpeedMetersPerSec = 100.f;
	UPROPERTY(EditAnywhere)
	float DistanceCm = 0.f;
	UPROPERTY(EditAnywhere)
	FTimerHandle TravelHandle;
	
	UPROPERTY(EditAnywhere)
	bool bForward = true;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<APawn> DynaPawn;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UCesiumFlyToComponent> FlyComp;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UCesiumOriginShiftComponent> ShiftComp;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<class UMaterialParameterCollection> PulseMPC;
	
};
