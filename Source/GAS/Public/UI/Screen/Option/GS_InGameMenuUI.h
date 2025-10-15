// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_InGameMenuUI.generated.h"

class UGS_OptionMenuUI;
class UGS_InGameManualWidget;

/**
 * 
 */
UCLASS()
class GAS_API UGS_InGameMenuUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UUserWidget* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UUserWidget* ManualButton;

	UPROPERTY(meta = (BindWidget))
	UUserWidget* SettingButton;

	UPROPERTY(meta = (BindWidget))
	UUserWidget* ExitButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_OptionMenuUI> OptionMenuUIClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_InGameManualWidget> ManualUIClass;

	UFUNCTION()
	void OnResumeButtonClicked();

	UFUNCTION()
	void OnManualButtonClicked();

	UFUNCTION()
	void OnSettingButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();
};
