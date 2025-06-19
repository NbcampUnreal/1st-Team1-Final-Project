// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DrakharFeverGauge.generated.h"

class AGS_Drakhar;
class UGS_StatComp;
class AGS_Character;
class UProgressBar;

UCLASS()
class GAS_API UGS_DrakharFeverGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	UGS_DrakharFeverGauge(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	void InitializeGauge(float InCurrentGauge);
	
	AGS_Drakhar* GetOwningActor()const { return Drakhar; }

	void SetOwningActor(AGS_Drakhar* InOwningCharacter) { Drakhar = InOwningCharacter; }

	UFUNCTION()
	void OnCurrentFeverGaugeChanged(float InCurrentGauge);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr <UProgressBar> DrakharFeverGauge;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Drakhar> Drakhar;
};
