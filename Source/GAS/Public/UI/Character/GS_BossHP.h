// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_BossHP.generated.h"

class UVerticalBox;
class UGS_HPWidget;
class AGS_Guardian;
class AGS_Character;

UCLASS()
class GAS_API UGS_BossHP : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void InitBossHPWidget();
	
	void SetOwningActor(AGS_Character* InOwningCharacter) { WidgetOwner = InOwningCharacter; }

	bool FindBoss();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> HPWidgetList;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_HPWidget> HPWidgetClass;

	UPROPERTY()
	TObjectPtr<UGS_HPWidget> HPWidgetInstance;
	
	UPROPERTY()
	TObjectPtr<AGS_Guardian> Guardian;
	
	UPROPERTY()
	TObjectPtr<AGS_Character> WidgetOwner;
};
