#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSessionSettings.h" 
#include "GS_JoinFriendListWidget.generated.h"

class UScrollBox;
class UGS_JoinFriendEntryWidget;
class UButton;
class UCircularThrobber;
class UGS_OnlineGameSubsystem;

UCLASS()
class GAS_API UGS_JoinFriendListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowLoadingIndicator(bool bShow);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* FriendsScrollBox;

	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(meta = (BindWidget))
	UCircularThrobber* LoadingIndicator;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_JoinFriendEntryWidget> FriendListEntryWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UGS_OnlineGameSubsystem> OnlineSubsystem;

	// --- 버튼 핸들러 ---
	UFUNCTION()
	void HandleRefreshButtonClicked();

	UFUNCTION()
	void HandleCloseButtonClicked();

	// --- 델리게이트 핸들러 ---
	void OnFriendsListRead(const TArray<TSharedRef<FOnlineFriend>>& FriendsList);
	UFUNCTION()
	void HandleJoinRequest(const FString& FriendId);
	void OnJoinSucceeded();
	void OnJoinFailed(const FString& Reason);
};