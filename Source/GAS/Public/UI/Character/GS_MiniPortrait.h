// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_MiniPortrait.generated.h"

struct FDebuffRepInfo;
class AGS_Monster;
class UGS_StatComp;
class UGS_DebuffComp;
/**
 * 
 */
UCLASS()
class GAS_API UGS_MiniPortrait : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AGS_Monster* BoundMonster = nullptr;
	
	UFUNCTION(BlueprintCallable, Category="UI")
	void Init(AGS_Monster* Monster);

protected:
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* PortraitImage;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* HPText;

	UPROPERTY(meta=(BindWidget))
	class UBorder* DebuffBorder;
	
	UFUNCTION()          
	void OnHPChanged(UGS_StatComp* InStatComp);
	
	UFUNCTION()          
	void OnDebuffChanged(const TArray<FDebuffRepInfo>& List);
	
	UFUNCTION(BlueprintCallable)
	void OnPortraitClicked();

private:
	FLinearColor DebuffColor;

	TWeakObjectPtr<UGS_StatComp> BoundStatComp;
	TWeakObjectPtr<UGS_DebuffComp> BoundDebuffComp;
};
