#include "UI/Screen/GS_UserIDWidget.h"
#include "GameFramework/PlayerController.h"
#include "System/GS_PlayerState.h"

void UGS_UserIDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AGS_PlayerState* PS = GetOwningPlayerState<AGS_PlayerState>();
	if (IsValid(PS))
	{
		SetupWidget(PS);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGS_UserIDWidget::DelayedUpdateUserID, 0.3f, true);
	}
}

void UGS_UserIDWidget::DelayedUpdateUserID()
{
	AGS_PlayerState* PS = GetOwningPlayerState<AGS_PlayerState>();
	if (IsValid(PS))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		SetupWidget(PS);
	}
}
