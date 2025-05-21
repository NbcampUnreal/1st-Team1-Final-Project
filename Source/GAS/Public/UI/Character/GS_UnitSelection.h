// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_UnitSelection.generated.h"

class UGS_MiniPortrait;
/**
 * 
 */
UCLASS()
class GAS_API UGS_UnitSelection : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* SelectionSwitcher;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* PortraitImage;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* NameText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* HPText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* DescText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TypeText;
	
	UPROPERTY(meta=(BindWidget))
	class UUniformGridPanel* MultiIconsGrid;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_MiniPortrait> MiniPortraitWidgetClass;

private:
	UFUNCTION()
	void HandleSelectionChanged(const TArray<AGS_Monster*>& NewSelection);
	
	class AGS_RTSController* GetRTSController() const;
	
};
