// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_ChanAimingSkillBar.generated.h"

class UProgressBar;
class AGS_Character;

UCLASS()
class GAS_API UGS_ChanAimingSkillBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void SetOwningActor(AGS_Character* InOwningCharacter);

	UFUNCTION(BlueprintCallable, Category="UI")
	void SetAimingProgress(float Progress);

	UFUNCTION(BlueprintCallable)
	void ShowSkillBar(bool bShow);

protected:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* AimingProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharacter;
};
