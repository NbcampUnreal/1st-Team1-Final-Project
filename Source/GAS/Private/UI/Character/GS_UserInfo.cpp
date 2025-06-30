#include "UI/Character/GS_UserInfo.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState.h"
#include "System/GS_PlayerState.h"

void UGS_UserInfo::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_UserInfo::SetupWidget(AGS_PlayerState* PlayerState)
{
	if (PlayerState)
	{
		if (Text_UserID)
		{
			Text_UserID->SetText(FText::FromString(PlayerState->GetPlayerName()));
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
