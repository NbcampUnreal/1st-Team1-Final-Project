#include "UI/Popup/GS_FriendListEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"

void UGS_FriendListEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InviteButton)
    {
        InviteButton->OnClicked.AddDynamic(this, &UGS_FriendListEntryWidget::OnInviteButtonPressed);
    }
}

void UGS_FriendListEntryWidget::SetupFriendEntry(const TSharedRef<FOnlineFriend>& InFriendData)
{
    if (!InFriendData->GetUserId()->IsValid())
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    FriendId = InFriendData->GetUserId();

    if (FriendNameText)
    {
        FriendNameText->SetText(FText::FromString(InFriendData->GetDisplayName()));
    }

    const FOnlineUserPresence& Presence = InFriendData->GetPresence();
    bool bIsOnline = Presence.bIsOnline;

    bool bIsInThisGame = Presence.bIsPlayingThisGame;
    bool bCanBeInvited = bIsOnline && bIsInThisGame;

    if (OnlineStatusImage)
    {
        OnlineStatusImage->SetVisibility(ESlateVisibility::Visible);
        OnlineStatusImage->SetColorAndOpacity(bIsOnline ? FLinearColor::Green : FLinearColor(0.3f, 0.3f, 0.3f));
    }

    if (InviteButton)
    {
        InviteButton->SetIsEnabled(bCanBeInvited);
        InviteButton->SetVisibility(bCanBeInvited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UGS_FriendListEntryWidget::OnInviteButtonPressed()
{
    if (FriendId.IsValid())
    {
        OnInviteButtonClicked.Broadcast(this);
    }
}