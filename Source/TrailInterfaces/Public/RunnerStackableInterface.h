#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RunnerStackableInterface.generated.h"

UINTERFACE()
class TRAILINTERFACES_API URunnerStackableInterface : public UInterface
{
	GENERATED_BODY()
};

class TRAILINTERFACES_API IRunnerStackableInterface
{
	GENERATED_BODY()

public:
	// Doit retourner le composant "Hook" placé en haut du pied (point d'attache pour empiler).
	virtual USceneComponent* GetStackHookComponent() const = 0;

	// Composant à attacher/détacher (par défaut RootComponent si tu veux).
	virtual USceneComponent* GetStackAttachComponent() const = 0;

	// Le subsystem appelle ça pour que ton code d’update ignore le SetActorTransform quand stacké.
	virtual void SetIsStacked(bool bInStacked) = 0;
	virtual bool IsStacked() const = 0;
};
