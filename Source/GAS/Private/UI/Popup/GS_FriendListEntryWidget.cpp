//#include "UI/Popup/GS_FriendListEntryWidget.h"
//#include "Components/TextBlock.h"
//#include "Components/Button.h"
//#include "Components/Image.h" // OnlineStatusImage 사용 안 할거면 삭제
//#include "OnlineSubsystem.h" // FOnlineFriend
//#include "Interfaces/OnlinePresenceInterface.h"
//#include "Interfaces/OnlineFriendsInterface.h"
//
//void UGS_FriendListEntryWidget::NativeConstruct()
//{
//    Super::NativeConstruct();
//
//    if (InviteButton)
//    {
//        InviteButton->OnClicked.AddDynamic(this, &UGS_FriendListEntryWidget::OnInviteButtonPressed);
//    }
//}
//
//void UGS_FriendListEntryWidget::NativeDestruct()
//{
//    if (InviteButton)
//    {
//        InviteButton->OnClicked.RemoveDynamic(this, &UGS_FriendListEntryWidget::OnInviteButtonPressed);
//    }
//    Super::NativeDestruct();
//}
//
//void UGS_FriendListEntryWidget::SetupFriendEntry(TSharedRef<FOnlineFriend> InFriendData)
//{
//    if (!InFriendData->GetUserId()->IsValid())
//    {
//        UE_LOG(LogTemp, Warning, TEXT("UGS_FriendListEntryWidget::SetupFriendEntry - Invalid Friend User ID."));
//        SetVisibility(ESlateVisibility::Collapsed); // 잘못된 데이터면 숨김
//        return;
//    }
//
//    FriendId = InFriendData->GetUserId();
//
//    if (FriendNameText)
//    {
//        FriendNameText->SetText(FText::FromString(InFriendData->GetDisplayName()));
//    }
//
//    bool bIsOnline = InFriendData->GetPresence().bIsOnline;
//    // GetPresence().Status.State == EOnlinePresenceState::Online; 와 같이 더 자세한 상태 확인 가능
//
//    if (OnlineStatusImage) // 온라인 상태 이미지 처리 (선택 사항)
//    {
//        OnlineStatusImage->SetVisibility(ESlateVisibility::Visible);
//        OnlineStatusImage->SetColorAndOpacity(bIsOnline ? FLinearColor::Green : FLinearColor(0.3f, 0.3f, 0.3f));
//    }
//
//    if (InviteButton)
//    {
//        // 이미 세션에 있는 친구는 초대 버튼 비활성화하는 기능 생각해보기 (세션 멤버 목록과 비교 필요)
//        bool bCanInvite = bIsOnline;
//        InviteButton->SetIsEnabled(bCanInvite);
//        InviteButton->SetVisibility(bCanInvite ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
//    }
//    SetVisibility(ESlateVisibility::Visible);
//}
//
//void UGS_FriendListEntryWidget::OnInviteButtonPressed()
//{
//    if (FriendId.IsValid())
//    {
//        UE_LOG(LogTemp, Log, TEXT("UGS_FriendListEntryWidget - Invite button pressed for friend: %s"), *FriendId->ToString());
//        OnInviteButtonClicked.Broadcast(FriendId);
//    }
//    else
//    {
//        UE_LOG(LogTemp, Warning, TEXT("UGS_FriendListEntryWidget - Invite button pressed but FriendId is invalid."));
//    }
//}