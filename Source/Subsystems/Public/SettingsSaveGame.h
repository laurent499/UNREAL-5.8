// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TrailSharedTypes.h"
#include "SettingsSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class SUBSYSTEMS_API USettingsSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, SaveGame)
	TMap<int64, FSettings> SettingsMap;
	UPROPERTY(VisibleAnywhere, SaveGame)
	FString MainURL;
	UPROPERTY(VisibleAnywhere, SaveGame)
	float GlobalPitch;
	UPROPERTY(VisibleAnywhere, SaveGame)
	float GlobalLength;
	UPROPERTY(VisibleAnywhere, SaveGame)
	float GlobalZAnchor;
	/** 1 = ZOffset en reglage fin au-dessus du trace recale sur les tuiles (0 = anciennes valeurs, compensant le geoide) */
	UPROPERTY(VisibleAnywhere, SaveGame)
	int32 ZOffsetVersion = 0;
};
