// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/FlagComponent.h"
#include "GTWS_FlagComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRAILSIMULATOR_API UGTWS_FlagComponent : public UFlagComponent
{
	GENERATED_BODY()

public:
	UGTWS_FlagComponent(const FObjectInitializer& ObjectInitializer);
	
};
