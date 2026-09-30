// Copyright LTV Prod 2026. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SlateNotificationsBFL.generated.h"

UENUM(BlueprintType)
enum class EMessageType : uint8
{
	Success UMETA(DisplayName = "Success"),
	Error UMETA(DisplayName = "Error")
};
UCLASS()
class SLATENOTIFICATIONS_API USlateNotificationsBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (Category = "SlateNotifications"))
	static void SlateNotify(const FText& NotificationText, const EMessageType& MessageType);
};
