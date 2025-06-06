// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_GameResultBoardUI.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UCommonButtonBase;
class AGS_PlayerState;
class UGS_PawnMappingDataAsset;

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
	UTextBlock* Text_Level; //?

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

	//생사 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Result UI")
	UTexture2D* DeadTexture;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Result UI")
	UTexture2D* AliveTexture;

	UFUNCTION()
	void OnArcaneBoardButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Game Result UI")
	void SetupResultBoard(AGS_PlayerState* TargetPlayerState, UGS_PawnMappingDataAsset* PawnMappingDataAsset);
};
