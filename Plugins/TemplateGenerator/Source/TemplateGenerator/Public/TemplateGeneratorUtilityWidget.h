// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Engine/EngineTypes.h"
#include "TemplateGeneratorUtilityWidget.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TEMPLATEGENERATOR_API UTemplateGeneratorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	
	/** Dossier dans /Game où sera créé RunnerTemplate (default: "/Game/RunnerTemplates") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TemplateGenerator")
	FDirectoryPath TargetFolder;

	/** Classe parent du Blueprint à créer (class Runner for default or other already created Runner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TemplateGenerator", meta=(AllowAbstract="false"))
	TSubclassOf<AActor> TeamParentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TemplateGenerator", meta=(AllowAbstract="false"))
	TSubclassOf<AActor> PoiParentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TemplateGenerator", meta=(AllowAbstract="false"))
	TSubclassOf<AActor> CheckpointParentClass;

	/** Fonction appelée depuis le widget (Click on button) */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "TemplateGenerator")
	void CreateTeamTemplate();
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "TemplateGenerator")
	void CreatePoiTemplate();
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "TemplateGenerator")
	void CreateCheckpointTemplate();
};