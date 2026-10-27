// All Rights Reserved

#include "Actors/Cinematic/CinematicGeoAnchor.h"
#include "Actors/Path.h"
#include "Cesium3DTileset.h"
#include "CesiumCartographicPolygon.h"
#include "CesiumGeoreference.h"
#include "CesiumPolygonRasterOverlay.h"
#include "EngineUtils.h"
#include "LidarPointCloud.h"
#include "LidarPointCloudComponent.h"
#include "LidarPointCloudSettings.h"
#include "SlateNotificationsBFL.h"

namespace CinematicGeoAnchor
{
	// Lambert-93 (IGN, ellipsoide GRS80)
	constexpr double N = 0.7256077650532670;
	constexpr double C = 11754255.4261;
	constexpr double Xs = 700000.0;
	constexpr double Ys = 12655612.0499;
	constexpr double Lon0Deg = 3.0;
	constexpr double E = 0.0818191910428158;

	/** Pas des differences finies du calage (cm) */
	constexpr double StepCm = 10000.0;
}

ACinematicGeoAnchor::ACinematicGeoAnchor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

FVector2D ACinematicGeoAnchor::Lambert93ToLonLat(double East, double North)
{
	using namespace CinematicGeoAnchor;
	const double Dx = East - Xs;
	const double Dy = Ys - North;
	const double R = FMath::Sqrt(Dx * Dx + Dy * Dy);
	const double Gamma = FMath::Atan2(Dx, Dy);
	const double Lon = FMath::DegreesToRadians(Lon0Deg) + Gamma / N;
	const double L = -FMath::Loge(R / C) / N;

	// Latitude isometrique -> latitude, par iteration
	double Lat = 2.0 * FMath::Atan(FMath::Exp(L)) - UE_DOUBLE_HALF_PI;
	for (int32 i = 0; i < 10; ++i)
	{
		const double S = E * FMath::Sin(Lat);
		Lat = 2.0 * FMath::Atan(FMath::Pow((1.0 + S) / (1.0 - S), E / 2.0) * FMath::Exp(L)) - UE_DOUBLE_HALF_PI;
	}
	return FVector2D(FMath::RadiansToDegrees(Lon), FMath::RadiansToDegrees(Lat));
}

bool ACinematicGeoAnchor::LocalToLambert(const FVector& Local, FVector& OutLambert) const
{
	if (!IsValid(Target)) return false;

	// Import LiDAR : X est, Y = -nord, Z haut, en unites Unreal (ImportScale) ; point affiche + (origine - decalage) = absolu
	const ULidarPointCloudComponent* CloudComponent = Target->FindComponentByClass<ULidarPointCloudComponent>();
	const ULidarPointCloud* Cloud = CloudComponent ? CloudComponent->GetPointCloud() : nullptr;
	if (Cloud)
	{
		const double Scale = FMath::Max(GetDefault<ULidarPointCloudSettings>()->ImportScale, UE_KINDA_SMALL_NUMBER);
		const FVector Abs = Local + Cloud->OriginalCoordinates - Cloud->LocationOffset;
		OutLambert = FVector(Abs.X / Scale, -Abs.Y / Scale, Abs.Z / Scale);
		return true;
	}
	OutLambert = ManualLambert93Origin + FVector(Local.X, -Local.Y, Local.Z) / 100.0;
	return true;
}

FBox ACinematicGeoAnchor::GetLocalBounds() const
{
	return IsValid(Target) ? Target->CalculateComponentsBoundingBoxInLocalSpace(true) : FBox(ForceInit);
}

FVector ACinematicGeoAnchor::LambertToUnreal(const FVector& Lambert) const
{
	const ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(this);
	const FVector2D LonLat = Lambert93ToLonLat(Lambert.X, Lambert.Y);
	// Altitude IGN69 -> hauteur ellipsoidale, avec le geoide EGM96 du trace
	const double HeightM = Lambert.Z + APath::GetGeoidUndulationM(LonLat.X, LonLat.Y) + HeightOffsetM;
	return Georef->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(LonLat.X, LonLat.Y, HeightM));
}

void ACinematicGeoAnchor::Align()
{
	FVector Lambert;
	if (!LocalToLambert(FVector::ZeroVector, Lambert) || !ACesiumGeoreference::GetDefaultGeoreference(this))
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(TEXT("Calage Lambert-93 : choisir la cible (Target)")), EMessageType::Error);
		return;
	}

	// Transformation locale -> monde par differences finies autour du centre de la cible : position,
	// convergence des meridiens et facteur d'echelle de la projection compris
	const FBox Bounds = GetLocalBounds();
	const FVector P0 = Bounds.IsValid ? Bounds.GetCenter() : FVector::ZeroVector;
	auto World = [this](const FVector& Local)
	{
		FVector L;
		LocalToLambert(Local, L);
		return LambertToUnreal(L);
	};
	const FVector W0 = World(P0);
	const double D = CinematicGeoAnchor::StepCm;
	const FVector AxisX = (World(P0 + FVector(D, 0, 0)) - W0) / D;
	const FVector AxisY = (World(P0 + FVector(0, D, 0)) - W0) / D;
	const FVector AxisZ = (World(P0 + FVector(0, 0, D)) - W0) / D;
	const FVector Origin = W0 - (AxisX * P0.X + AxisY * P0.Y + AxisZ * P0.Z);

	Target->Modify();
	if (USceneComponent* TargetRoot = Target->GetRootComponent())
	{
		TargetRoot->SetMobility(EComponentMobility::Movable);
	}
	Target->SetActorTransform(FTransform(FMatrix(AxisX, AxisY, AxisZ, Origin)));

	LocalToLambert(P0, Lambert);
	const FVector2D LonLat = Lambert93ToLonLat(Lambert.X, Lambert.Y);
	Modify();
	CenterLonLatHeight = FVector(LonLat.X, LonLat.Y, Lambert.Z + APath::GetGeoidUndulationM(LonLat.X, LonLat.Y) + HeightOffsetM);
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Calage Lambert-93 : %s cale (%.6f, %.6f, %.1f m)"),
		*Target->GetActorNameOrLabel(), CenterLonLatHeight.X, CenterLonLatHeight.Y, CenterLonLatHeight.Z)), EMessageType::Success);
}

void ACinematicGeoAnchor::ClipCesium()
{
	const FBox Bounds = GetLocalBounds();
	if (!Bounds.IsValid)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(TEXT("Calage Lambert-93 : choisir la cible (Target)")), EMessageType::Error);
		return;
	}
	if (!Tileset)
	{
		TActorIterator<ACesium3DTileset> It(GetWorld());
		Tileset = It ? *It : nullptr;
	}
	if (!Tileset) return;

	// Emprise de la cible, agrandie de la marge, en longitude / latitude
	const FVector M(ClipMarginM * 100.0, ClipMarginM * 100.0, 0.0);
	const FVector Min = Bounds.Min - M;
	const FVector Max = Bounds.Max + M;
	TArray<FVector> Points;
	for (const FVector& Corner : { FVector(Min.X, Min.Y, Min.Z), FVector(Max.X, Min.Y, Min.Z), FVector(Max.X, Max.Y, Min.Z), FVector(Min.X, Max.Y, Min.Z) })
	{
		FVector Lambert;
		LocalToLambert(Corner, Lambert);
		const FVector2D LonLat = Lambert93ToLonLat(Lambert.X, Lambert.Y);
		Points.Add(FVector(LonLat.X, LonLat.Y, Lambert.Z));
	}

	Modify();
	if (!ClipPolygon)
	{
		FActorSpawnParameters Params;
		Params.Name = MakeUniqueObjectName(GetLevel(), ACesiumCartographicPolygon::StaticClass(), TEXT("DecoupeCesium"));
		ClipPolygon = GetWorld()->SpawnActor<ACesiumCartographicPolygon>(Params);
	}
	ClipPolygon->Modify();
	ClipPolygon->SetPolygonPoints(ECesiumCoordinateReferenceSystem::LongitudeLatitudeHeight, Points);

	// Un overlay de decoupe par tileset suffit : on y ajoute le polygone s'il n'y est pas
	TArray<UCesiumPolygonRasterOverlay*> Overlays;
	Tileset->GetComponents(Overlays);
	UCesiumPolygonRasterOverlay* Overlay = Overlays.Num() > 0 ? Overlays[0] : nullptr;
	Tileset->Modify();
	if (!Overlay)
	{
		Overlay = NewObject<UCesiumPolygonRasterOverlay>(Tileset, TEXT("DecoupeCesium"));
		Overlay->CreationMethod = EComponentCreationMethod::Instance;
		Tileset->AddInstanceComponent(Overlay);
		Overlay->RegisterComponent();
	}
	Overlay->Modify();
	Overlay->Polygons.AddUnique(TSoftObjectPtr<ACesiumCartographicPolygon>(ClipPolygon.Get()));
	Tileset->RefreshTileset();
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Calage Lambert-93 : tuiles Cesium masquees sous %s"),
		*Target->GetActorNameOrLabel())), EMessageType::Success);
}
