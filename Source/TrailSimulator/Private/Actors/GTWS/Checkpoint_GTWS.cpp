// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/GTWS/Checkpoint_GTWS.h"

#include "RaceManager.h"
#include "GTWS/GTWSStyle.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

void ACheckpoint_GTWS::BeginPlay()
{
	Super::BeginPlay();
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &ACheckpoint_GTWS::UpdateDayNight);
	}
}

void ACheckpoint_GTWS::UpdateCheckpoint(FRaceCheckpoint NewCheckpointDatas, FRaceSetup NewRaceSetup)
{
	Super::UpdateCheckpoint(NewCheckpointDatas, NewRaceSetup);
	
	// MainPicto
	if (MainPicto)
	{
		const FString Type = CheckpointDatas.type; 
		if (Type.IsEmpty()) return;
		const FString PictoPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/GTWS/%s.%s"), *Type, *Type);
		
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
		FootComponent->SetColorParameterValueOnMaterials("BaseColor", FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
	}
	
	// Name
	if (NameText)
	{
		NameText->SetTextRenderColor(FColor(255, 255, 255));
		NameText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	if (NameBkgComponent)
	{		
		NameBkgComponent->SetCustomPrimitiveDataVector4(0, GTWSStyle::GetBkgColor());
		NameBkgComponent->SetCustomPrimitiveDataFloat(5,1.f);

	}
	
	// infos
	if (InfosBkgComponent)
	{
		InfosBkgComponent->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex("#ffffffff")));
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
		
		const FString DistPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/GTWS/loc.loc"));
		
		UTexture2D* DistTex = LoadObject<UTexture2D>(
		nullptr,
		*DistPath);
		
		UMaterialInterface* DistMat = DistPicto->GetMaterial(0);
		UMaterialInstanceDynamic* DistMID = DistPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, DistMat);
		if (!DistMID) return;

		DistMID->SetTextureParameterValue(TEXT("Photo"), DistTex);
		DistMID->SetVectorParameterValue(TEXT("Color"), FLinearColor::FromSRGBColor(FColor::FromHex("#000000ff")));
		
		const FString AltPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/GTWS/peak.peak"));
		
		UTexture2D* AltTex = LoadObject<UTexture2D>(
		nullptr,
		*AltPath);
		
		UMaterialInterface* AltMat = AltPicto->GetMaterial(0);
		UMaterialInstanceDynamic* AltMID = AltPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, AltMat);
		if (!AltMID) return;

		AltMID->SetTextureParameterValue(TEXT("Photo"), AltTex);
		AltMID->SetVectorParameterValue(TEXT("Color"), FLinearColor::FromSRGBColor(FColor::FromHex("#000000ff")));
	}
	
	if (DistText)
	{
		DistText->SetTextRenderColor(FColor(0, 0, 0, 255));
		DistText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (AltText)
	{
		AltText->SetTextRenderColor(FColor(0, 0, 0, 255));
		AltText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	// Line1
	if (Line1Component)
	{
		Line1Component->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.0625f);
		Line1Component->SetCustomPrimitiveDataFloat(5, 1.f);
	}
	
	// WeatherBkg
	if (WeatherBkgComponent)
	{
		WeatherBkgComponent->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex("#ffffffff")));
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);

	}
	
	if (TempText)
	{
		TempText->SetTextRenderColor(FColor(0, 0, 0, 255));
		TempText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather1Text)
	{
		Weather1Text->SetTextRenderColor(FColor(0, 0, 0, 255));
		Weather1Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather2Text)
	{
		Weather2Text->SetTextRenderColor(FColor(0, 0, 0, 255));
		Weather2Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather3Text)
	{
		Weather3Text->SetTextRenderColor(FColor(0, 0, 0, 255));
		Weather3Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	// Line2
	if (Line2Component)
	{
		Line2Component->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
		Line2Component->SetCustomPrimitiveDataFloat(4, 0.0625f);
		Line2Component->SetCustomPrimitiveDataFloat(5, 1.f);
	}
}

void ACheckpoint_GTWS::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	
	// Chaque MID doit être créé sur son propre composant, sinon les paramètres
	// n'atteignent pas le picto et le slot 0 de Line2Component se fait écraser
	MainPictoMat = MainPicto->GetMaterial(0);
	MainPictoMID = MainPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MainPictoMat);
	MainPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);

	DistPictoMat = DistPicto->GetMaterial(0);
	DistPictoMID = DistPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, DistPictoMat);
	DistPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);

	AltPictoMat = AltPicto->GetMaterial(0);
	AltPictoMID = AltPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, AltPictoMat);
	AltPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);

	WeatherPictoMat = WeatherPicto->GetMaterial(0);
	WeatherPictoMID = WeatherPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, WeatherPictoMat);
	WeatherPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);
	
	if (bIsDay)
	{
		FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
		MainPictoMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.5, 1.5, 1.5, 1));
		NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f );
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
