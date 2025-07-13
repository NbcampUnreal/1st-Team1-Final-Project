#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "GS_FriendListEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UGS_FriendListEntryWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteButtonClickedDelegate, UGS_FriendListEntryWidget*, ClickedEntry);

UCLASS()
class GAS_API UGS_FriendListEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetupFriendEntry(const TSharedRef<FOnlineFriend>& InFriendData);

    TSharedPtr<const FUniqueNetId> GetFriendId() const { return FriendId; }

    UPROPERTY(BlueprintAssignable, Category = "Friends|Event")
    FOnInviteButtonClickedDelegate OnInviteButtonClicked;

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* FriendNameText;

    UPROPERTY(meta = (BindWidget))
    UButton* InviteButton;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* OnlineStatusImage;

private:
    TSharedPtr<const FUniqueNetId> FriendId;

    UFUNCTION()
    void OnInviteButtonPressed();
};
