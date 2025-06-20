#include "UI/Screen/GS_UserIDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "System/GS_PlayerState.h"

void UGS_UserIDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerState* PS = GetOwningPlayerState();
	if (IsValid(PS))
	{
		if (TEXT_UserID)
		{
			TEXT_UserID->SetText(FText::FromString(GetOwningPlayerState()->GetPlayerName()));
		}
		if (SteamAvatar)
		{
			AGS_PlayerState* GPS = Cast<AGS_PlayerState>(GetOwningPlayerState());
			if (GPS && GPS->MySteamAvatar)
			{
				SteamAvatar->SetBrushFromTexture(GPS->MySteamAvatar);
			}
		}
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGS_UserIDWidget::DelayedUpdateUserID, 0.3f, true);
	}
}

void UGS_UserIDWidget::DelayedUpdateUserID()
{
	APlayerState* PS = GetOwningPlayerState();
	if (IsValid(PS))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		if (TEXT_UserID)
		{
			TEXT_UserID->SetText(FText::FromString(GetOwningPlayerState()->GetPlayerName()));
		}
		if (SteamAvatar)
		{
			AGS_PlayerState* GPS = Cast<AGS_PlayerState>(GetOwningPlayerState());
			if (GPS && GPS->MySteamAvatar)
			{
				SteamAvatar->SetBrushFromTexture(GPS->MySteamAvatar);
			}
		}
	}
}
