#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystemTypes.h" // FUniqueNetIdRepl
#include "Interfaces/OnlineFriendsInterface.h"
#include "GS_FriendListWidget.generated.h"

class UScrollBox;
class UGS_FriendListEntryWidget;
class UButton;
class UCircularThrobber;

UCLASS()
class GAS_API UGS_FriendListWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void FillFriendList(const TArray<TSharedRef<FOnlineFriend>>& Friends);
    void ClearFriendList();
    void ShowLoadingIndicator();


protected:
    virtual void NativeConstruct() override;

    // 위젯 바인딩
    UPROPERTY(meta = (BindWidget))
    UScrollBox* FriendsScrollBox;
    UPROPERTY(meta = (BindWidget))
    UButton* RefreshButton;
    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;
    UPROPERTY(meta = (BindWidget))
    UCircularThrobber* LoadingIndicator;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UGS_FriendListEntryWidget> FriendListEntryWidgetClass;

private:
    UFUNCTION()
    void HandleFriendInviteButtonClicked(UGS_FriendListEntryWidget* ClickedEntry);

    UFUNCTION()
    void HandleRefreshButtonClicked();

    UFUNCTION()
    void HandleCloseButtonClicked();

    void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
};