// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_GameResultUI.generated.h"

class UTextBlock;
class UCommonButtonBase;
class UHorizontalBox;
class UGS_GameResultBoardUI;
class UGS_PawnMappingDataAsset;

UCLASS()
class GAS_API UGS_GameResultUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(EditAnywhere, Category = "Game Result UI | Setup")
	TSubclassOf<UGS_GameResultBoardUI> GameResultBoardUIClass;

	UPROPERTY(EditAnywhere, Category = "Game Result UI | Setup")
	UGS_PawnMappingDataAsset* PawnMappingDataAsset;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* PlayerResultHorizontalBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_MyRole;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Victory;

	UPROPERTY(meta = (BindWidget))
	UCommonButtonBase* ContinueButton;


	UFUNCTION()
	void OnContinueButtonClicked();


	void AddResultBoards();
};
