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

	void SetOwnerUI(UUserWidget* InOwnerUI);
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCloseButtonClicked();
};
