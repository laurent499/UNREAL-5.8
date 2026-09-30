// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PhotoComponent.generated.h"

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UPhotoComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UPhotoComponent();

	UFUNCTION()
	void UpdatePhoto(const FString& PhotoPath, const FVector& Scale);
	UFUNCTION()
	void ClearPhoto(const FString& PhotoPath, const FVector& Scale);
	virtual void TogglePhoto(bool bShow) const;

	UFUNCTION()
	void UpdateDayNight(bool bIsDay);
private:
	// Crée le MID une seule fois et le réutilise
	class UMaterialInstanceDynamic* EnsurePhotoMID();

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseMat;
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> PhotoMID = nullptr;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<class UStaticMeshComponent> PhotoSMC;

	// URL déjà appliquée : évite de retélécharger à chaque cycle de fetch
	UPROPERTY()
	FString CurrentPhotoUrl;
	// URL en cours de téléchargement : évite les requêtes concurrentes sur la même photo
	UPROPERTY()
	FString PendingPhotoUrl;
	// Référence forte sur la texture courante, sinon le GC peut la ramasser
	UPROPERTY()
	TObjectPtr<class UTexture2D> CurrentPhotoTexture;

};
