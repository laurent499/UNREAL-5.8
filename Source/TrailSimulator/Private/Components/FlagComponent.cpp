// All Rights Reserved


#include "Components/FlagComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

UFlagComponent::UFlagComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	FlagSMC = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, FName("FlagSMC"));
	FlagSMC->SetupAttachment(this);
	FlagSMC->SetRelativeTransform(FTransform::Identity);
}

void UFlagComponent::UpdateFlag(const FString& Country)
{
	const FString NewCountry = Country.ToLower();
	// Le fetch runners rappelle UpdateFlag à chaque cycle : rien à refaire si le pays n'a pas changé
	if (NewCountry == CurrentCountry && FlagMID)
	{
		return;
	}

	FString FlagPath = TEXT("/Game/LTVContent/2D/flags/flags_square/") + NewCountry + TEXT(".") + NewCountry;

	if (UTexture2D* Tex = LoadObject<UTexture2D>(
	nullptr,
	*FlagPath))
	{
		if (!FlagMID)
		{
			UMaterialInterface* BaseMat = FlagSMC->GetMaterial(0);
			FlagMID = FlagSMC->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMat);
		}
		if (!FlagMID) return;
		FlagMID->SetTextureParameterValue(TEXT("FlagName"), Tex);
		CurrentCountry = NewCountry;
		ApplyDayNightBrightness();
	}
}

void UFlagComponent::ToggleFlag(bool bShow) const
{
	FlagSMC->SetHiddenInGame(!bShow);
}

void UFlagComponent::UpdateDayNight(bool bIsDay)
{
	bIsDayCached = bIsDay;
	ApplyDayNightBrightness();
}

void UFlagComponent::ApplyDayNightBrightness() const
{
	if (bIsDayCached)
	{
		FlagSMC->SetCustomPrimitiveDataFloat(16, 0.6f);
		FlagSMC->SetCustomPrimitiveDataFloat(20, 0.6f);
	} else
	{
		FlagSMC->SetCustomPrimitiveDataFloat(16, 0.125f);
		FlagSMC->SetCustomPrimitiveDataFloat(20, 0.125f);
	}
}
