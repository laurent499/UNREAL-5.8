// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/UTMB/Poi_UTMB.h"

#include "RaceManager.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

void APoi_UTMB::BeginPlay()
{
	Super::BeginPlay();
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &APoi_UTMB::UpdateDayNight);
	}
}


void APoi_UTMB::UpdatePoi(FRacePOI PoiData, FRaceSetup RaceSetup)
{
	Super::UpdatePoi(PoiData, RaceSetup);
	
	// MainPicto
	if (MainPicto)
	{
		const FString Type = PoiDatas.type; 
		if (Type.IsEmpty()) return;
		const FString PictoPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/UTMB/%s.%s"), *Type, *Type);
		
		if(UTexture2D* Tex = LoadObject<UTexture2D>(
		nullptr,
		*PictoPath))
		{
			UMaterialInterface* BaseMat = MainPicto->GetMaterial(0);
			UMaterialInstanceDynamic* MID = MainPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMat);
			if (!MID) return;

			MID->SetTextureParameterValue(TEXT("Photo"), Tex);
		}else
		{
			MainPicto->SetHiddenInGame(true);
		}
	}
	
	// Foot
	if (FootComponent)
	{
		UMaterialInterface* CoreMat = FootComponent->GetMaterial(0);
		UMaterialInstanceDynamic* MID = FootComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, CoreMat);
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
	}
	
	// Name
	if (NameText)
	{
		NameText->SetTextRenderColor(FColor(0, 0, 0, 255));
		NameText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	if (NameBkgComponent)
	{		
		NameBkgComponent->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
		NameBkgComponent->SetCustomPrimitiveDataFloat(5,1.f);
	}
	
	// infos
	if (InfosBkgComponent)
	{
		InfosBkgComponent->SetCustomPrimitiveDataVector4(4,FLinearColor::FromSRGBColor(FColor::FromHex("#000000ff")));
		InfosBkgComponent->SetCustomPrimitiveDataFloat(5, 0.8f);
		
		const FString AltPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/UTMB/peak.peak"));
		
		UTexture2D* AltTex = LoadObject<UTexture2D>(
		nullptr,
		*AltPath);
		
		UMaterialInterface* AltMat = AltPicto->GetMaterial(0);
		UMaterialInstanceDynamic* AltMID = AltPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, AltMat);
		if (!AltMID) return;

		AltMID->SetTextureParameterValue(TEXT("Photo"), AltTex);
		AltMID->SetVectorParameterValue(TEXT("Color"), FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
	}
	
	if (AltText)
	{
		AltText->SetTextRenderColor(FColor(255, 255, 255, 255));
		AltText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	// Line1
	if (Line1Component)
	{
		Line1Component->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
		Line1Component->SetCustomPrimitiveDataFloat(5, 1.f);

	}
	
	// WeatherBkg
	if (WeatherBkgComponent)
	{
		WeatherBkgComponent->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex("#000000ff")));
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(5, 0.8f);

	}
	
	if (TempText)
	{
		TempText->SetTextRenderColor(FColor(255, 255, 255, 255));
		TempText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather1Text)
	{
		Weather1Text->SetTextRenderColor(FColor(255, 255, 255, 255));
		Weather1Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather2Text)
	{
		Weather2Text->SetTextRenderColor(FColor(255, 255, 255, 255));
		Weather2Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather3Text)
	{
		Weather3Text->SetTextRenderColor(FColor(255, 255, 255, 255));
		Weather3Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	// Line2
	if (Line2Component)
	{
		Line2Component->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex(RaceSetup.color)));
		Line2Component->SetCustomPrimitiveDataFloat(5, 1.f);
	}
}

void APoi_UTMB::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	
	MainPictoMat = MainPicto->GetMaterial(0);
	MainPictoMID = Line2Component->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MainPictoMat);
	MainPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);
	
	AltPictoMat = AltPicto->GetMaterial(0);
	AltPictoMID = Line2Component->CreateAndSetMaterialInstanceDynamicFromMaterial(0, AltPictoMat);
	AltPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);
	
	WeatherPictoMat = WeatherPicto->GetMaterial(0);
	WeatherPictoMID = Line2Component->CreateAndSetMaterialInstanceDynamicFromMaterial(0, WeatherPictoMat);
	WeatherPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);
	
	if (bIsDay)
	{
		FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
		MainPictoMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.5, 1.5, 1.5, 1));
		NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.5f);
		Line2Component->SetCustomPrimitiveDataFloat(4, 0.5f);
	} else
	{
		FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.025f);
		MainPictoMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.1, 0.1, 0.1, 1));
		NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.025f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.025f);
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.025f);
		Line2Component->SetCustomPrimitiveDataFloat(4, 0.025f);
	}
}
