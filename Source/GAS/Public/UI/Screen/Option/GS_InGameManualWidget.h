// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_InGameManualWidget.generated.h"

class UCommonButtonBase;

/**
 * 
 */
UCLASS()
class GAS_API UGS_InGameManualWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* CloseButton;

	UFUNCTION()
	void OnCloseButtonClicked();
};
