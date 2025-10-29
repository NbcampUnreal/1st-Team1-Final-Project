// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Data/GS_ManualRow.h"
#include "GS_InGameManualWidget.generated.h"

class UImage;
class UVerticalBox;
class UDataTable;
class UCommonButtonBase;

/**
 * 
 */
UCLASS()
class GAS_API UGS_InGameManualWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGS_InGameManualWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Manual")
	void InitManual();

	UFUNCTION(BlueprintCallable, Category = "Manual")
	void GoToPage(uint8 PageIndex);

	UFUNCTION(BlueprintCallable, Category = "Manual")
	void GoToPrevPage();

	UFUNCTION(BlueprintCallable, Category = "Manual")
	void GoToNextPage();

	UFUNCTION(BlueprintPure, Category = "Manual")
	uint8 GetCurrPageIndex() const { return CurrPageIndex; }

	UFUNCTION(BlueprintPure, Category = "Manual")
	uint8 GetTotalPageCount() const { return ManualDataArray.Num(); }

	UFUNCTION(BlueprintImplementableEvent)
	void InitButton(UCommonButtonBase* PageButton, const FText& ButtonText);

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeButtonState(UCommonButtonBase* PageButton, bool bIsCurrPage);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* ManualImage;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PageButtonContainer;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* PrevButton;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* NextButton;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ExitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	UDataTable* ManualImageTable;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	uint8 CurrPageIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TSubclassOf<UCommonButtonBase> PageButtonClass;

	UPROPERTY()
	TArray<UCommonButtonBase*> PageButtons;

	UPROPERTY()
	TArray<FManualImageRow> ManualDataArray;

	void LoadManualData();

	void CreatePageButtons();

	void UpdateCurrPage();

	void UpdatePageButtonStates();

	void OnPageButtonClicked(uint8 PageIndex);

	UFUNCTION()
	void OnPrevButtonClicked();

	UFUNCTION()
	void OnNextButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

private:
	void BindPageButtonEvents();
};
