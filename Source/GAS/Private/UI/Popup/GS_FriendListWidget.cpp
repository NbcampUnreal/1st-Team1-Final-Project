#include "UI/Popup/GS_FriendListWidget.h"
#include "UI/Popup/GS_FriendListEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/CircularThrobber.h"
#include "Components/Overlay.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

void UGS_FriendListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UGS_FriendListWidget::HandleRefreshButtonClicked);
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UGS_FriendListWidget::HandleCloseButtonClicked);
    }

    HandleRefreshButtonClicked();
}

void UGS_FriendListWidget::FillFriendList(const TArray<TSharedRef<FOnlineFriend>>& Friends)
{
    ClearFriendList();

    if (!FriendListEntryWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("FriendListWidget: FriendListEntryWidgetClass is not set!"));
        return;
    }
    if (!FriendsScrollBox)
    {
        UE_LOG(LogTemp, Error, TEXT("FriendListWidget: FriendsScrollBox is not bound!"));
        return;
    }

    TArray<TSharedRef<FOnlineFriend>> SortedFriends = Friends;
    SortedFriends.Sort([](const TSharedRef<FOnlineFriend>& A, const TSharedRef<FOnlineFriend>& B) {
        bool bAIsOnline = A->GetPresence().bIsOnline;
        bool bBIsOnline = B->GetPresence().bIsOnline;
        if (bAIsOnline != bBIsOnline)
        {
            return bAIsOnline > bBIsOnline;
        }
        return A->GetDisplayName() < B->GetDisplayName();
    });

    for (const TSharedRef<FOnlineFriend>& FriendData : SortedFriends)
    {
        UGS_FriendListEntryWidget* EntryWidget = CreateWidget<UGS_FriendListEntryWidget>(this, FriendListEntryWidgetClass);
        if (EntryWidget)
        {
            EntryWidget->SetupFriendEntry(FriendData);
            EntryWidget->OnInviteButtonClicked.AddDynamic(this, &UGS_FriendListWidget::HandleFriendInviteButtonClicked);
            FriendsScrollBox->AddChild(EntryWidget);
        }
    }

    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UGS_FriendListWidget::ClearFriendList()
{
    if (FriendsScrollBox)
    {
        FriendsScrollBox->ClearChildren();
    }
}

void UGS_FriendListWidget::ShowLoadingIndicator()
{
    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(ESlateVisibility::Visible);
    }
}

void UGS_FriendListWidget::HandleFriendInviteButtonClicked(UGS_FriendListEntryWidget* ClickedEntry)
{
    if (!ClickedEntry || !ClickedEntry->GetFriendId().IsValid()) return;
	
	APlayerController* OwningPC = GetOwningPlayer();
	if (!OwningPC) return;

	ULocalPlayer* LP = OwningPC->GetLocalPlayer();
	if (!LP) return;

	IOnlineSubsystem* Subsys = IOnlineSubsystem::Get();
	if (Subsys)
	{
		IOnlineSessionPtr SessionInterface = Subsys->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->SendSessionInviteToFriend(*LP->GetPreferredUniqueNetId(), NAME_GameSession, *ClickedEntry->GetFriendId());
		}
	}
}

void UGS_FriendListWidget::HandleRefreshButtonClicked()
{
    ShowLoadingIndicator();

    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        if (IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface())
        {
            FOnReadFriendsListComplete OnReadFriendsListCompleteDelegate;
            OnReadFriendsListCompleteDelegate.BindUObject(this, &UGS_FriendListWidget::OnReadFriendsListComplete);

            FriendsInterface->ReadFriendsList(0, EFriendsLists::ToString(EFriendsLists::Default), OnReadFriendsListCompleteDelegate);
        }
        else
        {
            if (LoadingIndicator)
            {
                LoadingIndicator->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
    else
    {
        if (LoadingIndicator)
        {
            LoadingIndicator->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UGS_FriendListWidget::HandleCloseButtonClicked()
{
    if (UOverlay* ParentOverlay = Cast<UOverlay>(GetParent()))
    {
        ParentOverlay->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        RemoveFromParent();
    }
}

void UGS_FriendListWidget::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (bWasSuccessful)
    {
        if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
        {
            if (IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface())
            {
                TArray<TSharedRef<FOnlineFriend>> FriendsList;
                FriendsInterface->GetFriendsList(0, ListName, FriendsList);
                FillFriendList(FriendsList);
            }
        }
    }
    else
    {
        ClearFriendList();
    }
}
