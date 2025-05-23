//#include "UI/Popup/GS_FriendListWidget.h"
//#include "UI/Popup/GS_FriendListEntryWidget.h"
//#include "Components/ScrollBox.h"
//#include "Components/Button.h"
//#include "Components/CircularThrobber.h"
//#include "OnlineSubsystem.h"
//#include "Kismet/GameplayStatics.h"
//#include "Interfaces/OnlinePresenceInterface.h"
//#include "Interfaces/OnlineFriendsInterface.h"
//
//void UGS_FriendListWidget::NativeConstruct()
//{
//    Super::NativeConstruct();
//
//    if (RefreshButton)
//    {
//        RefreshButton->OnClicked.AddDynamic(this, &UGS_FriendsListWidget::OnRefreshButtonClicked_Internal);
//    }
//    if (CloseButton)
//    {
//        CloseButton->OnClicked.AddDynamic(this, &UGS_FriendsListWidget::OnCloseButtonClicked_Internal);
//    }
//
//    if (LoadingIndicator)
//    {
//        LoadingIndicator->SetVisibility(ESlateVisibility::Collapsed);
//    }
//    ClearFriendList();
//}
//
//void UGS_FriendListWidget::NativeDestruct()
//{
//    if (RefreshButton)
//    {
//        RefreshButton->OnClicked.RemoveDynamic(this, &UGS_FriendsListWidget::OnRefreshButtonClicked_Internal);
//    }
//    if (CloseButton)
//    {
//        CloseButton->OnClicked.RemoveDynamic(this, &UGS_FriendsListWidget::OnCloseButtonClicked_Internal);
//    }
//    ClearFriendList();
//    Super::NativeDestruct();
//}
//
//void UGS_FriendListWidget::FillFriendList(const TArray<TSharedRef<FOnlineFriend>>& Friends)
//{
//    ClearFriendList();
//
//    if (!FriendListEntryWidgetClass)
//    {
//        UE_LOG(LogTemp, Error, TEXT("UGS_FriendsListWidget - Nooo FriendEntryWidgetClass"));
//        return;
//    }
//
//    if (!FriendsScrollBox)
//    {
//        UE_LOG(LogTemp, Error, TEXT("UGS_FriendsListWidget - Nooo FriendsScrollBox"));
//        return;
//    }
//
//    UE_LOG(LogTemp, Log, TEXT("UGS_FriendsListWidget - Populating friends list with %d friends."), Friends.Num());
//
//    // 온라인 상태인 친구를 먼저 표시하고, 그 다음 오프라인 친구 표시
//    TArray<TSharedRef<FOnlineFriend>> SortedFriends = Friends;
//    SortedFriends.Sort([](const TSharedRef<FOnlineFriend>& A, const TSharedRef<FOnlineFriend>& B) {
//        bool bAIsOnline = A->GetPresence().bIsOnline;
//        bool bBIsOnline = B->GetPresence().bIsOnline;
//        if (bAIsOnline != bBIsOnline)
//        {
//            return bAIsOnline > bBIsOnline; // 온라인인 친구가 위로
//        }
//        return A->GetDisplayName() < B->GetDisplayName(); // 이름순 정렬
//    });
//
//
//    for (const auto& FriendData : SortedFriends)
//    {
//        UGS_FriendListEntryWidget* EntryWidget = CreateWidget<UGS_FriendListEntryWidget>(this, FriendListEntryWidgetClass);
//        if (EntryWidget)
//        {
//            EntryWidget->SetupFriendEntry(FriendData);
//            // 각 친구 항목의 초대 버튼 클릭 이벤트를 이 위젯의 핸들러에 바인딩
//            EntryWidget->OnInviteButtonClicked.AddDynamic(this, &UGS_FriendsListWidget::HandleFriendEntryInviteClicked);
//            FriendsScrollBox->AddChild(EntryWidget);
//        }
//    }
//
//    if (LoadingIndicator)
//    {
//        LoadingIndicator->SetVisibility(ESlateVisibility::Collapsed); // 목록 채우고 로딩 표시 제거
//    }
//}
//
//void UGS_FriendListWidget::ClearFriendList()
//{
//    if (FriendsScrollBox)
//    {
//        for (UWidget* ChildWidget : FriendsScrollBox->GetAllChildren())
//        {
//            if (UGS_FriendListEntryWidget* EntryWidget = Cast<UGS_FriendListEntryWidget>(ChildWidget))
//            {
//                EntryWidget->OnInviteButtonClicked.RemoveDynamic(this, &UGS_FriendsListWidget::HandleFriendEntryInviteClicked);
//            }
//        }
//        FriendsScrollBox->ClearChildren();
//    }
//    if (LoadingIndicator)
//    {
//        LoadingIndicator->SetVisibility(ESlateVisibility::Visible); // 목록을 비우고 로딩 표시
//    }
//}
//
//
//void UGS_FriendListWidget::HandleFriendInviteButtonClicked(FUniqueNetIdPtr FriendId)
//{
//    if (FriendId.IsValid())
//    {
//        UE_LOG(LogTemp, Log, TEXT("UGS_FriendsListWidget - Invite requested for friend: %s. Broadcasting to PC."), *FriendId->ToString());
//        OnFriendInviteRequested.Broadcast(FriendId); // 이 이벤트를 PlayerController에서 받아서 처리
//    }
//}
//
//void UGS_FriendListWidget::HandleRefreshButtonClicked()
//{
//    UE_LOG(LogTemp, Log, TEXT("UGS_FriendsListWidget - Refresh button clicked."));
//    ClearFriendList();
//    OnRefreshFriendsClicked.Broadcast();
//}
//
//void UGS_FriendListWidget::HandleCloseButtonClicked()
//{
//    UE_LOG(LogTemp, Log, TEXT("UGS_FriendsListWidget - Close button clicked."));
//    // UMG 위젯을 닫는 로직 (예: PlayerController에게 알려서 RemoveFromParent() 호출)
//    // 여기서 바로 숨기거나 제거할 수 있지만, PC에서 관리하는 것이 더 좋음
//    // 예시: GetOwningPlayer()->... 
//    RemoveFromParent();
//}