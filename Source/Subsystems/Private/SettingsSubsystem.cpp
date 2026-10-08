// All Rights Reserved

#include "SettingsSubsystem.h"

#include "SettingsSaveGame.h"
#include "SlateNotificationsBFL.h"
#include "TrailSharedTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TrailHttpDebug.h"

void USettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Load(); // charge la save
	
	// LookAt
	LookAtIntervalSeconds = 0.05f;
	LookAtInterpSpeed = 12.0f;
	
	// UI Updates
	UpdateIntervalSeconds = 1.0f;
	
	// Weather
	WeatherFrequency = 60.f /*1 mn*/ * 30.f; 
}

bool USettingsSubsystem::Load()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex))
		{
			CachedSave = Cast<USettingsSaveGame>(Loaded);
			if (CachedSave)
			{
				TrailSettingsMap = CachedSave->SettingsMap;
				TrailMainURL = CachedSave->MainURL;
				TrailGlobalPitch = CachedSave->GlobalPitch;
				TrailGlobalArmLength = CachedSave->GlobalLength;
				TrailGlobalZAnchor = CachedSave->GlobalZAnchor;
				// Les anciens ZOffset (50 a 115 m) compensaient l'altitude GPS ; le trace est maintenant recale
				// sur les tuiles, ils le feraient flotter : remise a zero une seule fois
				if (CachedSave->ZOffsetVersion < 1)
				{
					for (TPair<int64, FSettings>& Pair : TrailSettingsMap)
					{
						Pair.Value.ZOffset = 0.f;
					}
					Save();
				}
				return true;
			}
		}
	}
	// si pas de save, on crée un objet vide
	CachedSave = Cast<USettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass()));

	return CachedSave != nullptr;
}

bool USettingsSubsystem::Save()
{
	if (!CachedSave)
	{
		Load(); // tente de créer si absent
		if (!CachedSave) return false;
	}
	CachedSave->SettingsMap = TrailSettingsMap;
	CachedSave->MainURL = TrailMainURL;
	CachedSave->GlobalPitch = TrailGlobalPitch;
	CachedSave->GlobalLength = TrailGlobalArmLength;
	CachedSave->GlobalZAnchor = TrailGlobalZAnchor;
	CachedSave->ZOffsetVersion = 1;
	if (UGameplayStatics::SaveGameToSlot(CachedSave, SlotName, UserIndex))
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Saving Settings done"))), EMessageType::Success);
		return true;
		
	} else
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Saving Settings failed"))), EMessageType::Error);
		return false;
	}
}

/**
 * @brief Creating the FSettings entry for a Race
 * @param NewRaceID 
 * @param NewTrailSettings 
 */
void USettingsSubsystem::CreateTrailSettingsById(int64 NewRaceID, const FSettings& NewTrailSettings)
{
	FSettings* TmpSettings = TrailSettingsMap.Find(NewRaceID);
	if (!TmpSettings)
	{
		TrailSettingsMap.Add(NewRaceID, NewTrailSettings);
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Settings for Race %lld saved"), NewRaceID)), EMessageType::Success);
		Save();
	} else
	{
		*TmpSettings = NewTrailSettings;
	}
}

bool USettingsSubsystem::DoesSettingsExist(int64 RaceID)
{
	const FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp) return false;
	return true;
}

/**
 * @brief Getting the Settings for a Race
 * @param RaceID 
 * @return 
 */
FSettings USettingsSubsystem::GetTrailSettingsById(int64 RaceID) const
{
	const FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return FSettings();
	}
	return *Tmp;
}

/** 
 * @brief Set the Fetch value by Race 
 * @param FetchValue 
 * @param RaceID 
 */
void USettingsSubsystem::SetFetchById(float FetchValue, int64 RaceID)
{
	FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[FetchUpdate] No Settings for Race %lld"), RaceID)), EMessageType::Error);
		return;
	}
	Tmp->FetchFrequency = FetchValue;
	TrailSettingsMap.Add(RaceID, *Tmp);
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("[FetchUpdate] Race %lld - Fetch: %f"), RaceID, FetchValue)), EMessageType::Success);
	Save();
	OnFetchUpdate.Broadcast(FetchValue, RaceID);
}

/**
 * @brief Get the Fetch value by Race
 * @param RaceID 
 * @return 
 */
float USettingsSubsystem::GetFetchById(int64 RaceID) const
{
	const FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return -1;
	}
	return Tmp->FetchFrequency;
}

/**
 * @brief Set the ZOffset value by Race
 * @param ZOffsetValue 
 * @param RaceID 
 */
void USettingsSubsystem::SetZOffsetById(float ZOffsetValue, int64 RaceID)
{
	FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return;
	}
	Tmp->ZOffset = ZOffsetValue;
	TrailSettingsMap.Add(RaceID, *Tmp);
	Save();
	OnZOffsetUpdate.Broadcast(ZOffsetValue, RaceID);
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("ZOffset for Race %lld saved"), RaceID)), EMessageType::Success);
}

/**
 * @brief Get the ZOffset value by Race
 * @param RaceID 
 * @return 
 */
float USettingsSubsystem::GetZOffsetById(int64 RaceID) const
{
	const FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return -1;
	}
	return Tmp->ZOffset;
}

/**
 * @brief Set the Glow value by Race
 * @param NewGlowValue 
 * @param RaceID 
 */
void USettingsSubsystem::SetGlowById(float NewGlowValue, int64 RaceID)
{
	FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return;
	}
	Tmp->GlowValue = NewGlowValue;
	TrailSettingsMap.Add(RaceID, *Tmp);
	Save();
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Glow for Race %lld saved"), RaceID)), EMessageType::Success);
}

/**
 * @brief Get the Glow value by Race
 * @param RaceID 
 * @return 
 */
float USettingsSubsystem::GetGlowById(int64 RaceID) const
{
	const FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return -1;
	}
	return Tmp->GlowValue;
}

/**
 * @brief Set the Pulse value by Race
 * @param NewPulseFrequency 
 * @param RaceID 
 */
void USettingsSubsystem::SetPulseFrequencyById(float NewPulseFrequency, int64 RaceID)
{
	FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return;
	}
	Tmp->PulseFrequency = NewPulseFrequency;
	TrailSettingsMap.Add(RaceID, *Tmp);
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Pulse for Race %lld saved"), RaceID)), EMessageType::Success);
	Save();
}

/**
 * @brief Set the Fetch value by Race
 * @param RaceID 
 * @return 
 */
float USettingsSubsystem::GetPulseFrequencyById(int64 RaceID) const
{
	const FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return -1;
	}
	return Tmp->PulseFrequency;
}

/**
 * @brief Set the Pulse value by Race
 * @param  
 * @param NewPulseGlow
 * @param RaceID 
 */
void USettingsSubsystem::SetPulseGlowById(float NewPulseGlow, int64 RaceID)
{
	FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return;
	}
	Tmp->PulseGlowValue = NewPulseGlow;
	TrailSettingsMap.Add(RaceID, *Tmp);
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Pulse for Race %lld saved"), RaceID)), EMessageType::Success);
	Save();
}

/**
 * @brief Set the MinMax value by Race
 * @param MinValue 
 * @param MaxValue 
 * @param RaceID 
 */
void USettingsSubsystem::SetMinMaxById(float MinValue, float MaxValue, int64 RaceID)
{
	FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return;
	}
	Tmp->MinMax = FMinMax(FVector(MinValue), FVector(MaxValue));
	TrailSettingsMap.Add(RaceID, *Tmp);
	OnMinMaxSet.Broadcast(MinValue, MaxValue, RaceID);
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("MinMax for Race %lld saved"), RaceID)), EMessageType::Success);
	Save();
}

/**
 * @brief Get the MinMax value by Race
 * @param RaceID 
 * @return 
 */
FMinMax USettingsSubsystem::GetMinMaxById(int64 RaceID) const
{
	const FSettings* Tmp = TrailSettingsMap.Find(RaceID);
	if (!Tmp)
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("No Settings found for Race %lld"), RaceID)), EMessageType::Error);
		return FMinMax();
	}
	return Tmp->MinMax;
}

void USettingsSubsystem::SetMainURL(FString NewUrl)
{
	if (NewUrl.IsEmpty())
	{
		USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("%s is empty"), *NewUrl)), EMessageType::Error);
		return;
	}
	TrailMainURL = NewUrl;
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("MainURL %s is saved"), *NewUrl)), EMessageType::Success);
	
	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
	CachedSave = Cast<USettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass()));
	Save();
}

/**
 * @brief Get the LookAt interval
 * @return 
 */
float USettingsSubsystem::GetUpdateIntervalSeconds() const
{
	return LookAtIntervalSeconds;
}

/**
 * @brief Getting the Interp speed
 * @return 
 */
float USettingsSubsystem::GetInterpSpeed() const
{
	return LookAtInterpSpeed;
}

/**
 * @brief Getting the Weather fetch frequency
 * @return 
 */
float USettingsSubsystem::GetWeatherFrequency() const
{
	return WeatherFrequency;
}

void USettingsSubsystem::SetCameraPitch(float NewPitch)
{
	TrailGlobalPitch = NewPitch;
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("Pitch %f is saved"), NewPitch)), EMessageType::Success);
	Save();
	OnPitchChanged.Broadcast(NewPitch);
}

float USettingsSubsystem::GetCameraPitch() const
{
	return TrailGlobalPitch;
}

void USettingsSubsystem::SetArmLength(float NewLength)
{
	TrailGlobalArmLength = NewLength;
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("ArmLength %f is saved"), NewLength)), EMessageType::Success);
	Save();
	OnLengthChanged.Broadcast(NewLength);
}

float USettingsSubsystem::GetArmLength() const
{
	return TrailGlobalArmLength;
}

void USettingsSubsystem::SetZAnchor(float NewZAnchor)
{
	TrailGlobalZAnchor = NewZAnchor;
	USlateNotificationsBFL::SlateNotify(FText::FromString(FString::Printf(TEXT("ZAnchor %f is saved"), NewZAnchor)), EMessageType::Success);
	Save();
	OnZAnchorChanged.Broadcast(NewZAnchor);
}

float USettingsSubsystem::GetZAnchor()
{
	return TrailGlobalZAnchor;
}