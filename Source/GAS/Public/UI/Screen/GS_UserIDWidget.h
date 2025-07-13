// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Character/GS_UserInfo.h"
#include "GS_UserIDWidget.generated.h"

UCLASS()
class GAS_API UGS_UserIDWidget : public UGS_UserInfo
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

private:
	FTimerHandle TimerHandle;

	void DelayedUpdateUserID();
};
