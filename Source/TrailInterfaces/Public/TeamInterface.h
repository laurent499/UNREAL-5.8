// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TeamInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTeamInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TRAILINTERFACES_API ITeamInterface
{
	GENERATED_BODY()

public:
	
	virtual void AssignRunnerToFollow(int64 IdRunner) = 0;
};
