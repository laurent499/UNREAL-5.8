// Copyright LTV Prod 2026. All Rights Reserved

#include "SlateNotificationsBFL.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void USlateNotificationsBFL::SlateNotify(const FText& NotificationText, const EMessageType& MessageType)
{
#if WITH_EDITOR
	// En jeu (UnrealEditor.exe -game), WITH_EDITOR vaut 1 mais chaque notification ouvre une fenetre Windows
	// redimensionnee a chaque image pendant son fondu : 40 a 90 ms par image sur ~1 s (trace Insights du 14/10).
	// Hors editeur, on se contente du journal.
	if (!GIsEditor)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *NotificationText.ToString());
		return;
	}

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
