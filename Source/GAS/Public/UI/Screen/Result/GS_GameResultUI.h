// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_GameResultBoardUI.h"
#include "Components/Overlay.h"
#include "GS_GameResultUI.generated.h"


class UCommonButtonBase;
class UOverlay;

UCLASS()
class GAS_API UGS_GameResultUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	class UOverlay* ResultBoardOverlay;


	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ContinueButton;


	UFUNCTION()
	void OnContinueButtonClicked();


	void AddResultBoards();
};
