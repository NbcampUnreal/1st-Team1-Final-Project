// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GS_DrakharStaminaGauge.generated.h"

class AGS_Drakhar;
class UProgressBar;

UCLASS()
class GAS_API UGS_DrakharStaminaGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	UGS_DrakharStaminaGauge(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	void InitializeGauge(float InCurrentGauge);
	
	AGS_Drakhar* GetOwningActor()const { return Drakhar; }

	void SetOwningActor(AGS_Drakhar* InOwningCharacter) { Drakhar = InOwningCharacter; }

	UFUNCTION()
	void OnCurrentStaminaGaugeChanged(float InCurrentGauge);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr <UProgressBar> DrakharStaminaGauge;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Drakhar> Drakhar;
};
