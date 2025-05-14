// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HPText.generated.h"

class UGS_StatComp;
class UTextBlock;

UCLASS()
class GAS_API UGS_HPText : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGS_HPText(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	void InitializeHPTextWidget(UGS_StatComp* InStatComp);

	AActor* GetOwningActor()const { return OwningActor; }

	void SetOwningActor(AActor* InOwningActor) { OwningActor = InOwningActor; }

	UFUNCTION()
	void OnCurrentHPChanged(float InCurrentHP);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentHPText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor>OwningActor;
};
