// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_MainMenuUI.generated.h"

class UUserWidget;
class UButton;
class UTextBlock;
class UCanvasPanel;

UCLASS()
class GAS_API UGS_MainMenuUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UButton* PlayButton;
	UPROPERTY(meta = (BindWidget))
	UButton* CreditButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UPROPERTY(meta = (BindWidget))
	UButton* SettingButton;
	UPROPERTY(meta = (BindWidget))
	UButton* SeekerButton;
	UPROPERTY(meta = (BindWidget))
	UButton* GuardianButton;
	UPROPERTY(meta = (BindWidget))
	UButton* CustomGameButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayText;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* ExitPopupCanvas;

	bool bIsPlayButtonClicked = false;

	UFUNCTION()
	void OnPlayButtonClicked();
	UFUNCTION()
	void OnCustomGameButtonClicked();
	UFUNCTION()
	void OnExitButtonClicked();

};
