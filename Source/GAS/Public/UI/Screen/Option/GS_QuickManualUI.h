// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "System/GS_PlayerRole.h"
#include "GS_QuickManualUI.generated.h"

class UImage;
class UCommonButtonBase;

/**
 * 
 */
UCLASS()
class GAS_API UGS_QuickManualUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Manual")
	void InitImage();
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* KeyManualImage;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ExitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	UDataTable* ManualImageTable;

	UFUNCTION()
	void OnExitButtonClicked();
};
