// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_UserIDWidget.generated.h"

class UTextBlock;
class UImage;
class AGS_PlayerState;

UCLASS()
class GAS_API UGS_UserIDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TEXT_UserID;
	UPROPERTY(meta = (BindWidget))
	UImage* SteamAvatar;
	
	void SetupWidget(AGS_PlayerState* PlayerState);
private:
	FTimerHandle TimerHandle;

	void DelayedUpdateUserID();
};
