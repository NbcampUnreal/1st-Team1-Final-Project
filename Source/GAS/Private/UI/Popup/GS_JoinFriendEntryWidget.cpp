#include "UI/Popup/GS_JoinFriendEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h" // FOnlineUserPresence 사용을 위해 추가

void UGS_JoinFriendEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UGS_JoinFriendEntryWidget::OnJoinButtonPressed);
	}
}

void UGS_JoinFriendEntryWidget::SetupFriendEntry(const TSharedRef<FOnlineFriend>& InFriendData)
{
	FriendIdString = InFriendData->GetUserId()->ToString();

	if (FriendNameText)
	{
		FriendNameText->SetText(FText::FromString(InFriendData->GetDisplayName()));
	}
	const FOnlineUserPresence& Presence = InFriendData->GetPresence();
	const bool bIsJoinable = Presence.bIsJoinable;

	if (JoinButton)
	{
		JoinButton->SetIsEnabled(bIsJoinable);
	}

	if (OnlineStatusImage)
	{
		const EOnlinePresenceState::Type OnlineState = Presence.Status.State;

		if (OnlineState == EOnlinePresenceState::Offline)
		{
			OnlineStatusImage->SetColorAndOpacity(FLinearColor::Red);
		}
		else
		{
			OnlineStatusImage->SetColorAndOpacity(bIsJoinable ? FLinearColor::Green : FLinearColor(0.8f, 0.8f, 0.0f));
		}
	}
}

void UGS_JoinFriendEntryWidget::OnJoinButtonPressed()
{
	if (!FriendIdString.IsEmpty())
	{
		OnJoinRequest.Broadcast(FriendIdString);
	}
}