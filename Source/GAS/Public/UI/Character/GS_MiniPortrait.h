// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_MiniPortrait.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_MiniPortrait : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="UI")
	void Init(UTexture2D* InPortrait);

protected:
	UPROPERTY(meta=(BindWidget))
	class UImage* PortraitImage;
	
};
