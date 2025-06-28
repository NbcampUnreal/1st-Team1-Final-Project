#include "UI/Screen/GS_UserIDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState.h"
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

void UGS_UserIDWidget::SetupWidget(AGS_PlayerState* PlayerState)
{
	if (PlayerState)
	{
		if (TEXT_UserID)
		{
			TEXT_UserID->SetText(FText::FromString(PlayerState->GetPlayerName()));
		}
		if (SteamAvatar && PlayerState->MySteamAvatar)
		{
			SteamAvatar->SetBrushFromTexture(PlayerState->MySteamAvatar);
		}
		else if (SteamAvatar)
		{
			PlayerState->FetchMySteamAvatar();
			if (PlayerState->MySteamAvatar)
			{
				SteamAvatar->SetBrushFromTexture(PlayerState->MySteamAvatar);
			}
		}
	}
}
