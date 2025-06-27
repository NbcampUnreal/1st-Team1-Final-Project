// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Data/GS_TutorialRow.h"
#include "GS_TutorialWidget.generated.h"

class UImage;
class UVerticalBox;
class UDataTable;
class UCommonButtonBase;

/**
 * 
 */
UCLASS()
class GAS_API UGS_TutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGS_TutorialWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void InitTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void GoToPage(uint8 PageIndex);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void GoToPrevPage();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void GoToNextPage();

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	uint8 GetCurrPageIndex() const { return CurrPageIndex; }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	uint8 GetTotalPageCount() const { return TutorialDataArray.Num(); }

	UFUNCTION(BlueprintImplementableEvent)
	void InitButton(UCommonButtonBase* PageButton, const FText& ButtonText);

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeButtonState(UCommonButtonBase* PageButton, bool bIsCurrPage);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* TutorialImage;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PageButtonContainer;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* PrevButton;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* NextButton;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ExitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	UDataTable* TutorialImageTable;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	uint8 CurrPageIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TSubclassOf<UCommonButtonBase> PageButtonClass;

	UPROPERTY()
	TArray<UCommonButtonBase*> PageButtons;

	UPROPERTY()
	TArray<FTutorialImageRow> TutorialDataArray;

	void LoadTutorialData();

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
