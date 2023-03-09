#pragma  once

#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"

namespace DebugHeader
{
	static void Print(const FString& Message,const FColor& Color)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,8.f,Color,Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp,Warning,TEXT("%s"),*Message);
	}
	EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType,const FString & Message,bool bShowMsgAsWarning = true
		)
	{
		if(bShowMsgAsWarning)
		{
			FText title = FText::FromString("Wearning");
			return FMessageDialog::Open(MsgType,FText::FromString(Message),&title);
		}
		else
		{
			return FMessageDialog::Open(MsgType,FText::FromString(Message));
		}
	}

	void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 5.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}


}
