// BroadcastCaptureSubsystem.cpp
#include "BroadcastCaptureSubsystem.h"
#include "BroadcastSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "BroadcastInterface.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/GameViewportClient.h"
#include "HAL/IConsoleManager.h"
#include "OWLCaptureComponent.h"
#include "Widgets/SLeafWidget.h"
#include "Brushes/SlateColorBrush.h"
#include "Rendering/DrawElements.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

static TAutoConsoleVariable<int32> CVarBroadcastMirrorViewport(
	TEXT("trail.Broadcast.MirrorViewport"),
	-1,
	TEXT("Recopie la capture OWL dans le viewport et coupe le rendu du monde principal.\n")
	TEXT(" -1 : suit BroadcastSettings.bMirrorCaptureToViewport (defaut)\n")
	TEXT("  0 : desactive (le viewport rend la scene normalement)\n")
	TEXT("  1 : active"));

namespace
{
	// Affiche le render target partage, letterboxe a son ratio, sur fond noir.
	class SBroadcastMirror : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SBroadcastMirror) {}
		SLATE_END_ARGS()

		void Construct(const FArguments&, UTextureRenderTarget2D* RT)
		{
			Target = RT;
			// Affichage seul : la souris doit rester au viewport
			SetVisibility(EVisibility::HitTestInvisible);
			Brush.SetResourceObject(RT);
			Brush.DrawAs = ESlateBrushDrawType::Image;
			Brush.ImageSize = FVector2D(RT->SizeX, RT->SizeY);
		}

		virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geom, const FSlateRect&, FSlateWindowElementList& OutDrawElements,
			int32 LayerId, const FWidgetStyle&, bool) const override
		{
			// Fond noir explicite (bandes hors ratio)
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geom.ToPaintGeometry(), &Background, ESlateDrawEffect::None, FLinearColor::Black);

			const UTextureRenderTarget2D* RT = Target.Get();
			if (!RT || RT->SizeX <= 0 || RT->SizeY <= 0) return LayerId;

			// Ratio relu a chaque image : OWL peut redimensionner la cible
			const float Aspect = float(RT->SizeX) / float(RT->SizeY);
			const FVector2f Size = Geom.GetLocalSize();
			FVector2f Fit(Size.X, Size.X / Aspect);
			if (Fit.Y > Size.Y) Fit = FVector2f(Size.Y * Aspect, Size.Y);
			const FVector2f Offset = (Size - Fit) * 0.5f;

			if (!bLogged)
			{
				bLogged = true;
				UE_LOG(LogTemp, Log, TEXT("[BroadcastMirror] RT %dx%d, zone %.0fx%.0f, image %.0fx%.0f"),
					RT->SizeX, RT->SizeY, Size.X, Size.Y, Fit.X, Fit.Y);
			}

			// L'alpha de la capture n'est pas une opacite d'affichage : on l'ignore
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, Geom.ToPaintGeometry(Fit, FSlateLayoutTransform(Offset)),
				&Brush, ESlateDrawEffect::IgnoreTextureAlpha);
			return LayerId + 1;
		}

		virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(16.f, 9.f); }

	private:
		FSlateBrush Brush;
		TWeakObjectPtr<UTextureRenderTarget2D> Target;
		FSlateColorBrush Background = FSlateColorBrush(FLinearColor::White);
		mutable bool bLogged = false;
	};
}

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

void UBroadcastCaptureSubsystem::Deinitialize()
{
	RemoveViewportMirror();
	Super::Deinitialize();
}

void UBroadcastCaptureSubsystem::UpdateViewportMirror()
{
	const int32 CVarValue = CVarBroadcastMirrorViewport.GetValueOnGameThread();
	bool bWanted = SharedRT && (CVarValue >= 0 ? CVarValue > 0 : GetDefault<UBroadcastSettings>()->bMirrorCaptureToViewport);
#if WITH_EDITOR
	// PIE ejecte (F8) : la camera editeur pilote le viewport, il faut rendre le monde
	if (GEditor && GEditor->bIsSimulatingInEditor) bWanted = false;
#endif

	if (!bWanted)
	{
		RemoveViewportMirror();
		return;
	}
	if (MirrorWidget.IsValid() && MirrorViewport.IsValid()) return;

	UGameViewportClient* GVC = GetWorld()->GetGameViewport();
	if (!GVC) return;

	MirrorWidget = SNew(SBroadcastMirror, SharedRT);
	// Derriere les widgets UMG (ZOrder >= 0)
	GVC->AddViewportWidgetContent(MirrorWidget.ToSharedRef(), -1000);
	GVC->bDisableWorldRendering = true;
	MirrorViewport = GVC;
}

void UBroadcastCaptureSubsystem::RemoveViewportMirror()
{
	if (UGameViewportClient* GVC = MirrorViewport.Get())
	{
		if (MirrorWidget.IsValid())
		{
			GVC->RemoveViewportWidgetContent(MirrorWidget.ToSharedRef());
		}
		GVC->bDisableWorldRendering = false;
	}
	MirrorWidget.Reset();
	MirrorViewport.Reset();
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

	UpdateViewportMirror();

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
			// -BroadcastResY=720 : hauteur de la capture OWL (et donc du flux), 1080 par defaut.
			// OWL recalcule la largeur d'apres le ratio de la camera.
			int32 ResY = 0;
			if (FParse::Value(FCommandLine::Get(), TEXT("BroadcastResY="), ResY) && ResY >= 64)
			{
				if (UOWLCaptureComponent* Capture = Cur->FindComponentByClass<UOWLCaptureComponent>())
				{
					Capture->ResolutionY = ResY;
					Capture->ResolutionX = FMath::RoundToInt(ResY * 16.f / 9.f);
				}
			}

			IBroadcastInterface::Execute_SetBroadcastCaptureEnabled(Cur, true, SharedRT);
		}
	}
}