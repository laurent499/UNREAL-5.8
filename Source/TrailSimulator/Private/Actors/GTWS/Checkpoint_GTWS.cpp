// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/GTWS/Checkpoint_GTWS.h"
#include "TemplateSubsystem.h"
#include "Actors/RoundedPanel.h"

#include "RaceManager.h"
#include "GTWS/GTWSStyle.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

/**
 * @brief Fond de panneau avec coins arrondis : M_PlainRounded (même rendu que M_Plain, qui
 *        ignore son masque d'opacité). Primitive data : 0-3 couleur, 5 alpha, 6 arrondi, 7 coins.
 */
static void UseRoundedBkg_CheckpointGTWS(UPrimitiveComponent* Comp)
{
	static TWeakObjectPtr<UMaterialInterface> Rounded;
	if (!Rounded.IsValid())
	{
		Rounded = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LTVContent/Materials/Masters/M_PlainRounded.M_PlainRounded"));
	}
	if (Comp && Rounded.IsValid() && Comp->GetMaterial(0) != Rounded.Get())
	{
		Comp->SetMaterial(0, Rounded.Get());
	}
}

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

	// Couleurs et pictos lus dans le template de la course (régie, page Templates)
	const FTrailTemplate Tpl = UTemplateSubsystem::GetForRace(this, NewRaceSetup);
	const FTrailTemplateStack& Style = Tpl.Checkpoint;
	auto Lin = [&NewRaceSetup](const FString& Value) { return FTrailTemplate::ToLinear(Value, NewRaceSetup.color); };
	auto Srgb = [&NewRaceSetup](const FString& Value) { return FTrailTemplate::ToSRGB(Value, NewRaceSetup.color); };
	MainPictoColor = Lin(Style.MainPicto);

	// Fonds arrondis : filets recentrés entre les fonds et raccourcis aux arrondis
	RoundedPanel::FitLine(Line1Component, NameBkgComponent, InfosBkgComponent, Tpl.Globals.CornerRadius);
	RoundedPanel::FitLine(Line2Component, InfosBkgComponent, WeatherBkgComponent, Tpl.Globals.CornerRadius);
	// Pictos distance et altitude un peu plus discrets dans leur bandeau (0,2 à l'origine)
	if (DistPicto) DistPicto->SetRelativeScale3D(FVector(0.16f));
	if (AltPicto) AltPicto->SetRelativeScale3D(FVector(0.16f));
	
	// MainPicto
	if (MainPicto)
	{
		const FString Type = CheckpointDatas.type; 
		if (Type.IsEmpty()) return;
		const FString PictoPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/%s/%s.%s"), *Tpl.Globals.PictoSet, *Type, *Type);
		
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
		FootComponent->SetColorParameterValueOnMaterials("BaseColor", Lin(Style.Foot));
	}
	
	// Name
	if (NameText)
	{
		NameText->SetTextRenderColor(Srgb(Style.NameText));
		NameText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	if (NameBkgComponent)
	{		
		NameBkgComponent->SetCustomPrimitiveDataVector4(0, Lin(Style.NameBkg));
		NameBkgComponent->SetCustomPrimitiveDataFloat(5,1.f);
		UseRoundedBkg_CheckpointGTWS(NameBkgComponent);
		NameBkgComponent->SetCustomPrimitiveDataFloat(6, Tpl.Globals.CornerRadius); // arrondi des coins (M_Plain)

	}
	
	// infos
	if (InfosBkgComponent)
	{
		InfosBkgComponent->SetCustomPrimitiveDataVector4(0, Lin(Style.InfosBkg));
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
		UseRoundedBkg_CheckpointGTWS(InfosBkgComponent);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(6, Tpl.Globals.CornerRadius); // arrondi des coins (M_Plain)
		
		const FString DistPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/%s/loc.loc"), *Tpl.Globals.PictoSet);
		
		UTexture2D* DistTex = LoadObject<UTexture2D>(
		nullptr,
		*DistPath);
		
		UMaterialInterface* DistMat = DistPicto->GetMaterial(0);
		UMaterialInstanceDynamic* DistMID = DistPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, DistMat);
		if (!DistMID) return;

		DistMID->SetTextureParameterValue(TEXT("Photo"), DistTex);
		DistMID->SetVectorParameterValue(TEXT("Color"), Lin(Style.InfosPicto));
		
		const FString AltPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/%s/peak.peak"), *Tpl.Globals.PictoSet);
		
		UTexture2D* AltTex = LoadObject<UTexture2D>(
		nullptr,
		*AltPath);
		
		UMaterialInterface* AltMat = AltPicto->GetMaterial(0);
		UMaterialInstanceDynamic* AltMID = AltPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, AltMat);
		if (!AltMID) return;

		AltMID->SetTextureParameterValue(TEXT("Photo"), AltTex);
		AltMID->SetVectorParameterValue(TEXT("Color"), Lin(Style.InfosPicto));
	}
	
	if (DistText)
	{
		DistText->SetTextRenderColor(Srgb(Style.InfosText));
		DistText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (AltText)
	{
		AltText->SetTextRenderColor(Srgb(Style.InfosText));
		AltText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	// Line1
	if (Line1Component)
	{
		Line1Component->SetCustomPrimitiveDataVector4(0, Lin(Style.Line1));
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.0625f);
		Line1Component->SetCustomPrimitiveDataFloat(5, 1.f);
	}
	
	// WeatherBkg
	if (WeatherBkgComponent)
	{
		WeatherBkgComponent->SetCustomPrimitiveDataVector4(0, Lin(Style.WeatherBkg));
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
		UseRoundedBkg_CheckpointGTWS(WeatherBkgComponent);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(6, Tpl.Globals.CornerRadius); // arrondi des coins (M_Plain)

	}
	
	if (TempText)
	{
		TempText->SetTextRenderColor(Srgb(Style.WeatherText));
		TempText->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather1Text)
	{
		Weather1Text->SetTextRenderColor(Srgb(Style.WeatherText));
		Weather1Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather2Text)
	{
		Weather2Text->SetTextRenderColor(Srgb(Style.WeatherText));
		Weather2Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	if (Weather3Text)
	{
		Weather3Text->SetTextRenderColor(Srgb(Style.WeatherText));
		Weather3Text->SetScalarParameterForCustomPrimitiveData("Illum", 0.125);
	}
	
	// Line2
	if (Line2Component)
	{
		Line2Component->SetCustomPrimitiveDataVector4(0, Lin(Style.Line2));
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
	MainPictoMID = MainPicto->CreateDynamicMaterialInstance(0, MainPictoMat);
	MainPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);

	DistPictoMat = DistPicto->GetMaterial(0);
	DistPictoMID = DistPicto->CreateDynamicMaterialInstance(0, DistPictoMat);
	DistPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);

	AltPictoMat = AltPicto->GetMaterial(0);
	AltPictoMID = AltPicto->CreateDynamicMaterialInstance(0, AltPictoMat);
	AltPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);

	WeatherPictoMat = WeatherPicto->GetMaterial(0);
	WeatherPictoMID = WeatherPicto->CreateDynamicMaterialInstance(0, WeatherPictoMat);
	WeatherPictoMID->SetScalarParameterValue(TEXT("Illum"), 1.f);
	
	if (bIsDay)
	{
		FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.5f);
		MainPictoMID->SetVectorParameterValue(TEXT("Color"), MainPictoColor);
		NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f );
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.5f);
		Line2Component->SetCustomPrimitiveDataFloat(4, 0.5f);
	} else
	{
		FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.025f);
		MainPictoMID->SetVectorParameterValue(TEXT("Color"), MainPictoColor);
		NameBkgComponent->SetCustomPrimitiveDataFloat(4, 0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.025f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.025f);
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.025f);
		Line2Component->SetCustomPrimitiveDataFloat(4, 0.025f);

	}
}
