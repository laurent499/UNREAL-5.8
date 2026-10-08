#include "UI/SLoadingOverlay.h"
#include "LoadingStatusSubsystem.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"

namespace LoadingOverlayStyle
{
	// L'overlay est ajoute directement au viewport : on garde l'echelle d'origine de l'ecran
	static constexpr float S = 3.f;

	const FLinearColor Scrim(0.020f, 0.027f, 0.043f, 0.94f);
	const FLinearColor CardFill(0.063f, 0.082f, 0.114f, 1.f);
	const FLinearColor CardBorder(0.137f, 0.165f, 0.212f, 1.f);
	const FLinearColor Track(0.137f, 0.165f, 0.212f, 1.f);
	const FLinearColor Text(0.945f, 0.957f, 0.969f, 1.f);
	const FLinearColor Muted(0.545f, 0.584f, 0.639f, 1.f);
	const FLinearColor Accent(0.231f, 0.510f, 0.965f, 1.f);
	const FLinearColor Ok(0.204f, 0.780f, 0.471f, 1.f);
	const FLinearColor Run(0.976f, 0.620f, 0.157f, 1.f);
	const FLinearColor Fail(0.937f, 0.329f, 0.329f, 1.f);
	const FLinearColor Queued(0.369f, 0.408f, 0.471f, 1.f);

	FSlateFontInfo Font(const char* Weight, float Size)
	{
		return FCoreStyle::GetDefaultFontStyle(Weight, FMath::RoundToInt(Size * S));
	}

	const FSlateBrush* Rounded(const FLinearColor& Fill, float Radius, const FLinearColor& Outline = FLinearColor::Transparent, float OutlineWidth = 0.f)
	{
		// Les brosses doivent survivre au widget : on les garde en cache par apparence
		static TMap<FString, TSharedPtr<FSlateRoundedBoxBrush>> Cache;
		const FString Key = FString::Printf(TEXT("%s|%.1f|%s|%.1f"), *Fill.ToString(), Radius, *Outline.ToString(), OutlineWidth);
		TSharedPtr<FSlateRoundedBoxBrush>& Brush = Cache.FindOrAdd(Key);
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateRoundedBoxBrush>(Fill, Radius * S, Outline, OutlineWidth * S);
		}
		return Brush.Get();
	}

	const FProgressBarStyle& ProgressStyle()
	{
		static FProgressBarStyle Style = FProgressBarStyle()
			.SetBackgroundImage(FSlateRoundedBoxBrush(Track, 4.f * S))
			.SetFillImage(FSlateRoundedBoxBrush(FLinearColor::White, 4.f * S))
			.SetMarqueeImage(FSlateRoundedBoxBrush(FLinearColor::White, 4.f * S));
		return Style;
	}

	FLinearColor StateColor(ELoadingTaskState State)
	{
		switch (State)
		{
		case ELoadingTaskState::Success: return Ok;
		case ELoadingTaskState::Running: return Run;
		case ELoadingTaskState::Failed:  return Fail;
		default:                         return Queued;
		}
	}
}

namespace
{
	// Etapes connues d'une course : prefixe de l'Id de tache (avant « _<raceId> ») -> libelle et ordre
	struct FStepDef { const TCHAR* Prefix; const TCHAR* Label; };
	const FStepDef Steps[] = {
		{ TEXT("GatherSetup"),      TEXT("R\u00e9glages") },
		{ TEXT("SpawnPath"),        TEXT("Trac\u00e9") },
		{ TEXT("BuildKms"),         TEXT("Kms") },
		{ TEXT("SpawnRunners"),     TEXT("Coureurs") },
		{ TEXT("SpawnPois"),        TEXT("POI") },
		{ TEXT("SpawnCheckpoints"), TEXT("Checkpoints") },
	};

	struct FRaceCard
	{
		int64 RaceId = 0;
		FText Title;
		TArray<TPair<int32, FLoadingTaskInfo>> Steps; // ordre d'affichage, tache
	};

	// "SpawnRunners_200" -> ("SpawnRunners", 200)
	bool SplitTaskId(const FName Id, FString& OutPrefix, int64& OutRaceId)
	{
		const FString S = Id.ToString();
		int32 Sep;
		if (!S.FindLastChar(TEXT('_'), Sep)) return false;
		const FString Suffix = S.Mid(Sep + 1);
		if (Suffix.IsEmpty() || !Suffix.IsNumeric()) return false;
		OutPrefix = S.Left(Sep);
		LexFromString(OutRaceId, *Suffix);
		return true;
	}

	TSharedRef<SWidget> MakePill(const FText& Label, const FLinearColor& Color)
	{
		using namespace LoadingOverlayStyle;
		return SNew(SBorder)
			.BorderImage(Rounded(Color.CopyWithNewOpacity(0.16f), 10.f))
			.Padding(FMargin(8.f * S, 2.f * S))
			[
				SNew(STextBlock)
				.Font(Font("Bold", 8.f))
				.ColorAndOpacity(Color)
				.Text(Label)
			];
	}

	TSharedRef<SWidget> MakeStep(const FText& Label, const FLoadingTaskInfo& Task)
	{
		using namespace LoadingOverlayStyle;
		const FLinearColor Color = StateColor(Task.State);
		return SNew(SBorder)
			.BorderImage(Rounded(Color.CopyWithNewOpacity(0.10f), 8.f, Color.CopyWithNewOpacity(0.45f), 1.f))
			.Padding(FMargin(8.f * S, 4.f * S))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, 6.f * S, 0.f)
				[
					SNew(SBox)
					.WidthOverride(8.f * S)
					.HeightOverride(8.f * S)
					[
						SNew(SBorder).BorderImage(Rounded(Color, 4.f))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(Font("Regular", 9.f))
					.ColorAndOpacity(Text)
					.Text(Task.State == ELoadingTaskState::Running && Task.Progress01 > 0.f && Task.Progress01 < 1.f
						? FText::Format(FText::FromString(TEXT("{0}  {1}%")), Label, FText::AsNumber(FMath::RoundToInt(Task.Progress01 * 100.f)))
						: Label)
				]
			];
	}
}

void SLoadingOverlay::Construct(const FArguments& InArgs)
{
	using namespace LoadingOverlayStyle;
	LoadingSubsystem = InArgs._Loading;

	// Abonnement aux changements
	if (LoadingSubsystem.IsValid())
	{
		ChangedHandle = LoadingSubsystem->OnChanged.AddSP(SharedThis(this), &SLoadingOverlay::HandleChanged);
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Scrim)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(24.f * S)
		[
			SNew(SBox)
			.WidthOverride(980.f * S)
			.MaxDesiredHeight(1000.f * S)
			[
				SNew(SBorder)
				.BorderImage(Rounded(CardFill, 18.f, CardBorder, 1.f))
				.Padding(28.f * S)
				[
					SNew(SVerticalBox)

					// En-tete : titre, sous-titre, pourcentage
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Font(Font("Bold", 22.f))
								.ColorAndOpacity(Text)
								.Text(FText::FromString(TEXT("Initialisation")))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 2.f * S, 0.f, 0.f)
							[
								SAssignNew(SummaryText, STextBlock)
								.Font(Font("Regular", 10.f))
								.ColorAndOpacity(Muted)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SAssignNew(PercentText, STextBlock)
							.Font(Font("Bold", 26.f))
							.ColorAndOpacity(Accent)
						]
					]

					// Progression globale
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 14.f * S, 0.f, 18.f * S)
					[
						SNew(SBox)
						.HeightOverride(8.f * S)
						[
							SNew(SProgressBar)
							.Style(&ProgressStyle())
							.FillColorAndOpacity(Accent)
							.Percent(this, &SLoadingOverlay::GetOverallPercent)
						]
					]

					// Une carte par course
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(CardsBox, SVerticalBox)
						]
					]
				]
			]
		]
	];

	Rebuild();
}

SLoadingOverlay::~SLoadingOverlay()
{
	if (LoadingSubsystem.IsValid() && ChangedHandle.IsValid())
	{
		LoadingSubsystem->OnChanged.Remove(ChangedHandle);
	}
}

void SLoadingOverlay::HandleChanged()
{
	bDirty = true;
}

void SLoadingOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (bDirty && InCurrentTime - LastRebuildTime >= 0.1)
	{
		LastRebuildTime = InCurrentTime;
		Rebuild();
	}
}

void SLoadingOverlay::Rebuild()
{
	using namespace LoadingOverlayStyle;
	bDirty = false;
	if (!CardsBox.IsValid()) return;
	CardsBox->ClearChildren();

	const TArray<FLoadingTaskInfo> Snapshot = LoadingSubsystem.IsValid() ? LoadingSubsystem->GetSnapshot() : TArray<FLoadingTaskInfo>();

	// Regroupement par course ; les taches sans course (chargement de la liste) a part
	TMap<int64, FRaceCard> Cards;
	TArray<FLoadingTaskInfo> Global;
	int32 Done = 0, Failed = 0;
	for (const FLoadingTaskInfo& Task : Snapshot)
	{
		if (Task.State == ELoadingTaskState::Success) ++Done;
		if (Task.State == ELoadingTaskState::Failed) { ++Done; ++Failed; }

		FString Prefix;
		int64 RaceId = 0;
		if (!SplitTaskId(Task.Id, Prefix, RaceId))
		{
			Global.Add(Task);
			continue;
		}
		FRaceCard& Card = Cards.FindOrAdd(RaceId);
		Card.RaceId = RaceId;
		int32 Order = UE_ARRAY_COUNT(Steps);
		for (int32 i = 0; i < UE_ARRAY_COUNT(Steps); ++i)
		{
			if (Prefix == Steps[i].Prefix) { Order = i; break; }
		}
		if (Prefix == TEXT("GatherSetup")) Card.Title = Task.Label;
		Card.Steps.Emplace(Order, Task);
	}

	const int32 Total = Snapshot.Num();
	SummaryText->SetText(Total == 0
		? FText::FromString(TEXT("Connexion au serveur de course\u2026"))
		: FText::FromString(FString::Printf(TEXT("%d course%s  \u00b7  %d / %d \u00e9tapes termin\u00e9es%s"),
			Cards.Num(), Cards.Num() > 1 ? TEXT("s") : TEXT(""), Done, Total,
			Failed > 0 ? *FString::Printf(TEXT("  \u00b7  %d en erreur"), Failed) : TEXT(""))));
	PercentText->SetText(FText::FromString(FString::Printf(TEXT("%d %%"), FMath::RoundToInt(GetOverallPercent().Get(0.f) * 100.f))));

	// Taches globales (liste des courses) : une ligne discrete au-dessus des cartes
	for (const FLoadingTaskInfo& Task : Global)
	{
		CardsBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 10.f * S)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f * S, 0.f)
			[
				SNew(SBox).WidthOverride(8.f * S).HeightOverride(8.f * S)
				[
					SNew(SBorder).BorderImage(Rounded(StateColor(Task.State), 4.f))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(Font("Regular", 10.f))
				.ColorAndOpacity(Muted)
				.Text(Task.Detail.IsEmpty() ? Task.Label : FText::Format(FText::FromString(TEXT("{0} \u2014 {1}")), Task.Label, Task.Detail))
			]
		];
	}

	TArray<int64> RaceIds;
	Cards.GetKeys(RaceIds);
	RaceIds.Sort();

	for (const int64 RaceId : RaceIds)
	{
		FRaceCard& Card = Cards[RaceId];
		Card.Steps.Sort([](const TPair<int32, FLoadingTaskInfo>& A, const TPair<int32, FLoadingTaskInfo>& B) { return A.Key < B.Key; });

		// Etat de la course : erreur si une etape a echoue, prete si tout est fini, sinon en cours
		bool bAnyFailed = false, bAllDone = true;
		const FLoadingTaskInfo* Focus = nullptr; // etape a detailler : erreur, sinon celle en cours
		for (const TPair<int32, FLoadingTaskInfo>& Step : Card.Steps)
		{
			const FLoadingTaskInfo& T = Step.Value;
			if (T.State == ELoadingTaskState::Failed) { bAnyFailed = true; if (!Focus || Focus->State != ELoadingTaskState::Failed) Focus = &T; }
			if (T.State != ELoadingTaskState::Success && T.State != ELoadingTaskState::Failed) bAllDone = false;
			if (T.State == ELoadingTaskState::Running && !Focus) Focus = &T;
		}
		const FLinearColor RaceColor = bAnyFailed ? Fail : (bAllDone ? Ok : Run);
		const FText RaceState = FText::FromString(bAnyFailed ? TEXT("Erreur") : (bAllDone ? TEXT("Pr\u00eate") : TEXT("En cours")));

		TSharedRef<SWrapBox> StepsBox = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(8.f * S, 8.f * S));
		for (const TPair<int32, FLoadingTaskInfo>& Step : Card.Steps)
		{
			FString Prefix;
			int64 Unused;
			SplitTaskId(Step.Value.Id, Prefix, Unused);
			const FText Label = Step.Key < UE_ARRAY_COUNT(Steps) ? FText::FromString(Steps[Step.Key].Label) : FText::FromString(Prefix);
			StepsBox->AddSlot()[ MakeStep(Label, Step.Value) ];
		}

		CardsBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f * S)
		[
			SNew(SBorder)
			.BorderImage(Rounded(FLinearColor(1.f, 1.f, 1.f, 0.03f), 12.f, CardBorder, 1.f))
			.Padding(FMargin(16.f * S, 12.f * S))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(Font("Bold", 13.f))
						.ColorAndOpacity(Text)
						.Text(Card.Title.IsEmpty() ? FText::FromString(FString::Printf(TEXT("Course %lld"), RaceId)) : Card.Title)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 10.f * S, 0.f)
					[
						SNew(STextBlock)
						.Font(Font("Regular", 9.f))
						.ColorAndOpacity(Muted)
						.Text(FText::FromString(FString::Printf(TEXT("#%lld"), RaceId)))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						MakePill(RaceState, RaceColor)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 10.f * S, 0.f, 0.f)
				[
					StepsBox
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 8.f * S, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Font(Font("Regular", 9.f))
					.ColorAndOpacity(Focus ? StateColor(Focus->State) : Muted)
					.AutoWrapText(true)
					.Visibility(Focus && !Focus->Detail.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)
					.Text(Focus ? Focus->Detail : FText::GetEmpty())
				]
			]
		];
	}
}

TOptional<float> SLoadingOverlay::GetOverallPercent() const
{
	return LoadingSubsystem.IsValid() ? LoadingSubsystem->GetOverallProgress01() : 0.f;
}
