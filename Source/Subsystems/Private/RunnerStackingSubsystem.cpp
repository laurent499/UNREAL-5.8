#include "RunnerStackingSubsystem.h"
#include "RunnerInterface.h"
#include "RunnerStackableInterface.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"

void URunnerStackingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UWorld* World = GetWorld(); World && World->IsGameWorld())
	{
		StartTimer();
	}
}

void URunnerStackingSubsystem::Deinitialize()
{
	StopTimer();
	Entries.Reset();
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

		AttachRunnerToBase(Runner, E, NewBaseActor, *BaseEntry, NewO);

		if (IRunnerStackableInterface* SI = Cast<IRunnerStackableInterface>(Runner))
		{
			SI->SetIsStacked(true);
		}

		E.CurrentBase  = NewBaseActor;
		E.CurrentOrder = NewO;
	}

	// ------------------------------------------------------------
	// PASS 2 : reconstruire l'état appliqué pour les photos
	// ------------------------------------------------------------
	TMap<TObjectPtr<AActor>, int32> AppliedChildCountByBase;
	AppliedChildCountByBase.Reserve(Entries.Num());

	for (const FStackEntry& E : Entries)
	{
		AActor* Runner = E.Runner.Get();
		AActor* Base   = E.CurrentBase.Get();

		if (!IsEligible(Runner))
		{
			continue;
		}

		if (IsEligible(Base))
		{
			AppliedChildCountByBase.FindOrAdd(Base)++;
		}
	}

	// ------------------------------------------------------------
	// PASS 3 : rafraîchir la visibilité réelle des photos
	// ------------------------------------------------------------
	for (FStackEntry& E : Entries)
	{
		AActor* Runner = E.Runner.Get();
		if (!IsValid(Runner))
		{
			continue;
		}

		const bool bIsChild = IsValid(E.CurrentBase.Get());
		const bool bIsBaseWithChildren = AppliedChildCountByBase.FindRef(Runner) > 0;

		if (IRunnerInterface* RI = Cast<IRunnerInterface>(Runner))
		{
			// IsPhotoVisible() = préférence user uniquement
			const bool bShouldShowPhoto =
				RI->IsPhotoVisible() && !(bIsChild || bIsBaseWithChildren);

			// Le club reste visible en pile : sa visibilité ne dépend que de la préférence user
			RI->TogglePhoto(bShouldShowPhoto);
		}
	}
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

			const bool bWasStackedOnThisBase = (CandEntry.CurrentBase.Get() == BaseActor);
			const float Extra = bWasStackedOnThisBase ? Config.HysteresisCm : 0.f;

			const float Th = RadiusCm + Extra;
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

		// Le club reste visible en pile : sa visibilité ne dépend que de la préférence user
		RI->TogglePhoto(bShouldShowPhoto);
		E.bPhotoVisible = bShouldShowPhoto;
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