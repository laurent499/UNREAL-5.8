// TrailTextWidgetComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Fonts/SlateFontInfo.h"
#include "TrailTextWidgetComponent.generated.h"

class STextBlock;

UCLASS(
	ClassGroup = (UserInterface),
	meta = (BlueprintSpawnableComponent)
)
class TRAILSIMULATOR_API UTrailTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UTrailTextWidgetComponent(
		const FObjectInitializer& ObjectInitializer
	);
	void UpdateDrawSizeFromText();
	UFUNCTION()
	void SetLabelText(const FText& NewText);
	UFUNCTION()
	void SetLabelCasseToUpper(const FText& NewText);
	UFUNCTION()
	void SetLabelCasseToLower(const FText& NewText);
	UFUNCTION()
	void SetLabelColor(const FLinearColor& NewColor);
	UFUNCTION()
	void SetLabelFont(const FSlateFontInfo& NewFont);
	UFUNCTION()
	void SetLabelHJustification(const ETextJustify::Type& NewJustification);
	FSlateFontInfo GetFontInfo();
	virtual void OnRegister() override;
	
	UPROPERTY(EditAnywhere)
	FText CurrentText;
	UPROPERTY(EditAnywhere)
	FLinearColor CurrentColor = FLinearColor::White;
	UPROPERTY(EditAnywhere)
	FSlateFontInfo CurrentFont;
	UPROPERTY(EditAnywhere)
	float CurrentSize;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ETextJustify::Type> CurrentJustification = ETextJustify::Center;

private:
	void BuildSlateWidgetIfNeeded();

	/*
	 * Ce pointeur ne doit surtout pas être marqué UPROPERTY.
	 * Un STextBlock n'est pas un UObject.
	 * Slate utilise des shared pointers.
	 */
	TSharedPtr<STextBlock> SlateTextBlock;

	
};