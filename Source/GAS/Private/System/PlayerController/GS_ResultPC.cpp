#include "System/PlayerController/GS_ResultPC.h"
#include "Blueprint/UserWidget.h"
#include "UI/Screen/Result/GS_GameResultUI.h"

void AGS_ResultPC::Client_ShowResultUI_Implementation()
{
	if (!ResultUIClass) return;

	if (IsLocalController())
	{
		UGS_GameResultUI* ResultWidget = CreateWidget<UGS_GameResultUI>(this, ResultUIClass);
		if (ResultWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_ResultPC::Client_ShowResultUI_Implementation() - Trying to Add to Viewport"));
			ResultWidget->AddToViewport();
		}
	}
}
