// Copyright LTVProd 2026 All Rights Reserved.


#include "Components/GTWS/GTWS_ClubComponent.h"

#include "GTWS/GTWSStyle.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UGTWS_ClubComponent::UGTWS_ClubComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ClubTextComponent->SetTextRenderColor(FColor::White);

	// GTWS_SplineMeshClub arrive avec MI_Black (parent M_Master), qui ignore les custom
	// primitive data. On lui pose M_Plain, le master utilisé par les fonds de nom, pour que
	// la couleur du template pilote ce fond comme les deux autres.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlainMat(
		TEXT("/Game/LTVContent/Materials/Masters/M_Plain.M_Plain"));
	if (PlainMat.Succeeded())
	{
		ClubBkgComponent->SetMaterial(0, PlainMat.Object);
	}

	ClubBkgComponent->SetCustomPrimitiveDataVector4(0, GTWSStyle::GetBkgColor());
	ClubBkgComponent->SetCustomPrimitiveDataFloat(4, 0.5f);	// Illum
	ClubBkgComponent->SetCustomPrimitiveDataFloat(5, 1.f);	// Alpha
}

void UGTWS_ClubComponent::UpdateClub(const FRunnerStruct& RunnerDatas)
{
	Super::UpdateClub(RunnerDatas);
	// Forcer le texte du club en blanc
	if (ClubTextComponent)
	{
		ClubTextComponent->SetTextRenderColor(FColor::White);
	}
}
