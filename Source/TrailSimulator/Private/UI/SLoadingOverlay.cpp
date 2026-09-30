#include "UI/SLoadingOverlay.h"
#include "LoadingStatusSubsystem.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"


static FSlateFontInfo MakeFont(int32 Size)
{
	return FCoreStyle::GetDefaultFontStyle("Regular", Size);
}

void SLoadingOverlay::Construct(const FArguments& InArgs)
{
	LoadingSubsystem = InArgs._Loading;
	// FontFace = InArgs._FontFace;

	// Abonnement aux changements
	if (LoadingSubsystem.IsValid())
	{
		ChangedHandle = LoadingSubsystem->OnChanged.AddSP(SharedThis(this), &SLoadingOverlay::HandleChanged);
	}

	RefreshItemsFromSubsystem();
	
	static constexpr float UI_SCALE = 3.f;

	const int32 TitleSize  = FMath::RoundToInt(18 * UI_SCALE);
	const int32 NormalSize = FMath::RoundToInt(12 * UI_SCALE);
	const int32 SmallSize  = FMath::RoundToInt(10 * UI_SCALE);

	const FSlateFontInfo FontTitle  = FCoreStyle::GetDefaultFontStyle("Regular", TitleSize);
	const FSlateFontInfo FontNormal = FCoreStyle::GetDefaultFontStyle("Regular", NormalSize);
	const FSlateFontInfo FontSmall  = FCoreStyle::GetDefaultFontStyle("Regular", SmallSize);

	const float Pad   = 16.f * UI_SCALE;
	const float PadSm = 8.f;
	const float PanelWidth = 780.f * UI_SCALE;

	const float ProgressH = 10.f * UI_SCALE;
	
	const float RowHeight = 34.f * UI_SCALE;   // choisis ta hauteur de ligne
	const float ListHeight = RowHeight * 10.f; // 10 rows visibles

	ChildSlot
	[
		SNew(SOverlay)
		
		// "Fond" (simple border) + panneau central
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(20.f * UI_SCALE)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.WidthOverride(PanelWidth)
				[
					SNew(SBorder)
					.Padding(Pad)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.5f))	
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Font(FontTitle)
							.Text(FText::FromString(TEXT("Initialisation...")))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, PadSm)
						[
							SNew(SBox)
							.HeightOverride(ProgressH)
							[
								SNew(SProgressBar)
								.Percent(this, &SLoadingOverlay::GetOverallPercent)
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, PadSm)
						[
							SNew(SSeparator)
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SAssignNew(ListView, SListView<FTaskItemPtr>)
							.ListItemsSource(&Items)
							.OnGenerateRow(this, &SLoadingOverlay::OnGenerateRow)
							
						]
					]
				]
			]
		]
	];
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
	RefreshItemsFromSubsystem();
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

void SLoadingOverlay::RefreshItemsFromSubsystem()
{
	Items.Reset();

	if (!LoadingSubsystem.IsValid())
		return;

	const TArray<FLoadingTaskInfo> Snapshot = LoadingSubsystem->GetSnapshot();
	Items.Reserve(Snapshot.Num());

	for (const FLoadingTaskInfo& T : Snapshot)
	{
		Items.Add(MakeShared<FLoadingTaskInfo>(T));
	}
}

TOptional<float> SLoadingOverlay::GetOverallPercent() const
{
	return LoadingSubsystem.IsValid() ? LoadingSubsystem->GetOverallProgress01() : 0.f;
}

FText SLoadingOverlay::StateToText(ELoadingTaskState State)
{
	switch (State)
	{
	case ELoadingTaskState::Queued:   
		return FText::FromString(TEXT("..."));
		break;
	case ELoadingTaskState::Running:  
		return FText::FromString(TEXT("RUN"));
		break;
	case ELoadingTaskState::Success:  
		return FText::FromString(TEXT("OK"));
		break;
	case ELoadingTaskState::Failed:   
		return FText::FromString(TEXT("FAIL"));
		break;
	default:                          
		return FText::FromString(TEXT("?"));
		break;
	}
}

TSharedRef<ITableRow> SLoadingOverlay::OnGenerateRow(FTaskItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	static constexpr float UI_SCALE = 3.f;
	const FSlateFontInfo FontNormal = MakeFont(FMath::RoundToInt(4 * UI_SCALE));
	const FSlateFontInfo FontSmall  = MakeFont(FMath::RoundToInt(2 * UI_SCALE));

	const float PadSm = 8.f;
	const float ProgressH = 4.f * UI_SCALE;
	
	const float RowHeight = 36.f * UI_SCALE;
	
	return SNew(STableRow<FTaskItemPtr>, OwnerTable)
	  [
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
	  	.Padding(PadSm, 0.f)
		[
		  SNew(SHorizontalBox)

		  + SHorizontalBox::Slot()
		  .AutoWidth()
			.Padding(0.f, 0.f, PadSm, 0.f)
		  [
			SNew(STextBlock)
			.Font(FontNormal)
			.Text(StateToText(Item->State))
		  	.ColorAndOpacity(Item->Color)
		  ]

		  + SHorizontalBox::Slot()
		  .FillWidth(1.f)
		  [
			SNew(STextBlock)
			.Font(FontNormal)
			.Text(Item->Label)
		  ]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, PadSm * 0.5f)
		[
		  SNew(SBox)
		  .HeightOverride(ProgressH)
		  [
		  	SNew(SProgressBar)
		  	.Percent(Item->Progress01)
			.FillColorAndOpacity(Item->Color)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, PadSm)
		[
		  SNew(STextBlock)
		  .Font(FontSmall)
		  .Text(Item->Detail)
			.ColorAndOpacity(Item->Color)
		]
	  ];
}
