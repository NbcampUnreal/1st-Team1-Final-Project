// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_GameResultBoardUI.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UCommonButtonBase;

UCLASS()
class GAS_API UGS_GameResultBoardUI : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	virtual void NativeConstruct() override;

public:
	// 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_PlayerID;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Level;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Victory;

	// 이미지
	UPROPERTY(meta = (BindWidget))
	UImage* Image_PlayerStatus;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_PlayerPortrait;

	// 버튼
	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ArcaneBoardButton;



	UFUNCTION()
	void OnArcaneBoardButtonClicked();

};
