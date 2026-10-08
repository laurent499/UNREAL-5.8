// All Rights Reserved


#include "Components/PhotoComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"

UPhotoComponent::UPhotoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PhotoSMC = CreateDefaultSubobject<UStaticMeshComponent>(FName("PhotoSMC"));
	PhotoSMC->SetupAttachment(this);
	PhotoSMC->SetRelativeTransform(FTransform::Identity);
	PhotoSMC->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	PhotoSMC->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PhotoMesh(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/Photo_UTMB.Photo_UTMB"));
	if (PhotoMesh.Succeeded())
	{
		PhotoSMC->SetStaticMesh(PhotoMesh.Object);
	}
}

UMaterialInstanceDynamic* UPhotoComponent::EnsurePhotoMID()
{
	if (PhotoMID)
	{
		return PhotoMID;
	}

	if (!PhotoSMC)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsurePhotoMID failed: PhotoSMC is null."));
		return nullptr;
	}

	UMaterialInterface* BaseMatLocal = PhotoSMC->GetMaterial(0);
	if (!BaseMatLocal)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsurePhotoMID failed: no material on PhotoSMC."));
		return nullptr;
	}

	PhotoMID = PhotoSMC->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMatLocal);
	if (!PhotoMID)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsurePhotoMID failed: could not create MID."));
	}
	return PhotoMID;
}

void UPhotoComponent::UpdatePhoto(const FString& PhotoPath, const FVector& Scale)
{
	if (!PhotoSMC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdatePhoto failed: PhotoSMC is null."));
		return;
	}

	PhotoSMC->SetRelativeScale3D(Scale);

	if (!EnsurePhotoMID())
	{
		return;
	}

	// Coureur sans photo : le backend envoie null, que les runners transforment en "null.png".
	// Sans schema http(s), inutile de lancer une requete (libcurl echouait chaque seconde).
	if (!PhotoPath.StartsWith(TEXT("http://"), ESearchCase::IgnoreCase) && !PhotoPath.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase))
	{
		return;
	}

	// Le fetch runners rappelle UpdatePhoto à chaque cycle : ne retélécharger que sur changement d'URL
	if (PhotoPath == CurrentPhotoUrl && CurrentPhotoTexture)
	{
		return;
	}
	// Une requête est déjà en vol pour cette même URL
	if (PhotoPath == PendingPhotoUrl)
	{
		return;
	}
	PendingPhotoUrl = PhotoPath;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(PhotoPath);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Accept"), TEXT("image/png"));
	Req->SetHeader(TEXT("CF-Access-Client-Id"), TEXT("590a6f4ead75fa83b8f1c3c7b2962461.access"));
	Req->SetHeader(TEXT("CF-Access-Client-Secret"), TEXT("27f032892b5c8669ccce1edaba9e4a9b64964f53e7a4fc96ccf135a9cc6e7474"));

	Req->OnProcessRequestComplete().BindWeakLambda(this,
		[this, PhotoPath](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			// Libère l'URL en vol quelle que soit l'issue, sinon un échec bloquerait tout réessai
			if (PendingPhotoUrl == PhotoPath)
			{
				PendingPhotoUrl.Reset();
			}

			if (!bWasSuccessful || !Response.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Download failed (no response)."));
				return;
			}

			const int32 Code = Response->GetResponseCode();
			if (Code < 200 || Code >= 300)
			{
				UE_LOG(LogTemp, Warning, TEXT("Download failed HTTP %d"), Code);
				return;
			}

			// const FString ContentType = Response->GetHeader(TEXT("Content-Type"));
			// if (!ContentType.Contains(TEXT("image/png")))
			// {
			// 	UE_LOG(LogTemp, Warning, TEXT("Expected image/png or image/jpeg, got %s"), *ContentType);
			// 	return;
			// }

			const TArray<uint8>& Bytes = Response->GetContent();
			if (Bytes.Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Download failed: empty payload."));
				return;
			}

			UTexture2D* Tex = FImageUtils::ImportBufferAsTexture2D(Bytes);
			if (!Tex)
			{
				UE_LOG(LogTemp, Warning, TEXT("photo decode failed."));
				return;
			}

			if (!EnsurePhotoMID())
			{
				return;
			}

			CurrentPhotoTexture = Tex;
			CurrentPhotoUrl = PhotoPath;
			PhotoMID->SetTextureParameterValue(TEXT("Photo"), Tex);
		});

	Req->ProcessRequest();
}

void UPhotoComponent::ClearPhoto(const FString& PhotoPath, const FVector& Scale)
{
}


void UPhotoComponent::TogglePhoto(bool bShow) const
{
	PhotoSMC->SetHiddenInGame(!bShow);
}

void UPhotoComponent::UpdateDayNight(bool bIsDay)
{
	if (!EnsurePhotoMID())
	{
		return;
	}

	// La photo ne reagit ni a la lumiere ni a l'heure : meme rendu de jour comme de nuit
	// (le materiau est Unlit, exposition compensee). Avant : teinte x0,1 la nuit.
	PhotoMID->SetVectorParameterValue("Color", FVector(1.f, 1.f, 1.f));
	PhotoMID->SetScalarParameterValue("Illum", 1.f);
}
