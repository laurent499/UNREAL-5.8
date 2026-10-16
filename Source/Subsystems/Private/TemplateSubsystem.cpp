// Copyright LTVProd 2026 All Rights Reserved.

#include "TemplateSubsystem.h"
#include "SettingsSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformHttp.h"

void UTemplateSubsystem::Deinitialize()
{
	for (auto& Pair : ActiveRequests)
	{
		Pair.Value->OnProcessRequestComplete().Unbind();
		Pair.Value->CancelRequest();
	}
	ActiveRequests.Empty();
	Super::Deinitialize();
}

void UTemplateSubsystem::RequestTemplate(const FString& TemplateName, TFunction<void()> OnDone)
{
	if (TemplateName.IsEmpty() || Templates.Contains(TemplateName))
	{
		if (!Templates.Contains(TemplateName))
		{
			Templates.Add(TemplateName, GetBuiltin(TemplateName));
		}
		OnDone();
		return;
	}
	Fetch(TemplateName, [OnDone = MoveTemp(OnDone)](bool) { OnDone(); });
}

void UTemplateSubsystem::ReloadTemplate(const FString& TemplateName)
{
	if (TemplateName.IsEmpty()) return;

	TWeakObjectPtr<UTemplateSubsystem> WeakThis(this);
	Fetch(TemplateName, [WeakThis, TemplateName](bool bOk)
	{
		if (bOk && WeakThis.IsValid())
		{
			WeakThis->OnTemplateChanged.Broadcast(TemplateName);
		}
	});
}

void UTemplateSubsystem::Fetch(const FString& TemplateName, TFunction<void(bool)> OnDone)
{
	USettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<USettingsSubsystem>() : nullptr;
	if (!Settings || Settings->GetMainURL().IsEmpty())
	{
		OnDone(false);
		return;
	}

	const FString Url = Settings->GetMainURL() + TEXT("template/") + FGenericPlatformHttp::UrlEncode(TemplateName);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(Url);
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	// Un template absent ne doit pas retarder longtemps le chargement de la course
	Req->SetTimeout(5.f);

	ActiveRequests.Add(TemplateName, Req);
	TWeakObjectPtr<UTemplateSubsystem> WeakThis(this);
	Req->OnProcessRequestComplete().BindLambda(
		[WeakThis, TemplateName, OnDone = MoveTemp(OnDone)](FHttpRequestPtr, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (!WeakThis.IsValid()) return;
			UTemplateSubsystem* Self = WeakThis.Get();
			Self->ActiveRequests.Remove(TemplateName);

			bool bOk = false;
			if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
			{
				FTrailTemplate Parsed;
				if (FJsonObjectConverter::JsonObjectStringToUStruct(Response->GetContentAsString(), &Parsed, 0, 0))
				{
					Parsed.Name = TemplateName;
					Self->Templates.Add(TemplateName, MoveTemp(Parsed));
					bOk = true;
				}
			}
			if (!bOk)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Template] %s indisponible sur la régie (code %d), valeurs d'origine utilisées"),
					*TemplateName, Response.IsValid() ? Response->GetResponseCode() : -1);
				// Mis en cache pour ne pas reconstruire les valeurs d'origine à chaque coureur ;
				// un template déjà lu reste en place si seul le rechargement échoue
				if (!Self->Templates.Contains(TemplateName))
				{
					Self->Templates.Add(TemplateName, GetBuiltin(TemplateName));
				}
			}
			// Le callback HTTP arrive déjà sur le game thread
			OnDone(bOk);
		});

	if (!Req->ProcessRequest())
	{
		ActiveRequests.Remove(TemplateName);
		OnDone(false);
	}
}

FTrailTemplate UTemplateSubsystem::GetTemplate(const FString& TemplateName) const
{
	if (const FTrailTemplate* Found = Templates.Find(TemplateName))
	{
		return *Found;
	}
	return GetBuiltin(TemplateName);
}

FString UTemplateSubsystem::GetBase(const FRaceSetup& RaceSetup) const
{
	return GetTemplate(RaceSetup.templateName).Base;
}

FTrailTemplate UTemplateSubsystem::GetForRace(const UObject* WorldContext, const FRaceSetup& RaceSetup)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (const UTemplateSubsystem* Self = GI ? GI->GetSubsystem<UTemplateSubsystem>() : nullptr)
	{
		return Self->GetTemplate(RaceSetup.templateName);
	}
	return GetBuiltin(RaceSetup.templateName);
}

float UTemplateSubsystem::GetRunnerHeight(const UObject* WorldContext, const FRaceSetup& RaceSetup)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (const UTemplateSubsystem* Self = GI ? GI->GetSubsystem<UTemplateSubsystem>() : nullptr)
	{
		if (const FTrailTemplate* Found = Self->Templates.Find(RaceSetup.templateName))
		{
			return Found->Runner.Height;
		}
	}
	return GetBuiltin(RaceSetup.templateName).Runner.Height;
}

const FTrailTemplate& UTemplateSubsystem::ResolveForRace(const UObject* WorldContext, const FRaceSetup& RaceSetup,
	TOptional<FTrailTemplate>& Storage)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (const UTemplateSubsystem* Self = GI ? GI->GetSubsystem<UTemplateSubsystem>() : nullptr)
	{
		if (const FTrailTemplate* Found = Self->Templates.Find(RaceSetup.templateName))
		{
			return *Found;
		}
	}
	Storage = GetBuiltin(RaceSetup.templateName);
	return Storage.GetValue();
}

/**
 * @brief Valeurs codées en dur avant l'éditeur, gardées en secours si la régie ne répond pas.
 * @note Mêmes valeurs que les templates d'origine créés par la régie (app/routers/templates.py).
 */
FTrailTemplate UTemplateSubsystem::GetBuiltin(const FString& TemplateName)
{
	FTrailTemplate T;
	T.Name = TemplateName;

	if (TemplateName == TEXT("GTWS"))
	{
		const FString Orange = TEXT("#FF3C00FF");
		T.Base = TEXT("GTWS");
		T.Globals.Primary = Orange;
		T.Globals.Secondary = TEXT("#FFFFFFFF");
		T.Globals.PictoSet = TEXT("GTWS");

		FTrailTemplateStack Stack;
		Stack.NameBkg = Orange;
		Stack.NameBorder = Orange;
		Stack.InfosBkg = TEXT("#FFFFFFFF");
		Stack.InfosPicto = TEXT("#000000FF");
		Stack.Line1 = TEXT("race");
		Stack.WeatherBkg = TEXT("#FFFFFFFF");
		T.Checkpoint = Stack;
		Stack.NameBorder = TEXT("race");
		T.Poi = Stack;

		T.Runner.NameBkg = TEXT("#FFFFFFFF");
		T.Runner.NameText = TEXT("#000000FF");
		T.Runner.ClubBkg = Orange;
		T.Runner.ClubText = TEXT("#FFFFFFFF");
		T.Runner.Height = 200.f;
	}
	else if (TemplateName == TEXT("UTMB") || TemplateName == TEXT("Nike"))
	{
		T.Base = TemplateName;
		T.Globals.PictoSet = TemplateName;
		T.Runner.Height = TemplateName == TEXT("UTMB") ? 230.f : 250.f;
	}
	else
	{
		// Generic, et tout nom inconnu comme avant l'éditeur
		T.Base = TEXT("Generic");
		T.Checkpoint.InfosPicto = TEXT("#FFFFFFFF");
		T.Poi.InfosPicto = TEXT("#FFFFFFFF");
	}

	return T;
}
