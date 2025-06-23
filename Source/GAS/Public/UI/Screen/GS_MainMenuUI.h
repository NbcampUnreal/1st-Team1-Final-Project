// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_MainMenuUI.generated.h"

class UGS_OptionMenuUI;
class UGS_CommonTwoBtnPopup;
class UScaleBox;
class UUserWidget;
class UButton;
class UTextBlock;
class UCanvasPanel;
class UOverlay;
class UGS_JoinFriendListWidget;

UCLASS()
class GAS_API UGS_MainMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UUserWidget* PlayButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* CreditButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* ExitButton;
	UPROPERTY(meta = (BindWidget))
	UScaleBox* PlayBtnPopUp;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SeekerButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* GuardianButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* CustomGameButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* FriendListButton;
	UPROPERTY(meta = (BindWidget))
	UOverlay* FriendListOverlay;
	UPROPERTY(meta = (BindWidget))
	UGS_CommonTwoBtnPopup* ExitPopUp;

	UPROPERTY(EditDefaultsOnly, Category = "Friend")
	TSubclassOf<UGS_JoinFriendListWidget> FriendListWidgetClass;
	UPROPERTY()
	TObjectPtr<UGS_JoinFriendListWidget> FriendListWidgetInstance;

	bool bIsPlayButtonClicked = false;

	UFUNCTION()
	void OnPlayButtonClicked();
	UFUNCTION()
	void OnCustomGameButtonClicked();
	UFUNCTION()
	void OnExitButtonClicked();
	
	void OnExitPopupYesButtonClicked();
	void OnExitPopupNoButtonClicked();
	UFUNCTION()
	void OnFriendListButtonClicked();

protected:
	virtual void NativeConstruct() override;
};
