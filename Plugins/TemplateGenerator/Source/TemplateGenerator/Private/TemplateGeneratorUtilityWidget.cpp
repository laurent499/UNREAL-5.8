#include "TemplateGeneratorUtilityWidget.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"

void UTemplateGeneratorUtilityWidget::CreateTeamTemplate()
{
	if (!TeamParentClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RunnerTemplateUtilityWidget] TeamParentClass est null."));
		return;
	}

	// Récupère le dossier choisi (ou le défaut)
	FString FolderPath = TargetFolder.Path;
	if (FolderPath.IsEmpty())
	{
		FolderPath = TEXT("/Game/RunnerTemplates/");
	}

	// S'assurer que ça commence par /Game
	if (!FolderPath.IsEmpty() && !FolderPath.StartsWith(TEXT("/Game")))
	{
		if (FolderPath.StartsWith(TEXT("/")))
		{
			FolderPath = TEXT("/Game") + FolderPath;
		}
		else
		{
			FolderPath = TEXT("/Game/") + FolderPath;
		}
	}

	const FString AssetName = TEXT("RunnerTemplate");
	const FString PackagePath = FolderPath / AssetName;

	// Évite de recréer si ça existe déjà
	if (FPackageName::DoesPackageExist(PackagePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TemplateGeneratorUtilityWidget] Le package '%s' existe déjà, création annulée."), *PackagePath);
		return;
	}

	// Création du package
	UPackage* Package = CreatePackage(*PackagePath);
	if (!ensure(Package))
	{
		UE_LOG(LogTemp, Error, TEXT("[TemplateGeneratorUtilityWidget] Impossible de créer le package '%s'."), *PackagePath);
		return;
	}

	Package->FullyLoad();

	// Création du Blueprint basé sur RunnerParentClass
	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		TeamParentClass,
		Package,
		*AssetName,
		EBlueprintType::BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		FName(TEXT("RunnerTemplateUtility"))
	);

	if (!NewBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("[RunnerTemplateUtilityWidget] Échec de création du Blueprint RunnerTemplate."));
		return;
	}

	// Enregistrement auprès de l'Asset Registry
	FAssetRegistryModule::AssetCreated(NewBlueprint);
	NewBlueprint->MarkPackageDirty();

	if (GEditor)
	{
		TArray<UObject*> ObjectsToSync;
		ObjectsToSync.Add(NewBlueprint);
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}

	UE_LOG(LogTemp, Log, TEXT("[TemplateGeneratorUtilityWidget] Blueprint RunnerTemplate créé à '%s'."), *PackagePath);
}

void UTemplateGeneratorUtilityWidget::CreatePoiTemplate()
{
	if (!PoiParentClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RunnerTemplateUtilityWidget] PoiParentClass est null."));
		return;
	}

	// Récupère le dossier choisi (ou le défaut)
	FString FolderPath = TargetFolder.Path;
	if (FolderPath.IsEmpty())
	{
		FolderPath = TEXT("/Game/PoiTemplates/");
	}

	// S'assurer que ça commence par /Game
	if (!FolderPath.IsEmpty() && !FolderPath.StartsWith(TEXT("/Game")))
	{
		if (FolderPath.StartsWith(TEXT("/")))
		{
			FolderPath = TEXT("/Game") + FolderPath;
		}
		else
		{
			FolderPath = TEXT("/Game/") + FolderPath;
		}
	}

	const FString AssetName = TEXT("PoiTemplate");
	const FString PackagePath = FolderPath / AssetName;

	// Évite de recréer si ça existe déjà
	if (FPackageName::DoesPackageExist(PackagePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TemplateGeneratorUtilityWidget] Le package '%s' existe déjà, création annulée."), *PackagePath);
		return;
	}

	// Création du package
	UPackage* Package = CreatePackage(*PackagePath);
	if (!ensure(Package))
	{
		UE_LOG(LogTemp, Error, TEXT("[TemplateGeneratorUtilityWidget] Impossible de créer le package '%s'."), *PackagePath);
		return;
	}

	Package->FullyLoad();

	// Création du Blueprint basé sur RunnerParentClass
	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		PoiParentClass,
		Package,
		*AssetName,
		EBlueprintType::BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		FName(TEXT("PoiTemplateUtility"))
	);

	if (!NewBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("[PoiTemplateUtilityWidget] Échec de création du Blueprint PoiTemplate."));
		return;
	}

	// Enregistrement auprès de l'Asset Registry
	FAssetRegistryModule::AssetCreated(NewBlueprint);
	NewBlueprint->MarkPackageDirty();

	if (GEditor)
	{
		TArray<UObject*> ObjectsToSync;
		ObjectsToSync.Add(NewBlueprint);
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}

	UE_LOG(LogTemp, Log, TEXT("[TemplateGeneratorUtilityWidget] Blueprint PoiTemplate créé à '%s'."), *PackagePath);
}

void UTemplateGeneratorUtilityWidget::CreateCheckpointTemplate()
{
	if (!CheckpointParentClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RunnerTemplateUtilityWidget] CheckpointParentClass est null."));
		return;
	}

	// Récupère le dossier choisi (ou le défaut)
	FString FolderPath = TargetFolder.Path;
	if (FolderPath.IsEmpty())
	{
		FolderPath = TEXT("/Game/CheckpointTemplates/");
	}

	// S'assurer que ça commence par /Game
	if (!FolderPath.IsEmpty() && !FolderPath.StartsWith(TEXT("/Game")))
	{
		if (FolderPath.StartsWith(TEXT("/")))
		{
			FolderPath = TEXT("/Game") + FolderPath;
		}
		else
		{
			FolderPath = TEXT("/Game/") + FolderPath;
		}
	}

	const FString AssetName = TEXT("CheckpointTemplate");
	const FString PackagePath = FolderPath / AssetName;

	// Évite de recréer si ça existe déjà
	if (FPackageName::DoesPackageExist(PackagePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TemplateGeneratorUtilityWidget] Le package '%s' existe déjà, création annulée."), *PackagePath);
		return;
	}

	// Création du package
	UPackage* Package = CreatePackage(*PackagePath);
	if (!ensure(Package))
	{
		UE_LOG(LogTemp, Error, TEXT("[TemplateGeneratorUtilityWidget] Impossible de créer le package '%s'."), *PackagePath);
		return;
	}

	Package->FullyLoad();

	// Création du Blueprint basé sur RunnerParentClass
	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		CheckpointParentClass,
		Package,
		*AssetName,
		EBlueprintType::BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		FName(TEXT("CheckpointTemplateUtility"))
	);

	if (!NewBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("[CheckpointTemplateUtilityWidget] Échec de création du Blueprint CheckpointTemplate."));
		return;
	}

	// Enregistrement auprès de l'Asset Registry
	FAssetRegistryModule::AssetCreated(NewBlueprint);
	NewBlueprint->MarkPackageDirty();

	if (GEditor)
	{
		TArray<UObject*> ObjectsToSync;
		ObjectsToSync.Add(NewBlueprint);
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}

	UE_LOG(LogTemp, Log, TEXT("[TemplateGeneratorUtilityWidget] Blueprint CheckpointTemplate créé à '%s'."), *PackagePath);
}
