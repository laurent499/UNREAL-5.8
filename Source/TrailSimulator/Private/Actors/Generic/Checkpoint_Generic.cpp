// Copyright LTV Prod 2026. All Rights Reserved


#include "Actors/Generic/Checkpoint_Generic.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"


// Sets default values
ACheckpoint_Generic::ACheckpoint_Generic()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameBkgMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newChk/Generic_Name.Generic_Name"));
	if (NameBkgMesh.Succeeded())
	{
		NameBkgComponent->SetStaticMesh(NameBkgMesh.Object);
	}
	
	NameLeftComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NameLeft"));
	NameLeftComponent->SetupAttachment(SplineHook);
	NameLeftComponent->SetRelativeTransform(FTransform::Identity);
	NameLeftComponent->SetMobility(EComponentMobility::Movable);
	NameLeftComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameLeftMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newChk/Chk_Generic_nom_L_bkg.Chk_Generic_nom_L_bkg"));
	if (NameLeftMesh.Succeeded())
	{
		NameLeftComponent->SetStaticMesh(NameLeftMesh.Object);
	}
	
	NameRightComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NameRight"));
	NameRightComponent->SetupAttachment(SplineHook);
	NameRightComponent->SetRelativeTransform(FTransform::Identity);
	NameRightComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NameRightMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newChk/Chk_Generic_nom_R_bkg.Chk_Generic_nom_R_bkg"));
	if (NameRightMesh.Succeeded())
	{
		NameRightComponent->SetStaticMesh(NameRightMesh.Object);
	}
}

void ACheckpoint_Generic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACheckpoint_Generic::UpdateCheckpoint(FRaceCheckpoint NewCheckpointDatas, FRaceSetup NewRaceSetup)
{
	Super::UpdateCheckpoint(NewCheckpointDatas, NewRaceSetup);
	
	// MainPicto
	if (MainPicto)
	{
		const FString Type = CheckpointDatas.type;
		if (Type.IsEmpty()) return;
		const FString PictoPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/Generic/%s.%s"), *Type, *Type);
		
		if(UTexture2D* Tex = LoadObject<UTexture2D>(
		nullptr,
		*PictoPath))
		{
			UMaterialInterface* BaseMat = MainPicto->GetMaterial(0);
			UMaterialInstanceDynamic* MID = MainPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMat);
			if (!MID) return;

			MID->SetTextureParameterValue(TEXT("Photo"), Tex);
		} else
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
	}
	
	if (NameBkgComponent)
	{		
		UMaterialInterface* BorderMat = NameBkgComponent->GetMaterial(0);
		UMaterialInstanceDynamic* BorderMID = NameBkgComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BorderMat);
		BorderMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
		BorderMID->SetScalarParameterValue(TEXT("Alpha"), 1.f);
		
		UMaterialInterface* CoreMat = NameBkgComponent->GetMaterial(1);
		UMaterialInstanceDynamic* MID = NameBkgComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(1, CoreMat);
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(FColor::FromHex("#000000ff")));
		MID->SetScalarParameterValue(TEXT("Alpha"), 0.95f);
	}
	
	if (NameRightComponent)
	{
		NameRightComponent->SetMobility(EComponentMobility::Movable);
		NameRightComponent->SetRelativeLocation(
			SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local));
		NameRightComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		NameRightComponent->SetRelativeScale3D(FVector( 1.f, 0.2f, 1.f));
		
		UMaterialInterface* BorderMat = NameRightComponent->GetMaterial(0);
		UMaterialInstanceDynamic* BorderMID = NameRightComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BorderMat);
		BorderMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
	}
	
	if (NameLeftComponent)
	{
		NameLeftComponent->SetMobility(EComponentMobility::Movable);
		NameLeftComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		NameLeftComponent->SetRelativeLocation(
			SplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local));
		NameLeftComponent->SetRelativeScale3D(FVector(1.f, 0.2f, 1.f));
		
		UMaterialInterface* BorderMat = NameLeftComponent->GetMaterial(0);
		UMaterialInstanceDynamic* BorderMID = NameLeftComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BorderMat);
		BorderMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
	}
	
	// infos
	if (InfosBkgComponent)
	{
		InfosBkgComponent->SetCustomPrimitiveDataVector4(0, FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4,0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
				
		const FString DistPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/Generic/loc.loc"));
		
		UTexture2D* DistTex = LoadObject<UTexture2D>(
		nullptr,
		*DistPath);
		
		UMaterialInterface* DistMat = DistPicto->GetMaterial(0);
		UMaterialInstanceDynamic* DistMID = DistPicto->CreateAndSetMaterialInstanceDynamicFromMaterial(0, DistMat);
		if (!DistMID) return;

		DistMID->SetTextureParameterValue(TEXT("Photo"), DistTex);
		DistMID->SetVectorParameterValue(TEXT("Color"), FLinearColor::FromSRGBColor(FColor::FromHex("#000000ff")));
		
		const FString AltPath = FString::Printf(
			TEXT("/Game/LTVContent/2D/Pictos/Generic/peak.peak"));
		
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
	}
	if (AltText)
	{
		AltText->SetTextRenderColor(FColor(0, 0, 0, 255));
	}
	
	// Line1
	if (Line1Component)
	{
		Line1Component->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex("#000000ff")));
		Line1Component->SetCustomPrimitiveDataFloat(4,0.0625f);
		Line1Component->SetCustomPrimitiveDataFloat(5, 1.f);
}
	
	// WeatherBkg
	if (WeatherBkgComponent)
	{
		WeatherBkgComponent->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4,0.0625f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);
	}
	
	if (TempText)
	{
		TempText->SetTextRenderColor(FColor(0, 0, 0, 255));
	}
	if (Weather1Text)
	{
		Weather1Text->SetTextRenderColor(FColor(0, 0, 0, 255));
	}
	if (Weather2Text)
	{
		Weather2Text->SetTextRenderColor(FColor(0, 0, 0, 255));
	}
	if (Weather3Text)
	{
		Weather3Text->SetTextRenderColor(FColor(0, 0, 0, 255));
	}
	
	// Line2
	if (Line2Component)
	{
		Line2Component->SetCustomPrimitiveDataVector4(0,FLinearColor::FromSRGBColor(FColor::FromHex(NewRaceSetup.color)));
		Line2Component->SetCustomPrimitiveDataFloat(4,0.0625f);
		Line2Component->SetCustomPrimitiveDataFloat(5, 1.f);

	}
}

void ACheckpoint_Generic::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	
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
		MainPictoMID->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
		NameBkgComponent->SetScalarParameterValueOnMaterials("Illum", 0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.5f);
		Line2Component->SetCustomPrimitiveDataFloat(4, 0.5f);
	} else
	{
		FootComponent->SetScalarParameterValueOnMaterials("Illum", 0.025f);
		MainPictoMID->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
		NameBkgComponent->SetScalarParameterValueOnMaterials("Illum", 0.0625f);
		InfosBkgComponent->SetCustomPrimitiveDataFloat(4, 0.025f);
		WeatherBkgComponent->SetCustomPrimitiveDataFloat(4, 0.025f);
		Line1Component->SetCustomPrimitiveDataFloat(4, 0.025f);
		Line2Component->SetCustomPrimitiveDataFloat(4, 0.025f);

	}
}