// TrailTextWidgetComponent.cpp

#include "Components/TrailTextWidgetComponent.h"

#include "Fonts/FontMeasure.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateColor.h"
#include "Widgets/Text/STextBlock.h"

UTrailTextWidgetComponent::UTrailTextWidgetComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetWidgetSpace(EWidgetSpace::World);

	SetDrawSize(FVector2D(2048.f, 128.f));
	// SetDrawAtDesiredSize(false);
	SetDrawAtDesiredSize(true);
	
	// tests flicking
	bUseAttachParentBound = false;
	bIsTwoSided = true;
	SetTickWhenOffscreen(false);

	SetPivot(FVector2D(0.5f, 0.5f));

	SetBlendMode(EWidgetBlendMode::Masked);
	SetBackgroundColor(FLinearColor::Transparent);

	SetManuallyRedraw(true);
	SetTickMode(ETickMode::Disabled);
	
	CurrentFont = FCoreStyle::GetDefaultFontStyle(
		FName(TEXT("Regular")),
		32.f
	);
}

void UTrailTextWidgetComponent::UpdateDrawSizeFromText()
{
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	const TSharedRef<FSlateFontMeasure> FontMeasureService =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	const FVector2D TextSize =
		FontMeasureService->Measure(CurrentText, CurrentFont);

	// const FVector2D Padding(32.f, 16.f);
	const FVector2D Padding(10.f, 0.f);

	const FVector2D NewDrawSize(
		FMath::FloorToFloat(TextSize.X + Padding.X * 2.f),
		FMath::FloorToFloat(TextSize.Y + Padding.Y * 2.f)
	);

	SetDrawSize(NewDrawSize);
}

void UTrailTextWidgetComponent::OnRegister()
{
	Super::OnRegister();
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BuildSlateWidgetIfNeeded();
}

void UTrailTextWidgetComponent::BuildSlateWidgetIfNeeded()
{
	if (!SlateTextBlock.IsValid())
	{
		SAssignNew(SlateTextBlock, STextBlock)
			.Font(CurrentFont)
			.Text(CurrentText)
			.Justification(CurrentJustification)
			.ColorAndOpacity(FSlateColor(CurrentColor));
	}

	SetSlateWidget(SlateTextBlock);
	RequestRenderUpdate();
}

void UTrailTextWidgetComponent::SetLabelText(const FText& NewText)
{
	CurrentText = NewText;

	if (SlateTextBlock.IsValid())
	{
		SlateTextBlock->SetText(CurrentText);
		// UpdateDrawSizeFromText();
		RequestRenderUpdate();
	}
}

void UTrailTextWidgetComponent::SetLabelCasseToUpper(const FText& NewText)
{
	CurrentText = NewText;
	if (SlateTextBlock.IsValid())
	{
		SlateTextBlock->SetText(CurrentText.ToUpper());
		RequestRenderUpdate();
	}
}

void UTrailTextWidgetComponent::SetLabelCasseToLower(const FText& NewText)
{
	CurrentText = NewText;
	if (SlateTextBlock.IsValid())
	{
		SlateTextBlock->SetText(CurrentText.ToLower());
		RequestRenderUpdate();
	}
}


void UTrailTextWidgetComponent::SetLabelColor(const FLinearColor& NewColor)
{
	CurrentColor = NewColor;

	if (SlateTextBlock.IsValid())
	{
		SlateTextBlock->SetColorAndOpacity(
			FSlateColor(CurrentColor)
		);

		RequestRenderUpdate();
	}
}

void UTrailTextWidgetComponent::SetLabelFont(const FSlateFontInfo& NewFont)
{
	CurrentFont = NewFont;

	if (SlateTextBlock.IsValid())
	{
		SlateTextBlock->SetFont(CurrentFont);
		RequestRenderUpdate();
	}
}

void UTrailTextWidgetComponent::SetLabelHJustification(const ETextJustify::Type& NewJustification)
{
	CurrentJustification = NewJustification;
	if (SlateTextBlock.IsValid())
	{
		SlateTextBlock->SetJustification(NewJustification);
		RequestRenderUpdate();
	}
}

FSlateFontInfo UTrailTextWidgetComponent::GetFontInfo()
{
	return CurrentFont;
}
