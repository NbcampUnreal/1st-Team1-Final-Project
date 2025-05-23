//#pragma once
//
//#include "CoreMinimal.h"
//#include "Blueprint/UserWidget.h"
//#include "Interfaces/OnlineFriendsInterface.h"
//#include "GS_FriendListWidget.generated.h"
//
//class UScrollBox;
//class UGS_FriendListEntryWidget;
//class UButton;
//class UCircularThrobber; // 로딩 표시용
//
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendInviteRequestedDelegate, FUniqueNetIdPtr, FriendIdToInvite);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRefreshFriendsClickedDelegate);
//
//UCLASS()
//class GAS_API UGS_FriendListWidget : public UUserWidget
//{
//	GENERATED_BODY()
//	
//public:
//	UPROPERTY(meta = (BindWidget))
//	UScrollBox* FriendsScrollBox;
//	UPROPERTY(meta = (BindWidget))
//	UButton* RefreshButton;
//	UPROPERTY(meta = (BindWidget))
//	UButton* CloseButton;
//	UPROPERTY(meta = (BindWidget))
//	UCircularThrobber* LoadingIndicator;
//
//	void FillFriendList(const TArray<TSharedRef<FOnlineFriend>>& Friends);
//	void ClearFriendList();
//
//	UPROPERTY(BlueprintAssignable, Category = "Friends|Event")
//	FOnFriendInviteRequestedDelegate OnFriendInviteRequested;
//	UPROPERTY(BlueprintAssignable, Category = "Friends|Event")
//	FOnRefreshFriendsCickedDelegate OnRefreshFriendsClicked;
//	
//protected:
//	virtual void NativeConstruct() override;
//	virtual void NativeDestruct() override;
//
//	TSubclassOf<UGS_FriendListEntryWidget> FriendListEntryWidgetClass;
//
//private:
//	UFUNCTION()
//	void HandleFriendInviteButtonClicked(FUniqueNetIdPtr FriendId);
//	UFUNCTION()
//	void HandleRefreshButtonClicked();
//	UFUNCTION()
//	void HandleCloseButtonClicked();
//};
