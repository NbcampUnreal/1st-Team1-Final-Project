// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_OptionMenuUI.generated.h"

class UCustomCommonButton;
/**
 * 
 */
UCLASS()
class GAS_API UGS_OptionMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UCustomCommonButton* CloseButton;

	UPROPERTY()
	UUserWidget* OwnerUI;

	// 설정이 적용되었는지 여부 플래그
	UPROPERTY()
	bool bSettingsApplied = false;

	void SetOwnerUI(UUserWidget* InOwnerUI);
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCloseButtonClicked();

	/**
	 * 모든 자식 설정 UI의 CancelSettings()를 호출합니다.
	 */
	void CancelAllSettings();

	/**
	 * 설정이 적용되었음을 표시합니다.
	 * Save 버튼 클릭 시 호출됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void MarkSettingsApplied();
};
