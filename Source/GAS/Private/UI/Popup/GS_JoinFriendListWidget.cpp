#include "UI/Popup/GS_JoinFriendListWidget.h"
#include "UI/Popup/GS_JoinFriendEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/CircularThrobber.h"
#include "Components/Overlay.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "System/GS_GameInstance.h"
#include "System/PlayerController/GS_MainMenuPC.h"
#include "System/GS_OnlineGameSubsystem.h"
#include "GameFramework/GameModeBase.h"

void UGS_JoinFriendListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetGameInstance())
	{
		OnlineSubsystem = GetGameInstance()->GetSubsystem<UGS_OnlineGameSubsystem>();
	}

	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UGS_JoinFriendListWidget: OnlineGameSubsystem을 찾을 수 없습니다!"));
		ShowLoadingIndicator(false);
		return;
	}

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UGS_JoinFriendListWidget::HandleRefreshButtonClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UGS_JoinFriendListWidget::HandleCloseButtonClicked);
	}

	OnlineSubsystem->OnFriendsReadComplete.AddUObject(this, &UGS_JoinFriendListWidget::OnFriendsListRead);
	OnlineSubsystem->OnJoinSuccess.AddUObject(this, &UGS_JoinFriendListWidget::OnJoinSucceeded);
	OnlineSubsystem->OnJoinFailure.AddUObject(this, &UGS_JoinFriendListWidget::OnJoinFailed);

	HandleRefreshButtonClicked();
}

void UGS_JoinFriendListWidget::NativeDestruct()
{
	if (OnlineSubsystem)
	{
		OnlineSubsystem->OnFriendsReadComplete.RemoveAll(this);
		OnlineSubsystem->OnJoinSuccess.RemoveAll(this);
		OnlineSubsystem->OnJoinFailure.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UGS_JoinFriendListWidget::HandleRefreshButtonClicked()
{
	if (OnlineSubsystem)
	{
		if (FriendsScrollBox)
		{
			FriendsScrollBox->ClearChildren();
		}
		ShowLoadingIndicator(true);
		OnlineSubsystem->ReadFriendsList();
	}
}

void UGS_JoinFriendListWidget::HandleCloseButtonClicked()
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

void UGS_JoinFriendListWidget::OnFriendsListRead(const TArray<TSharedRef<FOnlineFriend>>& FriendsList)
{
	ShowLoadingIndicator(false);

	if (!FriendListEntryWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("FriendListEntryWidgetClass가 설정되지 않았습니다!"));
		return;
	}
	if (!FriendsScrollBox) return;

	FriendsScrollBox->ClearChildren();

	struct FSortableFriendData
	{
		TSharedRef<FOnlineFriend> Friend;
		int32 SortCategory;
		FString DisplayName;
	};

	TArray<FSortableFriendData> SortableList;
	SortableList.Reserve(FriendsList.Num()); // 미리 메모리를 할당해 성능을 최적화

	for (const TSharedRef<FOnlineFriend>& Friend : FriendsList)
	{
		// 이 시점의 Presence 정보를 단 한번만 읽어서 스냅샷 생성
		const FOnlineUserPresence& Presence = Friend->GetPresence();

		int32 Category = 2; // 기본값: 오프라인
		if (Presence.bIsJoinable)
		{
			Category = 0; // 1순위: 참여 가능
		}
		else if (Presence.SessionId.IsValid() || Presence.Status.State != EOnlinePresenceState::Offline)
		{
			Category = 1; // 2순위: 온라인
		}


		const int32 PresenceStateInt = (int32)Presence.Status.State;
		UE_LOG(LogTemp, Warning, TEXT("Snapshotting Friend: %s | Category: %d | bIsJoinable: %s | PresenceState(int): %d"),
			*Friend->GetDisplayName(), Category, (Presence.bIsJoinable ? TEXT("True") : TEXT("False")), PresenceStateInt);


		SortableList.Add({ Friend, Category, Friend->GetDisplayName() });
	}

	SortableList.Sort([](const FSortableFriendData& A, const FSortableFriendData& B)
	{
		if (A.SortCategory != B.SortCategory)
		{
			// 주 정렬 기준: 미리 계산된 카테고리
			return A.SortCategory < B.SortCategory;
		}
		else
		{
			// 보조 정렬 기준: 이름순
			return A.DisplayName < B.DisplayName;
		}
	});

	for (const FSortableFriendData& SortedFriendData : SortableList)
	{
		UGS_JoinFriendEntryWidget* EntryWidget = CreateWidget<UGS_JoinFriendEntryWidget>(this, FriendListEntryWidgetClass);
		if (EntryWidget)
		{
			EntryWidget->SetupFriendEntry(SortedFriendData.Friend);
			EntryWidget->OnJoinRequest.AddDynamic(this, &UGS_JoinFriendListWidget::HandleJoinRequest);
			FriendsScrollBox->AddChild(EntryWidget);
		}
	}
}

void UGS_JoinFriendListWidget::HandleJoinRequest(const FString& FriendId)
{
	UE_LOG(LogTemp, Log, TEXT("JoinFriendListWidget: Join requested for friend ID: %s"), *FriendId);
	if (OnlineSubsystem)
	{
		ShowLoadingIndicator(true);
		OnlineSubsystem->JoinFriend(FriendId);
	}
}

void UGS_JoinFriendListWidget::OnJoinSucceeded()
{
	UE_LOG(LogTemp, Log, TEXT("Join Succeeded! Closing friend list widget."));
	ShowLoadingIndicator(false);
	RemoveFromParent();
}

void UGS_JoinFriendListWidget::OnJoinFailed(const FString& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("Join Failed: %s"), *Reason);
	ShowLoadingIndicator(false);
	HandleRefreshButtonClicked();
}

void UGS_JoinFriendListWidget::ShowLoadingIndicator(bool bShow)
{
	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}