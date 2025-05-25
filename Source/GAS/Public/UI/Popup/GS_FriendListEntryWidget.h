//#pragma once
//
//#include "CoreMinimal.h"
//#include "Blueprint/UserWidget.h"
//#include "Interfaces/OnlineFriendsInterface.h"
//#include "GS_FriendListEntryWidget.generated.h"
//
//class UTextBlock;
//class UButton;
//class UImage;
//
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteButtonClickedDelegate, FUniqueNetIdPtr, FriendId);
//
//UCLASS()
//class GAS_API UGS_FriendListEntryWidget : public UUserWidget
//{
//	GENERATED_BODY()
//	
//public:
//    UPROPERTY(meta = (BindWidget))
//    UTextBlock* FriendNameText;
//
//    UPROPERTY(meta = (BindWidget))
//    UButton* InviteButton;
//
//	UPROPERTY(meta = (BindWidgetOptional)) // 온라인 유무를 이미지로 표시. 이 방법 말고 opacity를 조정하는 방법도 있음
//    UImage* OnlineStatusImage;
//
//    void SetupFriendEntry(TSharedRef<FOnlineFriend> InFriendData);
//
//    UPROPERTY(BlueprintAssignable, Category = "Friends|Event")
//    FOnInviteButtonClickedDelegate OnInviteButtonClicked;
//
//protected:
//    virtual void NativeConstruct() override;
//	virtual void NativeDestruct() override;
//
//private:
//    TSharedPtr<const FUniqueNetId> FriendId;
//
//    UFUNCTION()
//    void OnInviteButtonPressed();
//};
