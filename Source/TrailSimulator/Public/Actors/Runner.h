// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BroadcastInterface.h"
#include "CameraControlInterface.h"
#include "RunnerInterface.h"
#include "RunnerStackableInterface.h"
#include "ScaleInterface.h"
#include "GameFramework/Actor.h"
#include "TrailSharedTypes.h"
#include "Runner.generated.h"

struct FRunnerState
{
	TWeakObjectPtr<ARunner> Runner;
	FVector CurrentOffset = FVector::ZeroVector;
	FVector CurrentScale  = FVector(1.f);
};

/**
 * @brief Class virtuelle de Runner
 */
UCLASS()
class TRAILSIMULATOR_API ARunner :	public AActor, 
									public IRunnerInterface,
									public IScaleInterface,
									public IRunnerStackableInterface,
									public IBroadcastInterface,
									public ICameraControlInterface
{
	GENERATED_BODY()

public:
	ARunner();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	virtual void StartAnimation(float RotationSpeed) override;
	UFUNCTION()
	virtual void StopAnimation() override;
	UFUNCTION()
	virtual void MarkDirty() override;
	UFUNCTION()
	virtual void UpdateDayNight(bool bIsDay) override;
		
	// Scale
	UFUNCTION()
	void SetMinMax(float MinValue, float MaxValue, int64 RaceID);
	UFUNCTION()
	virtual FMinMax GetRunnerMinMax() const override;
	UFUNCTION()
	virtual float GetRunnerVDelta() const override;
	
	// Stacked/Unstacked
	UFUNCTION()
	virtual USceneComponent* GetStackHookComponent() const override;
	UFUNCTION()
	virtual USceneComponent* GetStackAttachComponent() const override;
	UFUNCTION()
	virtual void SetIsStacked(bool bInStacked) override;
	UFUNCTION()
	virtual bool IsStacked() const override;
	UPROPERTY()
	bool bStackedState;
	UPROPERTY()
	bool bHasStackChildren;
	UPROPERTY()
	bool bShowPhoto = true;
	UPROPERTY()
	bool bShowClub = true;
	UPROPERTY()
	bool bShowFlag = true;
	
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
	
	UPROPERTY()
	TObjectPtr<class USettingsSubsystem> SettingsSubsystem;
	UPROPERTY()
	TObjectPtr<class URunnerSubsystem> RunnerSubsystem;
	UPROPERTY()
	TObjectPtr<class URunnerStackingSubsystem> StackingSubsystem;
	UPROPERTY()
	TObjectPtr<class UPathSubsystem> PathSubsystem;
	UPROPERTY()
	TObjectPtr<class UBroadcastCaptureSubsystem> BroadCastSubsystem;
	
	UFUNCTION(BlueprintCallable)
	virtual void AssignRunnerToTeam(int64 IdTeam) override;
	UFUNCTION(BlueprintCallable)
	virtual void UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup) override;
	UFUNCTION()
	virtual void ToggleClub(bool bDisplay) override;
	UFUNCTION()
	virtual void ToggleFlag(bool bDisplay) override;
	UFUNCTION()
	virtual void TogglePhoto(bool bDisplay) override;
	UFUNCTION()
	virtual bool IsPhotoVisible() override;
	UFUNCTION()
	virtual bool IsClubVisible() override;
	UFUNCTION()
	virtual bool IsFlagVisible() override;
	UFUNCTION()
	virtual void TriggerUpdateAfterHidden(int64 RunnerID) override;
	virtual void UpdateRunnerLocation(FRunnerStruct RunnerStruct, TObjectPtr<class APath> CurrentPath) override;

	// Interpolation le long du trace entre deux snapshots (avancee par ARaceManager::Tick)
	bool IsTrackInterpActive() const { return bTrackInterpActive; }
	// Avance l'interpolation ; renvoie false quand elle est terminee
	bool AdvanceTrackInterp(float DeltaTime);

	FVector CurrentLocation;
	
	virtual USceneComponent* GetFootHook() const override { return FootHook; }
	virtual UStaticMeshComponent* GetFootMesh() const override { return FootComponent; }
	
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

protected:
	void OnLookAtTimerTick();
	
	UFUNCTION()
	void UpdatePitch(float NewPitch);
	UFUNCTION()
	void UpdateLength(float NewLength);
	UFUNCTION()
	void UpdateZAnchor(float NewZ);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	bool bSnapToPath;
	
	FRunnerStruct RunnerDatas;
	
	//Components
	// GobeAnchor
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UCesiumGlobeAnchorComponent> GlobeAnchorComponent;
	UPROPERTY(meta=(allowPrivateAccess=true))
	TSoftObjectPtr<class ACesiumGeoreference> Georeference;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> MainRootComponent;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> PresentationRoot;
	
	// Foot
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> FootComponent;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> FootHook;
	
	// Slash
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> SlashHook;
	
	// Name Hook
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class USceneComponent> NameHook;
	
	UPROPERTY()
	TObjectPtr<class URaceSubsystem> RaceSubsystem;
	UPROPERTY()
	TObjectPtr<class UScaleSubsystem> ScaleSubsystem;
	UPROPERTY()
	TObjectPtr<class UHttpGatewaySubsystem> HttpGatewaySubsystem;
	
private:
	// Variables
	UPROPERTY(meta=(AllowPrivateAccess=true))
	EStackState StackState;
	UPROPERTY(meta=(AllowPrivateAccess=true))
	bool ActiveState;
	UPROPERTY(meta=(AllowPrivateAccess=true))
	int64 IdTeam;
	UPROPERTY(meta=(AllowPrivateAccess=true))
	FMinMax MinMax;
	UPROPERTY()
	float ArmLength;
	UPROPERTY()
	float Pitch;
	UPROPERTY()
	float Height;

	// Lecture differee le long du trace : on rejoue les positions recues avec 2 intervalles de retard,
	// pour avoir toujours un point d'avance et un deplacement continu (distances en cm sur la spline)
	struct FTrackSample
	{
		double Time = 0.0;
		float Dist = 0.f;
	};
	TArray<FTrackSample, TInlineAllocator<6>> TrackSamples;
	TWeakObjectPtr<class APath> TrackInterpPath;
	double TrackPlayTime = 0.0;
	float TrackAvgInterval = 0.f;
	float TrackDisplayedDist = 0.f;
	bool bTrackInterpActive = false;
	bool bHasTrackDist = false;

	//Pawn
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<APawn> DynaPawn;
	
	// Timer
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	FTimerHandle LookAtTimerHandle;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	FTimerHandle UpdateTimerHandle;
};
