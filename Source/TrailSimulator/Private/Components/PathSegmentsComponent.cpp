// All Rights Reserved

#include "Components/PathSegmentsComponent.h"
#include "Actors/Path.h"
#include "Components/RawTextMaterial.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "RaceManager.h"
#include "RaceSubsystem.h"
#include "SettingsSubsystem.h"
#include "TimerManager.h"

namespace PathSegments
{
	/** Plus petit morceau cree par une coupure (cm) : une borne a moins de 1 m d'un point GPS s'y accroche */
	constexpr float MinPieceCm = 100.f;
}

UPathSegmentsComponent::UPathSegmentsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Fond du nom : cube aplati avec M_Plain, le master des fonds de nom (couleur en Custom Primitive Data)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded()) PlateMesh = CubeFinder.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlainFinder(TEXT("/Game/LTVContent/Materials/Masters/M_Plain.M_Plain"));
	if (PlainFinder.Succeeded()) PlateMaterial = PlainFinder.Object;
}

void UPathSegmentsComponent::BeginPlay()
{
	Super::BeginPlay();
	if (ARaceManager* RaceManager = Cast<ARaceManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass())))
	{
		RaceManager->OnChangeDayNight.AddUniqueDynamic(this, &UPathSegmentsComponent::UpdateDayNight);
	}
}

void UPathSegmentsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LookAtTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UPathSegmentsComponent::SetSegments(const TArray<FPathSegment>& InSegments)
{
	Segments = InSegments;
	Apply();
}

void UPathSegmentsComponent::Refresh()
{
	Apply();
}

void UPathSegmentsComponent::OnPathVisibilityChanged(bool bShowPath)
{
	// APath vient de tout masquer ou afficher : les segments visibles restent affiches,
	// le trace recouvert par un segment reste masque
	for (USplineMeshComponent* Piece : HighlightedPieces)
	{
		if (Piece) Piece->SetHiddenInGame(false);
	}
	for (USplineMeshComponent* Piece : OverlappedPieces)
	{
		if (Piece) Piece->SetHiddenInGame(true);
	}
}

/**
 * Remet le trace dans son etat d'origine : morceaux ajoutes detruits, morceaux coupes
 * reallonges, couleur de la course, visibilite du trace. Les noms sont retires.
 */
void UPathSegmentsComponent::RestoreTrace()
{
	APath* Path = Cast<APath>(GetOwner());
	if (!Path) return;

	for (USplineMeshComponent* Piece : OverlappedPieces)
	{
		if (IsValid(Piece)) Piece->SetHiddenInGame(!Path->bPathShown);
	}
	OverlappedPieces.Reset();

	for (USplineMeshComponent* Piece : ExtraPieces)
	{
		if (!Piece) continue;
		Path->PathSplineMeshes.Remove(Piece);
		Piece->DestroyComponent();
	}
	ExtraPieces.Reset();

	for (const int32 Index : SplitIndices)
	{
		if (Path->PathSplineMeshes.IsValidIndex(Index) && Path->PathSplineMeshes[Index])
		{
			ConfigurePiece(Path->PathSplineMeshes[Index], Index, Index + 1);
		}
	}
	SplitIndices.Reset();

	if (HighlightedPieces.Num() > 0 && Path->RaceSubsystem)
	{
		const FLinearColor RaceColor = FLinearColor::FromSRGBColor(FColor::FromHex(Path->RaceSubsystem->GetRaceSetupById(Path->RaceId).color));
		for (int32 i = 0; i < HighlightedPieces.Num(); ++i)
		{
			USplineMeshComponent* Piece = HighlightedPieces[i];
			if (!Piece || !IsValid(Piece)) continue;
			Piece->SetCustomPrimitiveDataVector4(12, RaceColor);
			Piece->SetHiddenInGame(!Path->bPathShown);
			if (HighlightedBaseScales.IsValidIndex(i))
			{
				Piece->SetStartScale(HighlightedBaseScales[i].Key, false);
				Piece->SetEndScale(HighlightedBaseScales[i].Value, true);
			}
		}
	}
	HighlightedPieces.Reset();
	HighlightedBaseScales.Reset();

	for (UStaticMeshComponent* Plate : Plates)
	{
		if (Plate) Plate->DestroyComponent();
	}
	Plates.Reset();
	for (UTextRenderComponent* Label : Labels)
	{
		if (Label) Label->DestroyComponent();
	}
	Labels.Reset();
}

/** Place un morceau du trace entre deux cles de la spline (meme calcul que APath::RebuildPathSplineMeshes) */
void UPathSegmentsComponent::ConfigurePiece(USplineMeshComponent* Piece, float Key0, float Key1) const
{
	const APath* Path = Cast<APath>(GetOwner());
	if (!Piece || !Path || !Path->SplinePath) return;
	const USplineComponent* Spline = Path->SplinePath;

	// Tangentes de la spline (par cle) ramenees a la portion [Key0, Key1]
	const float Span = Key1 - Key0;
	const FVector P0 = Spline->GetLocationAtSplineInputKey(Key0, ESplineCoordinateSpace::Local);
	const FVector P1 = Spline->GetLocationAtSplineInputKey(Key1, ESplineCoordinateSpace::Local);
	const FVector T0 = Spline->GetTangentAtSplineInputKey(Key0, ESplineCoordinateSpace::Local) * Span;
	const FVector T1 = Spline->GetTangentAtSplineInputKey(Key1, ESplineCoordinateSpace::Local) * Span;
	Piece->SetStartAndEnd(P0, T0, P1, T1, true);

	const float TotalLen = FMath::Max(Spline->GetSplineLength(), 1.f);
	const float D0 = Spline->GetDistanceAlongSplineAtSplineInputKey(Key0);
	const float D1 = Spline->GetDistanceAlongSplineAtSplineInputKey(Key1);
	Piece->SetCustomPrimitiveDataFloat(0, D0 / TotalLen);			// StartNorm (pulse)
	Piece->SetCustomPrimitiveDataFloat(1, (D1 - D0) / TotalLen);	// LenNorm
	Piece->SetCustomPrimitiveDataVector3(2, P0);					// M_Glow
	Piece->SetCustomPrimitiveDataVector3(6, P1);
}

/** Copie d'un morceau du trace (meme maillage, materiau, donnees : couleur, glow...) */
USplineMeshComponent* UPathSegmentsComponent::CreatePiece(USplineMeshComponent* Original)
{
	APath* Path = Cast<APath>(GetOwner());
	if (!Path || !Original) return nullptr;

	USplineMeshComponent* Piece = NewObject<USplineMeshComponent>(Path);
	Piece->CreationMethod = EComponentCreationMethod::Instance;
	Path->AddInstanceComponent(Piece);
	Piece->SetComponentTickEnabled(false);
	Piece->SetGenerateOverlapEvents(false);
	Piece->SetMobility(EComponentMobility::Movable);
	Piece->SetStaticMesh(Original->GetStaticMesh());
	Piece->SetupAttachment(Original->GetAttachParent());
	Piece->SetTranslucentSortPriority(Original->TranslucencySortPriority);
	Piece->RegisterComponent();
	for (int32 i = 0; i < Original->GetNumMaterials(); ++i)
	{
		Piece->SetMaterial(i, Original->GetMaterial(i));
	}
	Piece->SetForwardAxis(Original->GetForwardAxis());
	Piece->SetStartScale(Original->GetStartScale());
	Piece->SetEndScale(Original->GetEndScale());
	const TArray<float>& Data = Original->GetCustomPrimitiveData().Data;
	for (int32 i = 0; i < Data.Num(); ++i)
	{
		Piece->SetCustomPrimitiveDataFloat(i, Data[i]);
	}
	Piece->SetHiddenInGame(Original->bHiddenInGame);

	ExtraPieces.Add(Piece);
	// Dans la liste du trace : le glow, le dosage jour/nuit et l'affichage du trace s'y appliquent
	Path->PathSplineMeshes.Add(Piece);
	return Piece;
}

void UPathSegmentsComponent::Apply()
{
	APath* Path = Cast<APath>(GetOwner());
	if (!Path || !Path->SplinePath) return;

	RestoreTrace();

	USplineComponent* Spline = Path->SplinePath;
	const int32 NumOriginal = Spline->GetNumberOfSplinePoints() - 1;
	if (NumOriginal < 1 || Path->PathSplineMeshes.Num() < NumOriginal) return; // trace pas encore construit

	struct FRange { float D0; float D1; FLinearColor Color; FString Label; };
	TArray<FRange> Ranges;
	const float TotalLen = Spline->GetSplineLength();
	// Borne posee sur un checkpoint : distance du point du trace qui le porte (il a pu etre deplace),
	// sinon le km recu de la regie
	auto BoundCm = [Path, Spline](int64 CheckpointID, float Km)
	{
		if (CheckpointID != 0)
		{
			const int32 Index = Path->RacePath.Points.IndexOfByPredicate([CheckpointID](const FRacePathPoint& Point)
			{
				return !Point.datas.name.IsEmpty() && Point.datas.checkpointId == CheckpointID;
			});
			if (Index != INDEX_NONE && Index < Spline->GetNumberOfSplinePoints())
			{
				return Spline->GetDistanceAlongSplineAtSplinePoint(Index);
			}
		}
		return Km * 100000.f;
	};
	for (const FPathSegment& Segment : Segments)
	{
		const float D0 = FMath::Clamp(BoundCm(Segment.CheckpointIn, Segment.KmIn), 0.f, TotalLen);
		const float D1 = FMath::Clamp(BoundCm(Segment.CheckpointOut, Segment.KmOut), 0.f, TotalLen);
		if (!Segment.bVisible || D1 - D0 < PathSegments::MinPieceCm) continue;
		const FColor Color = FColor::FromHex(Segment.Color);
		Ranges.Add({ D0, D1, FLinearColor::FromSRGBColor(Color), Segment.Label });
	}

	if (Ranges.Num() > 0)
	{
		const FLinearColor RaceColor = Path->RaceSubsystem
			? FLinearColor::FromSRGBColor(FColor::FromHex(Path->RaceSubsystem->GetRaceSetupById(Path->RaceId).color))
			: FLinearColor::White;
		TArray<float> Cuts;
		for (const FRange& Range : Ranges)
		{
			Cuts.Add(Range.D0);
			Cuts.Add(Range.D1);
		}
		// Morceaux restes de la couleur de la course, avec leur portion du trace (cm le long de la spline)
		struct FPlainPiece { USplineMeshComponent* Piece; float D0; float D1; };
		TArray<FPlainPiece> PlainPieces;

		for (int32 i = 0; i < NumOriginal; ++i)
		{
			USplineMeshComponent* Original = Path->PathSplineMeshes[i];
			if (!Original) continue;
			const float D0 = Spline->GetDistanceAlongSplineAtSplinePoint(i);
			const float D1 = Spline->GetDistanceAlongSplineAtSplinePoint(i + 1);

			TArray<float> Bounds;
			Bounds.Add(D0);
			for (const float Cut : Cuts)
			{
				if (Cut > D0 + PathSegments::MinPieceCm && Cut < D1 - PathSegments::MinPieceCm) Bounds.AddUnique(Cut);
			}
			Bounds.Add(D1);
			Bounds.Sort();
			const bool bSplit = Bounds.Num() > 2;
			if (bSplit) SplitIndices.Add(i);

			for (int32 p = 0; p < Bounds.Num() - 1; ++p)
			{
				const float Mid = 0.5f * (Bounds[p] + Bounds[p + 1]);
				const FRange* Match = nullptr;
				for (const FRange& Range : Ranges)
				{
					if (Mid >= Range.D0 && Mid <= Range.D1) Match = &Range; // le dernier segment l'emporte
				}
				if (!bSplit && !Match)
				{
					PlainPieces.Add({ Original, D0, D1 });
					break; // morceau entier hors segment : on n'y touche pas
				}

				USplineMeshComponent* Piece = p == 0 ? Original : CreatePiece(Original);
				if (!Piece) continue;
				if (bSplit)
				{
					const float K0 = p == 0 ? float(i) : Spline->GetInputKeyValueAtDistanceAlongSpline(Bounds[p]);
					const float K1 = p == Bounds.Num() - 2 ? float(i + 1) : Spline->GetInputKeyValueAtDistanceAlongSpline(Bounds[p + 1]);
					ConfigurePiece(Piece, K0, K1);
				}
				if (Match)
				{
					Piece->SetCustomPrimitiveDataVector4(12, Match->Color);
					Piece->SetHiddenInGame(false);
					HighlightedPieces.Add(Piece);
				}
				else
				{
					// Une copie faite par CreatePiece herite de la couleur du morceau d'origine,
					// deja recolore s'il est dans le segment : on lui remet la couleur de la course
					Piece->SetCustomPrimitiveDataVector4(12, RaceColor);
					Piece->SetHiddenInGame(!Path->bPathShown);
					PlainPieces.Add({ Piece, Bounds[p], Bounds[p + 1] });
				}
			}
		}

		// Trace qui repasse au meme endroit (aller-retour) : les morceaux de l'autre passage qui
		// recouvrent un segment visible sont masques, sinon les deux traces se chevauchent et clignotent
		if (OverlapRadius > 0.f)
		{
			constexpr float StepCm = 500.f;
			auto CellOf = [this](const FVector& P)
			{
				return FIntVector(FMath::FloorToInt32(P.X / OverlapRadius), FMath::FloorToInt32(P.Y / OverlapRadius), FMath::FloorToInt32(P.Z / OverlapRadius));
			};
			// Points des segments tous les 5 m, ranges par cases de la taille du rayon
			TMap<FIntVector, TArray<FVector>> Grid;
			for (const FRange& Range : Ranges)
			{
				for (float D = Range.D0; D <= Range.D1; D += StepCm)
				{
					const FVector P = Spline->GetLocationAtDistanceAlongSpline(D, ESplineCoordinateSpace::World);
					Grid.FindOrAdd(CellOf(P)).Add(P);
				}
			}
			const float RadiusSq = FMath::Square(OverlapRadius);
			auto IsCovered = [&Grid, &CellOf, RadiusSq](const FVector& P)
			{
				const FIntVector Cell = CellOf(P);
				for (int32 X = -1; X <= 1; ++X)
				for (int32 Y = -1; Y <= 1; ++Y)
				for (int32 Z = -1; Z <= 1; ++Z)
				{
					if (const TArray<FVector>* Samples = Grid.Find(Cell + FIntVector(X, Y, Z)))
					{
						for (const FVector& S : *Samples)
						{
							if (FVector::DistSquared(S, P) <= RadiusSq) return true;
						}
					}
				}
				return false;
			};
			for (const FPlainPiece& Plain : PlainPieces)
			{
				// Trace juste avant ou apres un segment (meme passage) : il le prolonge, on le garde
				bool bAdjacent = false;
				for (const FRange& Range : Ranges)
				{
					if (Plain.D1 > Range.D0 - 2.f * OverlapRadius && Plain.D0 < Range.D1 + 2.f * OverlapRadius) bAdjacent = true;
				}
				if (bAdjacent) continue;

				// Masque seulement si tout le morceau est sur le segment : pas de trou la ou les passages se separent
				const int32 NumSamples = FMath::Max(2, FMath::CeilToInt((Plain.D1 - Plain.D0) / StepCm) + 1);
				bool bCovered = true;
				for (int32 s = 0; s < NumSamples && bCovered; ++s)
				{
					const float D = FMath::Lerp(Plain.D0, Plain.D1, float(s) / float(NumSamples - 1));
					bCovered = IsCovered(Spline->GetLocationAtDistanceAlongSpline(D, ESplineCoordinateSpace::World));
				}
				if (bCovered)
				{
					Plain.Piece->SetHiddenInGame(true);
					OverlappedPieces.Add(Plain.Piece);
				}
			}
		}

		// Trace epaissi dans les segments. Apres la boucle de coupure : CreatePiece copie
		// l'echelle du morceau d'origine, qui doit encore etre l'echelle normale
		for (USplineMeshComponent* Piece : HighlightedPieces)
		{
			const FVector2D Start = Piece->GetStartScale();
			const FVector2D End = Piece->GetEndScale();
			HighlightedBaseScales.Add({ Start, End });
			Piece->SetStartScale(Start * HighlightThickness, false);
			Piece->SetEndScale(End * HighlightThickness, true);
		}

		// Noms au-dessus du milieu de chaque segment
		for (const FRange& Range : Ranges)
		{
			if (Range.Label.IsEmpty()) continue;
			UTextRenderComponent* Label = NewObject<UTextRenderComponent>(Path);
			Label->CreationMethod = EComponentCreationMethod::Instance;
			Path->AddInstanceComponent(Label);
			Label->SetComponentTickEnabled(false);
			Label->SetGenerateOverlapEvents(false);
			Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Label->SetCastShadow(false);
			Label->SetMobility(EComponentMobility::Movable);
			Label->SetupAttachment(Path->GetRootComponent());
			Label->SetHorizontalAlignment(EHTA_Center);
			Label->SetVerticalAlignment(EVRTA_TextCenter);
			Label->SetWorldSize(LabelWorldSize);
			Label->SetText(FText::FromString(Range.Label));
			Label->RegisterComponent();
			TrailRawText::Apply(Label, FColor::White);
			Label->SetScalarParameterForCustomPrimitiveData("Illum", LabelIllum);

			// Fond de la couleur du segment derriere le texte (le texte regarde +X)
			const FVector TextSize = Label->GetTextLocalSize();
			const float TextWidth = TextSize.Y > 1.f ? TextSize.Y : LabelWorldSize * 0.6f * Range.Label.Len();
			const float TextHeight = TextSize.Z > 1.f ? TextSize.Z : LabelWorldSize;
			const float Pad = LabelWorldSize * LabelPadding;
			if (PlateMesh)
			{
				UStaticMeshComponent* Plate = NewObject<UStaticMeshComponent>(Path);
				Plate->CreationMethod = EComponentCreationMethod::Instance;
				Path->AddInstanceComponent(Plate);
				Plate->SetComponentTickEnabled(false);
				Plate->SetGenerateOverlapEvents(false);
				Plate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Plate->SetCastShadow(false);
				Plate->SetMobility(EComponentMobility::Movable);
				Plate->SetStaticMesh(PlateMesh);
				Plate->SetupAttachment(Label);
				Plate->RegisterComponent();
				if (PlateMaterial) Plate->SetMaterial(0, PlateMaterial);
				Plate->SetCustomPrimitiveDataVector4(0, Range.Color);	// Couleur
				Plate->SetCustomPrimitiveDataFloat(4, 0.5f);				// Illum
				Plate->SetCustomPrimitiveDataFloat(5, 1.f);				// Alpha
				// Cube de 100 cm : 4 cm d'epaisseur, juste derriere le texte
				Plate->SetRelativeLocation(FVector(-10.f, 0.f, 0.f));
				Plate->SetRelativeScale3D(FVector(0.04f, (TextWidth + 2.f * Pad) / 100.f, (TextHeight + Pad) / 100.f));
				Plates.Add(Plate);
			}
			const FVector Anchor = Spline->GetLocationAtDistanceAlongSpline(0.5f * (Range.D0 + Range.D1), ESplineCoordinateSpace::World);
			Label->SetWorldLocation(Anchor + FVector::UpVector * (LabelHeight + 0.5f * (TextHeight + Pad)));
			Labels.Add(Label);
		}
	}

	UWorld* World = GetWorld();
	if (!World) return;
	if (Labels.Num() > 0)
	{
		if (!World->GetTimerManager().IsTimerActive(LookAtTimerHandle))
		{
			const float Interval = Path->SettingsSubsystem ? Path->SettingsSubsystem->GetUpdateIntervalSeconds() : 0.25f;
			World->GetTimerManager().SetTimer(LookAtTimerHandle, this, &UPathSegmentsComponent::OnLookAtTimerTick, FMath::Max(Interval, 0.05f), true, 0.f);
		}
	}
	else
	{
		World->GetTimerManager().ClearTimer(LookAtTimerHandle);
	}
}

/** Les noms se tournent vers la camera (lacet seulement), comme les bornes km */
void UPathSegmentsComponent::OnLookAtTimerTick()
{
	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC || !PC->PlayerCameraManager) return;
	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	for (UTextRenderComponent* Label : Labels)
	{
		if (!Label) continue;
		FVector ToCamera = CameraLocation - Label->GetComponentLocation();
		ToCamera.Z = 0.f;
		if (ToCamera.IsNearlyZero()) continue;
		Label->SetWorldRotation(FRotator(0.f, ToCamera.Rotation().Yaw, 0.f));
	}
}

void UPathSegmentsComponent::UpdateDayNight(bool bIsDay)
{
	LabelIllum = bIsDay ? 0.125f : 0.25f;
	for (UTextRenderComponent* Label : Labels)
	{
		if (Label) Label->SetCustomPrimitiveDataFloat(0, LabelIllum);
	}
}
