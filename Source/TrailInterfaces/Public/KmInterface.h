// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KmInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UKmInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TRAILINTERFACES_API IKmInterface
{
	GENERATED_BODY()
public:
	virtual void UpdateKm(int64 RaceID, const FText& NewLabel) const = 0;
	virtual void UpdateDayNight(bool bIsDay) = 0;
public:
};
