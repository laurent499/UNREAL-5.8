// Copyright LTV Prod 2026. All Rights Reserved

#include "SlateNotificationsBFL.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void USlateNotificationsBFL::SlateNotify(const FText& NotificationText, const EMessageType& MessageType)
{
#if WITH_EDITOR
	FNotificationInfo Info(NotificationText);
	Info.ExpireDuration = 2.0f;
	Info.bUseCopyToClipboard = true;

	switch (MessageType)
	{
	case EMessageType::Error :
		Info.Image = FCoreStyle::Get().GetBrush("Icons.ErrorWithColor");
		break;

	case EMessageType::Success:
		Info.Image = FCoreStyle::Get().GetBrush("Icons.SuccessWithColor");
		break;
	}
	FSlateNotificationManager::Get().AddNotification(Info);
#endif
	
}
