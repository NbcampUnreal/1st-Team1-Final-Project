#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "GS_JoinFriendEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UGS_JoinFriendEntryWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinRequestDelegate, const FString&, FriendId);

UCLASS()
class GAS_API UGS_JoinFriendEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupFriendEntry(const TSharedRef<FOnlineFriend>& InFriendData);

	UPROPERTY(BlueprintAssignable, Category = "Friends|Event")
	FOnJoinRequestDelegate OnJoinRequest;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FriendNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* OnlineStatusImage;

private:
	FString FriendIdString;

	UFUNCTION()
	void OnJoinButtonPressed();
};