// BroadcastCaptureSubsystem.cpp
#include "BroadcastCaptureSubsystem.h"
#include "BroadcastSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "BroadcastInterface.h"
#include "Engine/TextureRenderTarget2D.h"

void UBroadcastCaptureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UBroadcastSettings* S = GetDefault<UBroadcastSettings>();
	if (!S || S->SharedRT.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("BroadcastSettings.SharedRT not set"));
		return;
	}
	SharedRT = S->SharedRT.LoadSynchronous();
}

void UBroadcastCaptureSubsystem::RequestViewTarget(APlayerController* PC, AActor* NewTarget, float BlendTime)
{
	if (!PC) return;
	CachedPC = PC;

	PC->SetViewTargetWithBlend(NewTarget, BlendTime);

	// Choix “broadcast safe” : switch capture tout de suite
	ActivateSource(ResolveCaptureSourceFromViewTarget(NewTarget));
}

void UBroadcastCaptureSubsystem::Tick(float DeltaTime)
{
	UWorld* W = GetWorld();
	if (!W) return;
	if (W->WorldType != EWorldType::PIE && W->WorldType != EWorldType::Game)
		return;
	
	Accum += DeltaTime;

	const float Period = (MonitorHz > 0.f) ? (1.f / MonitorHz) : 0.f;
	if (Period > 0.f && Accum < Period)
		return;
	Accum = 0.f;

	APlayerController* PC = CachedPC.Get();
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		CachedPC = PC;
		if (!PC) return;

		// Default = pawn (ton BP broadcast)
		DefaultSource = PC->GetPawn();
		ActivateSource(DefaultSource.Get());
	}

	AActor* VT = PC->GetViewTarget();
	AActor* Desired = ResolveCaptureSourceFromViewTarget(VT);

	// Si la caméra active ne correspond à aucune source capturable,
	// on retombe sur le pawn default.
	if (!Desired) Desired = DefaultSource.Get();

	ActivateSource(Desired);
}

AActor* UBroadcastCaptureSubsystem::ResolveCaptureSourceFromViewTarget(AActor* ViewTarget) const
{
	if (!IsValid(ViewTarget)) return nullptr;

	// Le ViewTarget est souvent directement l’actor qui “porte” la cinecam
	// -> on vérifie s’il implémente l’interface.
	if (ViewTarget->GetClass()->ImplementsInterface(UBroadcastInterface::StaticClass()))
		return ViewTarget;

	// Sinon : aucun
	return nullptr;
}

void UBroadcastCaptureSubsystem::ActivateSource(AActor* Source)
{
	if (!SharedRT) return;
	if (!IsValid(Source)) return;
	if (ActiveSource.Get() == Source) return;

	// OFF ancienne
	if (AActor* Old = ActiveSource.Get())
	{
		if (Old->GetClass()->ImplementsInterface(UBroadcastInterface::StaticClass()))
		{
			IBroadcastInterface::Execute_SetBroadcastCaptureEnabled(Old, false, SharedRT);
		}
	}

	ActiveSource = Source;

	// ON nouvelle
	if (AActor* Cur = ActiveSource.Get())
	{
		if (Cur->GetClass()->ImplementsInterface(UBroadcastInterface::StaticClass()))
		{
			IBroadcastInterface::Execute_SetBroadcastCaptureEnabled(Cur, true, SharedRT);
		}
	}
}