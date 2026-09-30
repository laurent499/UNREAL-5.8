// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PathInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPathInterface : public UInterface
{
	GENERATED_BODY()
};

/**
* 
*/
class TRAILINTERFACES_API IPathInterface
{
	GENERATED_BODY()

public:
	// virtual void DrawPath(FRacePath RacePath) = 0;
	virtual void ChangePathVisibility(bool bShow) = 0;
	virtual void ChangeSlopeVisibility(bool bShow) = 0;
	
	virtual FVector GetClosestSplineLocation(FVector RunnerLocation) const = 0;
	virtual FVector GetLocationAtDistance(float Distance) const = 0;
	virtual float GetDistanceAlongSpline(FVector RunnerLocation) const = 0;
};
