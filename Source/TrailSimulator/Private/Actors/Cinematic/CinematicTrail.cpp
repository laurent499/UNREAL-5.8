// All Rights Reserved

#include "Actors/Cinematic/CinematicTrail.h"
#include "Actors/Path.h"
#include "CesiumGeoreference.h"
#include "PathSubsystem.h"
#include "SlateNotificationsBFL.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Interfaces/IHttpResponse.h"
#include "UObject/ConstructorHelpers.h"

ACinematicTrail::ACinematicTrail()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Trail = CreateDefaultSubobject<USplineComponent>(TEXT("Trail"));
	Trail->SetupAttachment(Root);
	Trail->ClearSplinePoints();

	Rail = CreateDefaultSubobject<USplineComponent>(TEXT("Rail"));
	Rail->SetupAttachment(Root);
	Rail->ClearSplinePoints();

	// Meme cylindre (et donc meme materiau) que le trace live
	ConstructorHelpers::FObjectFinder<UStaticMesh> SM(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/SM_Cylindre_1x1.SM_Cylindre_1x1"));
	if (SM.Succeeded())
	{
		TrailMesh = SM.Object;
	}
}

void ACinematicTrail::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Rebuild();
}

void ACinematicTrail::LoadRace()
{
	const FString Endpoint = MainURL / TEXT("race") / LexToString(RaceID) / TEXT("path");
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = UPathSubsystem::CreatePathRequest(Endpoint);
	TWeakObjectPtr<ACinematicTrail> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
		[WeakThis, Endpoint](FHttpRequestPtr, const FHttpResponsePtr Response, bool bWasSuccessful)
		{
			ACinematicTrail* Self = WeakThis.Get();
			if (!Self) return;

			FRacePath Path;
			if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode())
				|| !UPathSubsystem::ParsePathJson(Response->GetContentAsString(), Path) || Path.Points.Num() < 2)
			{
				const FString Msg = FString::Printf(TEXT("Trace cinema : echec du chargement de %s (code %d)"),
					*Endpoint, Response.IsValid() ? Response->GetResponseCode() : -1);
				UE_LOG(LogTemp, Error, TEXT("%s"), *Msg);
				USlateNotificationsBFL::SlateNotify(FText::FromString(Msg), EMessageType::Error);
				return;
			}

			Self->Modify();
			Self->PointsLLH.Reset(Path.Points.Num());
			for (const FRacePathPoint& P : Path.Points)
			{
				Self->PointsLLH.Add(FVector(P.lon, P.lat, P.ele));
			}

			if (Self->bCenterGeoreferenceOnStart)
			{
				if (ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(Self))
				{
					Georef->Modify();
					Georef->SetOriginLongitudeLatitudeHeight(Self->PointsLLH[0]);
				}
			}
			Self->Rebuild();
			USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Trace cinema : course %lld chargee (%d points, %.1f km)"),
				Self->RaceID, Self->PointsLLH.Num(), Self->Trail->GetSplineLength() / 100000.f)), EMessageType::Success);
		});
	Req->ProcessRequest();
}

void ACinematicTrail::Rebuild()
{
	Trail->ClearSplinePoints(false);
	Rail->ClearSplinePoints(false);

	ACesiumGeoreference* Georef = HasTrail() ? ACesiumGeoreference::GetDefaultGeoreference(this) : nullptr;
	if (Georef)
	{
		// Altitude GPS (niveau de la mer) -> hauteur ellipsoidale attendue par Cesium, comme APath
		for (const FVector& LLH : PointsLLH)
		{
			const double HeightM = LLH.Z + APath::GetGeoidUndulationM(LLH.X, LLH.Y) + HeightOffsetM;
			Trail->AddSplinePoint(Georef->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(LLH.X, LLH.Y, HeightM)),
				ESplineCoordinateSpace::World, false);
		}
		Trail->UpdateSpline();
		APath::BuildSmoothCameraRail(Trail, Rail, RailSmoothM);
	}
	else
	{
		Trail->UpdateSpline();
		Rail->UpdateSpline();
	}
	BuildTrailMeshes();
}

void ACinematicTrail::BuildTrailMeshes()
{
	for (USplineMeshComponent* SMC : TrailMeshes)
	{
		if (IsValid(SMC))
		{
			RemoveInstanceComponent(SMC);
			SMC->DestroyComponent();
		}
	}
	TrailMeshes.Reset();
	if (!bShowTrail || !TrailMesh || Trail->GetNumberOfSplinePoints() < 2) return;

	// Morceaux de longueur fixe le long du trace : beaucoup moins de composants qu'un par point GPS
	const float Len = Trail->GetSplineLength();
	const int32 Num = FMath::Max(1, FMath::CeilToInt(Len / (TrailStepM * 100.f)));
	const float Step = Len / Num;
	for (int32 i = 0; i < Num; ++i)
	{
		const float StartDist = Step * i;
		const float EndDist = Step * (i + 1);
		const FVector P1 = Trail->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
		const FVector P2 = Trail->GetLocationAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);
		const FVector T1 = Trail->GetDirectionAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local) * Step;
		const FVector T2 = Trail->GetDirectionAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local) * Step;

		// Composants d'instance, sauvegardes avec le level : le PIE du rendu ne relance pas la construction
		USplineMeshComponent* SMC = NewObject<USplineMeshComponent>(this);
		SMC->CreationMethod = EComponentCreationMethod::Instance;
		AddInstanceComponent(SMC);
		TrailMeshes.Add(SMC);
		SMC->SetMobility(EComponentMobility::Movable);
		SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SMC->SetGenerateOverlapEvents(false);
		SMC->SetStaticMesh(TrailMesh);
		SMC->SetupAttachment(Trail);
		SMC->SetForwardAxis(ESplineMeshAxis::Z, false);
		SMC->SetStartScale(FVector2D(TrailWidth), false);
		SMC->SetEndScale(FVector2D(TrailWidth), false);
		SMC->SetStartAndEnd(P1, T1, P2, T2, false);
		SMC->SetTranslucentSortPriority(-20);

		// Memes donnees que le trace live (APath::RebuildPathSplineMeshes)
		SMC->SetCustomPrimitiveDataFloat(0, StartDist / Len);
		SMC->SetCustomPrimitiveDataFloat(1, Step / Len);
		SMC->SetCustomPrimitiveDataVector3(2, P1);
		SMC->SetCustomPrimitiveDataVector3(6, P2);
		SMC->SetCustomPrimitiveDataFloat(10, 0.f);
		SMC->SetCustomPrimitiveDataFloat(11, 0.f);
		SMC->SetCustomPrimitiveDataVector4(12, TrailColor);
		SMC->SetCustomPrimitiveDataFloat(16, Glow);
		SMC->RegisterComponent();
	}
}
