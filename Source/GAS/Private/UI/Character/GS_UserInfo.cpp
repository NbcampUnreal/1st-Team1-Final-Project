#include "UI/Character/GS_UserInfo.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState.h"
#include "System/GS_PlayerState.h"
#include "System/SteamAvatarHelper.h"
#include "TimerManager.h"

void UGS_UserInfo::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(FetchAvatarTimerHandle);
	Super::NativeDestruct();
}

void UGS_UserInfo::PollAvatar() // 이렇게 폴링 형식 안 쓰면 처음 로비 들어갔을 때 흰색 이미지로 뜸
{
	if (!AssociatedPlayerState.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FetchAvatarTimerHandle);
		return;
	}

	AGS_PlayerState* PlayerState = AssociatedPlayerState.Get();

	if (PlayerState->MySteamAvatar)
	{
		SteamAvatar->SetBrushFromTexture(PlayerState->MySteamAvatar);
		GetWorld()->GetTimerManager().ClearTimer(FetchAvatarTimerHandle);
		return;
	}

	const FUniqueNetIdRepl PlayerNetId(PlayerState->GetUniqueId());
	if (PlayerNetId.IsValid())
	{
		if (UTexture2D* AvatarTexture = USteamAvatarHelper::GetSteamAvatar(PlayerNetId, ESteamAvatarSize::SteamAvatar_Large))
		{
			SteamAvatar->SetBrushFromTexture(AvatarTexture);
			GetWorld()->GetTimerManager().ClearTimer(FetchAvatarTimerHandle);
		}
		// 아직 다운로드 중이면 다음 주기에 다시 시도
	}
}

void UGS_UserInfo::SetupWidget(AGS_PlayerState* PlayerState)
{
	if (PlayerState)
	{
		AssociatedPlayerState = PlayerState;
		
		if (Text_UserID)
		{
			Text_UserID->SetText(FText::FromString(PlayerState->GetPlayerName()));
		}
		if (SteamAvatar)
		{
			SteamAvatar->SetBrushFromTexture(nullptr);
		
			GetWorld()->GetTimerManager().ClearTimer(FetchAvatarTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				FetchAvatarTimerHandle,
				this,
				&UGS_UserInfo::PollAvatar,
				0.1f,
				true,
				0.0f
			);
		}
	}
}
