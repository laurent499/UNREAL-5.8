// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/FlagComponent.h"
#include "UTMB_FlagComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UUTMB_FlagComponent : public UFlagComponent
{
	GENERATED_BODY()
	
public:
	UUTMB_FlagComponent(const FObjectInitializer& ObjectInitializer);
	
};
