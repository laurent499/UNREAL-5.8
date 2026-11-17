#include "RunnerStackingSubsystem.h"
#include "RunnerInterface.h"
#include "RunnerStackableInterface.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"
#include "CheckpointInterface.h"
#include "EngineUtils.h"
#include "SceneView.h"
#include "UnrealClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

// Ecart maximal le long du trace pour empiler deux runners. Sans ce garde-fou, seule la
// distance 3D comptait : deux coureurs a des km l'un de l'autre sur le parcours, mais proches a vol
// d'oiseau (lacets, aller-retour), etaient empiles.
static TAutoConsoleVariable<float> CVarStackingMaxTrackGapM(
	TEXT("Trail.Stacking.MaxTrackGapM"),
	1000.f,
	TEXT("Ecart maximal (m) le long du trace pour empiler deux runners, camera loin ; reduit avec la distance camera jusqu a 0 camera proche. 0 = pas de limite."),
	ECVF_Default);

// Distance (en hauteurs de runner : mat + etiquette, a l'echelle courante) en dessous de laquelle deux
// runners sont toujours empiles, quels que soient le rayon et l'ecart sur le trace. Sans ce plancher,
// camera proche (ecart autorise 0, rayon 1 cm), deux coureurs au meme endroit dessinaient leurs
// etiquettes l'une sur l'autre.
static TAutoConsoleVariable<float> CVarStackingOverlapFactor(
	TEXT("Trail.Stacking.OverlapFactor"),
	1.f,
	TEXT("Distance minimale d'empilement, en hauteurs de runner a l'echelle courante. 0 = desactive."),
	ECVF_Default);

// Evitement des panneaux checkpoint par les cartes des runners
static TAutoConsoleVariable<int32> CVarCardAvoidEnable(
	TEXT("Trail.CardAvoid.Enable"),
	1,
	TEXT("1 = la carte d'un runner qui recouvre un panneau checkpoint a l'ecran monte au-dessus (sinon descend dessous). 0 = desactive."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarCardAvoidMarginPx(
	TEXT("Trail.CardAvoid.MarginPx"),
	8.f,
	TEXT("Ecart (pixels) laisse entre la carte d'un runner et le panneau checkpoint qu'elle evite."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarCardAvoidSpeed(
	TEXT("Trail.CardAvoid.Speed"),
	6.f,
	TEXT("Vitesse du glissement de la carte (interpolation par image, plus grand = plus rapide)."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarCardAvoidDebug(
	TEXT("Trail.CardAvoid.Debug"),
	0,
	TEXT("1 = dessine les boites testees (cyan : panneaux checkpoint, jaune : cartes, rouge : cartes decalees) et journalise les cartes decalees. 2 = journalise aussi les composants de chaque boite."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarCardAvoidMinFoot(
	TEXT("Trail.CardAvoid.MinFoot"),
	0.25f,
	TEXT("En descendant, part minimale du pied d'origine gardee sous la carte (0..1)."),
	ECVF_Default);

namespace RunnerCardAvoid
{
	/** Union des primitives visibles sous Ref (pile comprise), dans le repere de Ref. */
	/** Visible en jeu, parents compris jusqu'a Ref (un parent masque masque ses enfants). */
	static bool IsShownUnder(const USceneComponent* C, const USceneComponent* Ref)
	{
		for (const USceneComponent* It = C; It && It != Ref; It = It->GetAttachParent())
		{
			if (!It->IsVisible() || It->bHiddenInGame) return false;
		}
		return true;
	}

	/** Union des primitives visibles sous Ref (pile comprise), dans le repere de Ref. */
	static FBox ComputeLocalBox(const USceneComponent* Ref)
	{
		FBox Box(ForceInit);
		if (!IsValid(Ref)) return Box;
		const bool bLog = CVarCardAvoidDebug.GetValueOnGameThread() >= 2;
		const FTransform RefTransform = Ref->GetComponentTransform();
		TArray<USceneComponent*> Children;
		Ref->GetChildrenComponents(true, Children);
		for (const USceneComponent* C : Children)
		{
			const UPrimitiveComponent* P = Cast<UPrimitiveComponent>(C);
			if (!P || !P->IsRegistered() || P->IsEditorOnly() || P->IsVisualizationComponent()) continue;
			if (!IsShownUnder(P, Ref)) continue;
			const AActor* Owner = P->GetOwner();
			if (Owner && Owner->IsHidden()) continue;
			const FBox B = P->CalcBounds(P->GetComponentTransform().GetRelativeTransform(RefTransform)).GetBox();
			if (bLog)
			{
				UE_LOG(LogTemp, Log, TEXT("[CardAvoid]   %s.%s Z %.0f..%.0f Y %.0f..%.0f"),
					*GetNameSafe(Owner), *P->GetName(), B.Min.Z, B.Max.Z, B.Min.Y, B.Max.Y);
			}
			Box += B;
		}
		return Box;
	}

	static void DrawBox(UWorld* World, const FBox& LocalBox, const FTransform& Ref, const FVector& Offset, const FColor& Color)
	{
		if (!LocalBox.IsValid) return;
		DrawDebugBox(World, Ref.TransformPosition(LocalBox.GetCenter()) + Offset,
			LocalBox.GetExtent() * Ref.GetScale3D().GetAbs(), Ref.GetRotation(), Color, false, -1.f, SDPG_Foreground, 2.f);
	}

	/** Rectangle ecran d'une boite locale ; false si un coin est derriere la camera. */
	static bool ProjectBox(const FBox& LocalBox, const FTransform& Ref, const FVector& WorldOffset,
		const FMatrix& ViewProj, const FIntRect& ViewRect, FBox2D& Out)
	{
		Out = FBox2D(ForceInit);
		for (int32 i = 0; i < 8; ++i)
		{
			const FVector Corner(
				(i & 1) ? LocalBox.Max.X : LocalBox.Min.X,
				(i & 2) ? LocalBox.Max.Y : LocalBox.Min.Y,
				(i & 4) ? LocalBox.Max.Z : LocalBox.Min.Z);
			FVector2D Screen;
			if (!FSceneView::ProjectWorldToScreen(Ref.TransformPosition(Corner) + WorldOffset, ViewRect, ViewProj, Screen))
			{
				return false;
			}
			Out += Screen;
		}
		return Out.bIsValid;
	}

	static bool Overlaps(const FBox2D& A, const FBox2D& B, float Margin)
	{
		return A.Min.X < B.Max.X + Margin && A.Max.X > B.Min.X - Margin
			&& A.Min.Y < B.Max.Y + Margin && A.Max.Y > B.Min.Y - Margin;
	}

	/**
	 * Decalage vertical (pixels, Y ecran vers le bas) qui sort R de tous les panneaux.
	 * Dir = -1 : vers le haut, +1 : vers le bas. Plusieurs passes : un panneau franchi peut en
	 * faire toucher un autre.
	 */
	static float SolveShift(const FBox2D& R, TArrayView<const FBox2D> Panels, int32 Dir, float Margin)
	{
		float Shift = 0.f;
		for (int32 Pass = 0; Pass < 6; ++Pass)
		{
			const FBox2D Rs = R.ShiftBy(FVector2D(0.f, Shift));
			bool bMoved = false;
			for (const FBox2D& C : Panels)
			{
				if (!Overlaps(Rs, C, Margin * 0.5f)) continue;
				const float Need = Dir < 0
					? (C.Min.Y - Margin) - R.Max.Y
					: (C.Max.Y + Margin) - R.Min.Y;
				if (Dir < 0 ? Need < Shift : Need > Shift)
				{
					Shift = Need;
					bMoved = true;
				}
			}
			if (!bMoved) break;
		}
		return Shift;
	}
}

void URunnerStackingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		StartTimer();
		PostActorTickHandle = FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &URunnerStackingSubsystem::HandlePostActorTick);
	}
}

void URunnerStackingSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldPostActorTick.Remove(PostActorTickHandle);
	PostActorTickHandle.Reset();
	StopTimer();
	Entries.Reset();
	CardStates.Reset();
	CheckpointPanels.Reset();
	Super::Deinitialize();
}

void URunnerStackingSubsystem::SetConfig(const FRunnerStackingConfig& NewConfig)
{
	Config = NewConfig;
	bDirty = true;

	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		StartTimer();
	}
}

void URunnerStackingSubsystem::StartTimer()
{
	StopTimer();

	UWorld* World = GetWorld();
	if (!World) return;

	const float Interval = Config.UpdateIntervalSec;
	World->GetTimerManager().SetTimer(
		UpdateTimerHandle, 
		this, 
		&URunnerStackingSubsystem::TickSubsystem, 
		Interval, 
		true);
}

void URunnerStackingSubsystem::StopTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
}

int32 URunnerStackingSubsystem::FindEntryIndex(AActor* Runner) const
{
	if (!IsValid(Runner)) return INDEX_NONE;

	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].Runner.Get() == Runner)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void URunnerStackingSubsystem::RegisterRunner(AActor* Runner)
{
	if (!IsValid(Runner)) return;

	// Marker par interface
	if (!Runner->GetClass()->ImplementsInterface(URunnerStackableInterface::StaticClass())) return;
	
	if (Runner->IsHidden()) return;
	
	
	if (FindEntryIndex(Runner) != INDEX_NONE) return;

	FStackEntry E;
	E.Runner = Runner;
	E.TrackTransform = Runner->GetActorTransform();
	E.TrackDistanceMeters = 0.f;

	// Scale d'origine
	E.OriginalScaleX = Runner->GetActorScale3D().X;

	// HookHeightCm : on dérive depuis HookComponent vs AttachComponent
	if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Runner))
	{
		USceneComponent* Hook = SI->GetStackHookComponent();
		USceneComponent* Attach = SI->GetStackAttachComponent();

		if (IsValid(Hook) && IsValid(Attach))
		{
			// On prend la distance locale entre attach et hook comme “hauteur de pied”
			const FVector LocalHook = Hook->GetRelativeLocation();
			E.HookHeightCm =  FMath::Abs(LocalHook.Z); 
			E.HookLocalInAttach =
				Attach->GetComponentTransform().InverseTransformPosition(Hook->GetComponentLocation());
		}
	}

	Entries.Add(MoveTemp(E));
	Runner->OnEndPlay.AddDynamic(this, &URunnerStackingSubsystem::HandleActorEndPlay);

	bDirty = true;

	// UnregisterRunner coupe le timer quand la liste se vide (changement de course) :
	// sans ce redemarrage, plus aucun empilement n'etait calcule pour les runners suivants.
	if (UWorld* World = GetWorld(); World && World->IsGameWorld()
		&& !World->GetTimerManager().IsTimerActive(UpdateTimerHandle))
	{
		StartTimer();
	}
}

void URunnerStackingSubsystem::UnregisterRunner(AActor* Runner)
{
	if (!IsValid(Runner)) return;

	const int32 Idx = FindEntryIndex(Runner);
	if (Idx == INDEX_NONE) return;

	// Si stacké, on le détache proprement
	if (AActor* R = Entries[Idx].Runner.Get())
	{
		DetachRunnerToTrack(R, Entries[Idx]);
	}

	if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Runner))
	{
		SI->SetStackCardLift(0.f);
	}
	CardStates.Remove(TObjectKey<AActor>(Runner));

	Entries.RemoveAtSwap(Idx);
	bDirty = true;

	if (Entries.Num() == 0)
	{
		StopTimer();
	}
}

void URunnerStackingSubsystem::HandleActorEndPlay(AActor* Actor, EEndPlayReason::Type Reason)
{
	UnregisterRunner(Actor);
}

float URunnerStackingSubsystem::ComputeDynamicRadiusCm(const FStackEntry& BaseEntry) const
{
	const AActor* Base = BaseEntry.Runner.Get();
	if (!IsValid(Base)) return Config.RadiusNearCm;

	float SMin = 5.f, SMax = 40.f; // fallback
	if (const IRunnerInterface* IRunner = Cast<IRunnerInterface>(Base))
	{
		FMinMax MM = IRunner->GetRunnerMinMax();
		SMin = MM.Min.X;
		SMax = MM.Max.X;
	}
	
	if (SMin > SMax)
	{
		Swap(SMin, SMax);
	}

	const float Den = FMath::Max(KINDA_SMALL_NUMBER, SMax - SMin);
	float S = Base->GetActorScale3D().X;
	
	if (const IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Base))
	{
		if (USceneComponent* ScaleComp = SI->GetStackAttachComponent()) // ← si c'est lui qui scale en pratique
		{
			S = ScaleComp->GetComponentScale().X;
		}
	}
	S = FMath::Clamp(S, SMin, SMax);
	
	// 3) Alpha : 0 proche → 1 loin
	float Alpha = (S - SMin) / Den;
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	
	// 4) Rayon
	return FMath::Lerp(Config.RadiusNearCm, Config.RadiusFarCm, Alpha);
}

float URunnerStackingSubsystem::ComputeOverlapRadiusCm(const FStackEntry& BaseEntry) const
{
	const AActor* Base = BaseEntry.Runner.Get();
	if (!IsValid(Base)) return 0.f;

	// Meme pas vertical que la pile (AttachRunnerToBase), converti en monde par l'echelle reelle
	float VDelta = 0.f;
	if (const IRunnerInterface* IRunner = Cast<IRunnerInterface>(Base))
	{
		VDelta = IRunner->GetRunnerVDelta();
	}

	float S = Base->GetActorScale3D().X;
	if (const IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Base))
	{
		if (USceneComponent* ScaleComp = SI->GetStackAttachComponent())
		{
			S = ScaleComp->GetComponentScale().X;
		}
	}

	const float Factor = FMath::Max(0.f, CVarStackingOverlapFactor.GetValueOnGameThread());
	return Factor * (FMath::Max(1.f, BaseEntry.HookHeightCm) + VDelta) * FMath::Abs(S);
}

void URunnerStackingSubsystem::CleanupInvalid()
{
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (!Entries[i].Runner.IsValid())
		{
			Entries.RemoveAtSwap(i);
		}
	}
}

void URunnerStackingSubsystem::ApplyNewStackingState(
	const TArray<TWeakObjectPtr<AActor>>& NewBase,
	const TArray<int32>& NewOrder)
{
	check(NewBase.Num() == Entries.Num());
	check(NewOrder.Num() == Entries.Num());

	auto IsEligible = [](AActor* A) -> bool
	{
		return IsValid(A) && !A->IsHidden();
	};

	auto FindEntryByActor = [this](AActor* A) -> FStackEntry*
	{
		if (!IsValid(A)) return nullptr;
		for (FStackEntry& E : Entries)
		{
			if (E.Runner.Get() == A) return &E;
		}
		return nullptr;
	};

	// ------------------------------------------------------------
	// PASS 0 : detacher d'abord tous ceux qui quittent leur base
	// Sinon, quand un enfant devient la base de son ancienne base (depassement dans la pile),
	// l'anti-cycle d'AttachRunnerToBase refusait l'attache selon l'ordre du tableau et le
	// runner restait fige sans pied pendant un intervalle.
	// ------------------------------------------------------------
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		FStackEntry& E = Entries[i];
		AActor* Runner = E.Runner.Get();
		AActor* OldBaseActor = E.CurrentBase.Get();
		if (IsValid(Runner) && IsValid(OldBaseActor) && OldBaseActor != NewBase[i].Get())
		{
			InvalidateCardBox(OldBaseActor);
			DetachRunnerToTrack(Runner, E);
			E.CurrentBase  = nullptr;
			E.CurrentOrder = -1;
		}
	}

	// ------------------------------------------------------------
	// PASS 1 : appliquer l'état réel de stacking
	// ------------------------------------------------------------
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		FStackEntry& E = Entries[i];
		AActor* Runner = E.Runner.Get();
		if (!IsValid(Runner))
		{
			continue;
		}

		AActor* OldBaseActor = E.CurrentBase.Get();
		AActor* NewBaseActor = NewBase[i].Get();
		const int32 NewO = NewOrder[i];

		const bool bOldStacked = IsValid(OldBaseActor);

		// Runner non éligible => unstack forcé
		if (!IsEligible(Runner))
		{
			if (bOldStacked)
			{
				DetachRunnerToTrack(Runner, E);
			}

			if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Runner))
			{
				SI->SetIsStacked(false);
			}

			E.CurrentBase  = nullptr;
			E.CurrentOrder = -1;
			continue;
		}

		// Base proposée non éligible → refus du stacking
		if (IsValid(NewBaseActor) && !IsEligible(NewBaseActor))
		{
			NewBaseActor = nullptr;
		}

		const bool bNewStacked = IsValid(NewBaseActor);

		// Doit être unstacké
		if (!bNewStacked)
		{
			if (bOldStacked)
			{
				DetachRunnerToTrack(Runner, E);
			}

			if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Runner))
			{
				SI->SetIsStacked(false);
			}

			E.CurrentBase  = nullptr;
			E.CurrentOrder = -1;
			continue;
		}

		// Doit être stacké
		FStackEntry* BaseEntry = FindEntryByActor(NewBaseActor);
		if (!BaseEntry)
		{
			if (bOldStacked)
			{
				DetachRunnerToTrack(Runner, E);
			}

			if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Runner))
			{
				SI->SetIsStacked(false);
			}

			E.CurrentBase  = nullptr;
			E.CurrentOrder = -1;
			continue;
		}

		const bool bBaseChanged = (OldBaseActor != NewBaseActor);

		if (bBaseChanged && bOldStacked)
		{
			DetachRunnerToTrack(Runner, E);
		}

		// Contenu de pile modifie : les cartes des bases concernees changent de taille
		if (bBaseChanged)
		{
			InvalidateCardBox(NewBaseActor);
			InvalidateCardBox(OldBaseActor);
		}

		AttachRunnerToBase(Runner, E, NewBaseActor, *BaseEntry, NewO);

		if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Runner))
		{
			SI->SetIsStacked(true);
		}

		E.CurrentBase  = NewBaseActor;
		E.CurrentOrder = NewO;
	}

	// La visibilite des photos est geree une seule fois, dans TickSubsystem (passe photo) :
	// l'ancienne passe faite ici etait aussitot ecrasee par celle-ci et effacait la preference.
}


void URunnerStackingSubsystem::AttachRunnerToBase(
	AActor* Runner,
	const FStackEntry& RunnerEntry,
	AActor* Base,
	const FStackEntry& BaseEntry,
	int32 Order)
{
	if (!IsValid(Runner) || !IsValid(Base) || Runner == Base) return;

	// Récupération du Runner qui sera attaché (SI) et celui qui servira de Base(BaseSI)
	IRunnerStackableInterface* RunnerSI = Cast<IRunnerStackableInterface>(Runner);
	IRunnerStackableInterface* BaseSI   = Cast<IRunnerStackableInterface>(Base);
	if (!RunnerSI || !BaseSI) return;

	USceneComponent* RunnerAttach = RunnerSI->GetStackAttachComponent();
	USceneComponent* BaseHook     = BaseSI->GetStackHookComponent();
	if (!IsValid(RunnerAttach) || !IsValid(BaseHook)) return;

	// Anti-cycle
	if (BaseHook->IsAttachedTo(RunnerAttach)) return;

	// Attach seulement si nécessaire
	if (RunnerAttach->GetAttachParent() != BaseHook)
	{
		RunnerAttach->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		RunnerAttach->AttachToComponent(BaseHook, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		
	}

	// Décalage vertical
	const float Step  = FMath::Max(1.f, BaseEntry.HookHeightCm);
	float VDelta = 0.f;
	if (IRunnerInterface* IRunner = Cast<IRunnerInterface>(Runner))
	{
		VDelta = IRunner->GetRunnerVDelta();
	}
	
	const int32 VisualIndex = Order + 1;

	// TEST 1 : si trous → commente HookLocalInAttach pour voir si c'est lui
	const FVector Rel = -RunnerEntry.HookLocalInAttach + FVector(0, 0, (Step + VDelta) * VisualIndex);
	RunnerAttach->SetRelativeLocation(Rel);
}

void URunnerStackingSubsystem::DetachRunnerToTrack(AActor* Runner, const FStackEntry& RunnerEntry)
{
	if (!IsValid(Runner)) return;

	// Détache du parent (si attaché)
	Runner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Snap immédiat sur la vraie position track
	Runner->SetActorTransform(RunnerEntry.TrackTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

/**
 * @brief TickSubsystem
 * @note Appellée par Timer pour évaluer le Stacking/Destacking
 */
void URunnerStackingSubsystem::TickSubsystem()
{
	CleanupInvalid();

	const int32 Num = Entries.Num();
	if (Num == 0)
	{
		return;
	}

	// ------------------------------------------------------------
	// 0) Si pas dirty, on peut rendre dirty si la caméra a bougé assez
	// ------------------------------------------------------------
	if (!bDirty)
	{
		AActor* AnyActor = Entries[0].Runner.Get();
		if (IsValid(AnyActor))
		{
			UWorld* World = AnyActor->GetWorld();
			APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;

			if (PC && PC->PlayerCameraManager)
			{
				const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();

				const float Th = CameraMoveThresholdCm;
				const float ThSq = Th * Th;

				if (!bHasLastCamLoc)
				{
					LastCamLoc = CamLoc;
					bHasLastCamLoc = true;
					bDirty = true;
				}
				else if (FVector::DistSquared(CamLoc, LastCamLoc) > ThSq)
				{
					LastCamLoc = CamLoc;
					bDirty = true;
				}
			}
		}
	}

	// V1 : recompute seulement si dirty
	if (!bDirty)
	{
		return;
	}
	bDirty = false;

	// ------------------------------------------------------------
	// Eligibilité : seuls les runners visibles participent au stacking
	// ------------------------------------------------------------
	auto IsEligible = [](AActor* A) -> bool
	{
		// Remplace IsHidden() par IsActorHiddenInGame() si c'est ce que tu utilises.
		return IsValid(A) && !A->IsHidden();
	};

	// ------------------------------------------------------------
	// 1) Tri stable (par distance sur la piste) — seulement éligibles
	// ------------------------------------------------------------
	TArray<int32> Sorted;
	Sorted.Reserve(Num);

	TArray<bool> Assigned;
	Assigned.Init(false, Num);

	for (int32 i = 0; i < Num; ++i)
	{
		AActor* A = Entries[i].Runner.Get();
		const bool bOk = IsEligible(A);

		// Non éligible → jamais base/candidat
		Assigned[i] = !bOk;

		if (bOk)
		{
			Sorted.Add(i);
		}
	}

	Sorted.Sort([this](int32 A, int32 B)
	{
		return Entries[A].TrackDistanceMeters < Entries[B].TrackDistanceMeters;
	});

	// ------------------------------------------------------------
	// 2) Calcul du nouvel état (NewBase / NewOrder)
	// ------------------------------------------------------------
	TArray<TWeakObjectPtr<AActor>> NewBase;
	TArray<int32> NewOrder;
	NewBase.Init(nullptr, Num);
	NewOrder.Init(-1, Num);

	// Sorted : tableau des index des runners triés par distance sur la spline
	for (int32 si = 0; si < Sorted.Num(); ++si)
	{
		const int32 BaseIdx = Sorted[si];
		if (Assigned[BaseIdx])
		{
			continue;
		}

		FStackEntry& BaseEntry = Entries[BaseIdx];
		AActor* BaseActor = BaseEntry.Runner.Get();

		// Sécurité : la base doit rester éligible
		if (!IsEligible(BaseActor))
		{
			Assigned[BaseIdx] = true;
			continue;
		}

		// Base
		Assigned[BaseIdx] = true;
		NewBase[BaseIdx] = nullptr;
		NewOrder[BaseIdx] = -1;

		const FVector BasePos = BaseEntry.TrackTransform.GetLocation();
		const float RadiusCm = ComputeDynamicRadiusCm(BaseEntry);
		// Plancher : en dessous d'une hauteur de runner, les etiquettes se chevauchent a l'image
		const float OverlapCm = ComputeOverlapRadiusCm(BaseEntry);

		// Meme progression que le rayon (echelle du runner, donc distance camera) : 0 camera
		// proche (plus aucune pile, on voit les vrais ecarts) -> 1 camera loin (ecart max).
		const float ScaleAlpha = FMath::Clamp(
			(RadiusCm - Config.RadiusNearCm) / FMath::Max(KINDA_SMALL_NUMBER, Config.RadiusFarCm - Config.RadiusNearCm),
			0.f, 1.f);

		TArray<int32> Children;
		Children.Reserve(16);

		for (int32 sj = si + 1; sj < Sorted.Num(); ++sj)
		{
			const int32 CandIdx = Sorted[sj];
			if (Assigned[CandIdx]) continue;

			FStackEntry& CandEntry = Entries[CandIdx];
			AActor* CandActor = CandEntry.Runner.Get();

			// Sécurité : candidat doit rester éligible
			if (!IsEligible(CandActor))
			{
				Assigned[CandIdx] = true; // on l'exclut complètement de ce tick
				continue;
			}

			const FVector CandPos = CandEntry.TrackTransform.GetLocation();
			const float DistSq = FVector::DistSquared(BasePos, CandPos);

			// Hysteresis proportionnelle au rayon : 10 cm fixes ne servaient a rien quand le
			// rayon monte a 2 km (camera loin), les piles clignotaient autour du seuil.
			const bool bWasStackedOnThisBase = (CandEntry.CurrentBase.Get() == BaseActor);

			// Ecart sur le trace (TrackDistanceMeters est en cm : distance de spline monde)
			const float MaxGapSettingCm = CVarStackingMaxTrackGapM.GetValueOnGameThread() * 100.f;
			if (MaxGapSettingCm > 0.f)
			{
				const float MaxGapCm = FMath::Max(MaxGapSettingCm * ScaleAlpha, OverlapCm);
				const float GapCm = FMath::Abs(CandEntry.TrackDistanceMeters - BaseEntry.TrackDistanceMeters);
				const float GapTh = bWasStackedOnThisBase ? MaxGapCm * 1.1f : MaxGapCm;
				if (GapCm > GapTh)
				{
					continue;
				}
			}

			const float Extra = bWasStackedOnThisBase
				? FMath::Max(Config.HysteresisCm, RadiusCm * 0.1f)
				: 0.f;

			const float Th = FMath::Max(RadiusCm, OverlapCm) + Extra;
			const float ThSq = Th * Th;

			if (DistSq <= ThSq)
			{
				Children.Add(CandIdx);
			}
		}

		// Tri des index des enfants pour éviter les trous
		Children.Sort([this, BaseActor](int32 A, int32 B)
		{
			const FStackEntry& EA = Entries[A];
			const FStackEntry& EB = Entries[B];

			const bool aWas = (EA.CurrentBase.Get() == BaseActor);
			const bool bWas = (EB.CurrentBase.Get() == BaseActor);

			if (aWas != bWas)
			{
				return aWas;
			}

			if (aWas)
			{
				const int32 OA = EA.CurrentOrder;
				const int32 OB = EB.CurrentOrder;

				const bool aValid = (OA >= 0);
				const bool bValid = (OB >= 0);

				if (aValid != bValid)
				{
					return aValid;
				}

				if (aValid && OA != OB)
				{
					return OA < OB;
				}

				if (EA.TrackDistanceMeters != EB.TrackDistanceMeters)
				{
					return EA.TrackDistanceMeters < EB.TrackDistanceMeters;
				}

				return A < B;
			}

			if (EA.TrackDistanceMeters != EB.TrackDistanceMeters)
			{
				return EA.TrackDistanceMeters < EB.TrackDistanceMeters;
			}

			return A < B;
		});

		int32 ChildOrder = 0;
		for (int32 CandIdx : Children)
		{
			Assigned[CandIdx] = true;
			NewBase[CandIdx]  = BaseActor;
			NewOrder[CandIdx] = ChildOrder++;
		}
	}

	// Debug
	// for (int32 i = 0; i < Entries.Num(); ++i)
	// {
	// 	if (AActor* Base = NewBase[i].Get())
	// 	{
	// 		if (NewOrder[i] < 0)
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("[OrderDbg] child has negative order: %s"), *Entries[i].Runner->GetName());
	// 		}
	// 	}
	// }

	ApplyNewStackingState(NewBase, NewOrder);

	// ------------------------------------------------------------
	// 4) Photo pass : base + enfants => photo OFF
	// ------------------------------------------------------------
	TSet<AActor*> UsedAsBase;
	UsedAsBase.Reserve(Num);

	for (int32 i = 0; i < Num; ++i)
	{
		if (AActor* Base = NewBase[i].Get())
		{
			UsedAsBase.Add(Base);
		}
	}

	for (int32 i = 0; i < Num; ++i)
	{
		FStackEntry& E = Entries[i];
		AActor* Runner = E.Runner.Get();
		if (!IsValid(Runner))
		{
			continue;
		}

		IRunnerInterface* RI = Cast<IRunnerInterface>(Runner);
		if (!RI)
		{
			continue;
		}

		const bool bIsChild = NewBase[i].IsValid();
		const bool bIsBaseWithChildren = UsedAsBase.Contains(Runner);
		const bool bShouldShowPhoto = !(bIsChild || bIsBaseWithChildren);

		// On ne touche la photo que quand l'etat de pile change : sinon chaque recalcul
		// (plusieurs fois par seconde) reaffichait une photo masquee depuis la regie.
		// Le club reste visible en pile : sa visibilité ne dépend que de la préférence user
		if (E.bPhotoVisible != bShouldShowPhoto)
		{
			RI->TogglePhoto(bShouldShowPhoto);
			E.bPhotoVisible = bShouldShowPhoto;
		}
		E.bClubVisible = RI->IsClubVisible();
	}
}


/**
 * @brief Mise à jour de l'évolution de la position du Runner
 * @note Appellée par Runner à l'update de sa Location
 * @param Runner 
 * @param TrackTransform 
 * @param TrackDistanceMeters 
 */
void URunnerStackingSubsystem::UpdateRunnerTrackState(AActor* Runner, const FTransform& TrackTransform, float TrackDistanceMeters)
{
	if (!IsValid(Runner)) return;

	const int32 Idx = FindEntryIndex(Runner);
	if (Idx == INDEX_NONE)
	{
		// pratique : auto-register si oublié
		RegisterRunner(Runner);
	}

	const int32 Idx2 = FindEntryIndex(Runner);
	if (Idx2 == INDEX_NONE) return;

	FStackEntry& E = Entries[Idx2];
	E.TrackTransform = TrackTransform;
	E.TrackDistanceMeters = TrackDistanceMeters;

	// Important : même si stacké, on veut recalculer le clustering
	bDirty = true;
}

void URunnerStackingSubsystem::MakeDirty()
{
	bDirty = true;
}

void URunnerStackingSubsystem::InvalidateCardBox(AActor* Runner)
{
	if (!Runner) return;
	if (FCardAvoidState* S = CardStates.Find(TObjectKey<AActor>(Runner)))
	{
		S->LocalBox = FBox(ForceInit);
	}
}

void URunnerStackingSubsystem::HandlePostActorTick(UWorld* InWorld, ELevelTick TickType, float DeltaSeconds)
{
	if (InWorld == GetWorld())
	{
		TickCardAvoidance(DeltaSeconds);
	}
}

/**
 * @brief Evitement runner / checkpoint, chaque image (apres le tick des acteurs : camera et
 *        positions de l'image courante).
 * @note  Rectangles ecran : boites locales mises en cache (rafraichies a tour de role, quelques
 *        unes par image), puis 8 coins projetes par carte. Rien n'est calcule tant qu'aucun
 *        panneau checkpoint n'est a l'ecran et qu'aucune carte n'est decalee.
 */
void URunnerStackingSubsystem::TickCardAvoidance(float DeltaSeconds)
{
	using namespace RunnerCardAvoid;

	UWorld* World = GetWorld();
	if (!World || Entries.Num() == 0) return;

	const bool bEnabled = CVarCardAvoidEnable.GetValueOnGameThread() != 0;

	// Liste des panneaux checkpoint (rarement modifiee : un balayage par seconde)
	const double Now = World->GetTimeSeconds();
	if (bEnabled && Now >= NextCheckpointScanTime)
	{
		NextCheckpointScanTime = Now + 1.0;
		TArray<FCheckpointPanel> Found;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const ICheckpointInterface* CI = Cast<ICheckpointInterface>(*It);
			USceneComponent* Panel = CI ? CI->GetPanelComponent() : nullptr;
			if (!Panel) continue;
			FCheckpointPanel& P = Found.AddDefaulted_GetRef();
			P.Panel = Panel;
			// Garde la boite deja calculee
			for (const FCheckpointPanel& Old : CheckpointPanels)
			{
				if (Old.Panel.Get() == Panel) { P.LocalBox = Old.LocalBox; break; }
			}
		}
		CheckpointPanels = MoveTemp(Found);
	}

	// Projection de la vue du joueur
	APlayerController* PC = World->GetFirstPlayerController();
	ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr;
	FViewport* Viewport = (LP && LP->ViewportClient) ? LP->ViewportClient->Viewport : nullptr;
	FSceneViewProjectionData ProjData;
	if (!Viewport || !LP->GetProjectionData(Viewport, ProjData)) return;
	const FMatrix ViewProj = ProjData.ComputeViewProjectionMatrix();
	const FIntRect ViewRect = ProjData.GetConstrainedViewRect();

	const int32 DebugLevel = CVarCardAvoidDebug.GetValueOnGameThread();
	const bool bDebug = DebugLevel > 0;
	static double LastDebugLog = 0.0;
	const bool bDebugLog = bDebug && Now - LastDebugLog > 0.5;
	if (bDebugLog) LastDebugLog = Now;

	// Rectangles des panneaux checkpoint visibles
	TArray<FBox2D, TInlineAllocator<16>> PanelRects;
	if (bEnabled && CheckpointPanels.Num() > 0)
	{
		// Une boite rafraichie par image (le texte et la meteo changent rarement)
		CheckpointBoxCursor = (CheckpointBoxCursor + 1) % CheckpointPanels.Num();
		for (int32 i = 0; i < CheckpointPanels.Num(); ++i)
		{
			FCheckpointPanel& P = CheckpointPanels[i];
			USceneComponent* Panel = P.Panel.Get();
			if (!Panel) continue;
			const AActor* Owner = Panel->GetOwner();
			if (!Owner || Owner->IsHidden() || !Panel->IsVisible()) continue;
			if (!P.LocalBox.IsValid || i == CheckpointBoxCursor)
			{
				P.LocalBox = ComputeLocalBox(Panel);
			}
			if (bDebug)
			{
				DrawBox(World, P.LocalBox, Panel->GetComponentTransform(), FVector::ZeroVector, FColor::Cyan);
			}
			FBox2D R;
			if (P.LocalBox.IsValid
				&& ProjectBox(P.LocalBox, Panel->GetComponentTransform(), FVector::ZeroVector, ViewProj, ViewRect, R)
				&& R.Max.X > ViewRect.Min.X && R.Min.X < ViewRect.Max.X
				&& R.Max.Y > ViewRect.Min.Y && R.Min.Y < ViewRect.Max.Y)
			{
				PanelRects.Add(R);
			}
		}
	}

	if (PanelRects.Num() == 0 && !bAnyCardLifted) return;

	const float Margin = FMath::Max(0.f, CVarCardAvoidMarginPx.GetValueOnGameThread());
	const float Speed = FMath::Max(0.f, CVarCardAvoidSpeed.GetValueOnGameThread());
	const float MinFoot = FMath::Clamp(CVarCardAvoidMinFoot.GetValueOnGameThread(), 0.f, 1.f);
	// Boites des cartes : environ un huitieme des runners par image, a tour de role
	const int32 BoxPeriod = FMath::Clamp(Entries.Num() / 2, 1, 8);
	bool bAnyLifted = false;

	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		const FStackEntry& E = Entries[i];
		AActor* Runner = E.Runner.Get();
		IRunnerStackableInterface* SI = Runner ? Cast<IRunnerStackableInterface>(Runner) : nullptr;
		if (!SI) continue;

		const float Lift = SI->GetStackCardLift();
		USceneComponent* Card = SI->GetStackCardComponent();
		USceneComponent* Root = Runner->GetRootComponent();

		// Enfant de pile, cache ou sans carte : a sa place tout de suite (il suit sa base)
		if (!Card || !Root || Runner->IsHidden() || E.CurrentBase.IsValid())
		{
			if (!FMath::IsNearlyZero(Lift)) SI->SetStackCardLift(0.f);
			if (FCardAvoidState* S = CardStates.Find(TObjectKey<AActor>(Runner))) { S->Target = 0.f; S->Side = 0; }
			continue;
		}

		FCardAvoidState& S = CardStates.FindOrAdd(TObjectKey<AActor>(Runner));
		float Target = 0.f;
		int8 Side = 0;

		if (PanelRects.Num() > 0)
		{
			// Boite de la carte (pile comprise)
			if (!S.LocalBox.IsValid || (i % BoxPeriod) == (RunnerBoxCursor % BoxPeriod))
			{
				S.LocalBox = ComputeLocalBox(Card);
			}

			// Rectangle a la place d'origine (decalage courant retire)
			const FVector UpPerUnit = Root->GetComponentTransform().TransformVector(FVector::UpVector);
			FBox2D R;
			if (S.LocalBox.IsValid
				&& ProjectBox(S.LocalBox, Card->GetComponentTransform(), -UpPerUnit * Lift, ViewProj, ViewRect, R))
			{
				// Hysteresis : une carte deja decalee garde sa place tant qu'elle frole le panneau
				const float TestMargin = S.Side != 0 ? Margin : 0.f;
				bool bOverlap = false;
				for (const FBox2D& C : PanelRects)
				{
					if (Overlaps(R, C, TestMargin)) { bOverlap = true; break; }
				}

				const float CardScaleZ = FMath::Max(KINDA_SMALL_NUMBER, FMath::Abs(Card->GetRelativeScale3D().Z));
				const float HeightUnits = (S.LocalBox.Max.Z - S.LocalBox.Min.Z) * CardScaleZ;
				const float PxPerUnit = HeightUnits > KINDA_SMALL_NUMBER ? R.GetSize().Y / HeightUnits : 0.f;

				if (bOverlap && PxPerUnit > KINDA_SMALL_NUMBER)
				{
					const FBox2D Expanded(R.Min - FVector2D(Margin, 0.f), R.Max + FVector2D(Margin, 0.f));
					const float UpPx = SolveShift(Expanded, PanelRects, -1, Margin);
					const float DownPx = SolveShift(Expanded, PanelRects, +1, Margin);

					// Au-dessus : la carte doit rester dans l'image
					const bool bUpFree = R.Min.Y + UpPx >= ViewRect.Min.Y;
					// En dessous : dans l'image, et le pied garde une part de sa hauteur
					const float CardBottomUnits = (Card->GetRelativeLocation().Z - Lift) + S.LocalBox.Min.Z * CardScaleZ;
					const float MaxDown = FMath::Max(0.f, CardBottomUnits * (1.f - MinFoot));
					const float DownLift = -DownPx / PxPerUnit;
					const bool bDownFree = R.Max.Y + DownPx <= ViewRect.Max.Y && DownLift >= -MaxDown;

					if (S.Side < 0 && bDownFree)        Side = -1; // garde le cote deja choisi
					else if (bUpFree)                   Side = +1;
					else if (bDownFree)                 Side = -1;
					else                                Side = FMath::Abs(UpPx) <= FMath::Abs(DownPx) ? +1 : -1;

					Target = Side > 0 ? -UpPx / PxPerUnit : FMath::Max(DownLift, -MaxDown);
				}
				if (bDebug)
				{
					DrawBox(World, S.LocalBox, Card->GetComponentTransform(), FVector::ZeroVector,
						FMath::IsNearlyZero(Lift) ? FColor::Yellow : FColor::Red);
				}
				if (bDebugLog && (bOverlap || !FMath::IsNearlyZero(Lift)))
				{
					UE_LOG(LogTemp, Log, TEXT("[CardAvoid] %s rect %.0f,%.0f-%.0f,%.0f (boite Z %.0f..%.0f, %.3f px/u) lift %.0f -> %.0f cote %d"),
						*Runner->GetName(), R.Min.X, R.Min.Y, R.Max.X, R.Max.Y, S.LocalBox.Min.Z, S.LocalBox.Max.Z,
						PxPerUnit, Lift, Target, Side);
					for (const FBox2D& C : PanelRects)
					{
						if (Overlaps(R, C, Margin))
						{
							UE_LOG(LogTemp, Log, TEXT("[CardAvoid]   panneau %.0f,%.0f-%.0f,%.0f"), C.Min.X, C.Min.Y, C.Max.X, C.Max.Y);
						}
					}
				}
			}
			else
			{
				// Derriere la camera : on ne change rien
				Target = S.Target;
				Side = S.Side;
			}
		}

		S.Target = Target;
		S.Side = Side;

		float NewLift = Speed > 0.f ? FMath::FInterpTo(Lift, Target, DeltaSeconds, Speed) : Target;
		if (FMath::Abs(NewLift - Target) < 0.5f) NewLift = Target;
		if (!FMath::IsNearlyEqual(NewLift, Lift, 0.01f))
		{
			SI->SetStackCardLift(NewLift);
		}
		bAnyLifted |= !FMath::IsNearlyZero(NewLift);
	}

	RunnerBoxCursor = (RunnerBoxCursor + 1) % 8;
	bAnyCardLifted = bAnyLifted;
}