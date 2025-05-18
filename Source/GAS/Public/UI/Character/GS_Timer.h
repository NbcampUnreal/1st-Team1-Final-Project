// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_Timer.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_Timer : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TimerText;

private:
	UFUNCTION()
	void HandleTimeUpdated(const FText& NewTime);
};
