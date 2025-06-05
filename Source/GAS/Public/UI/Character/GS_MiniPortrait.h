// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_MiniPortrait.generated.h"

class AGS_Monster;
/**
 * 
 */
UCLASS()
class GAS_API UGS_MiniPortrait : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="UI")
	void Init(AGS_Monster* Monster);

protected:
	UPROPERTY(meta=(BindWidget))
	class UImage* PortraitImage;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* HPText;

	UFUNCTION()          
	void OnHPChanged(UGS_StatComp* InStatComp);
};
