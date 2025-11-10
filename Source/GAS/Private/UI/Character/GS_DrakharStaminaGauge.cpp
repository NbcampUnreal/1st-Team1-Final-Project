// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_DrakharStaminaGauge.h"

#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Components/ProgressBar.h"

UGS_DrakharStaminaGauge::UGS_DrakharStaminaGauge(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGS_DrakharStaminaGauge::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsValid(Drakhar))
	{
		Drakhar = Cast<AGS_Drakhar>(GetOwningPlayer()->GetPawn());
	}
	
	if (IsValid(Drakhar))
	{
		Drakhar->SetStaminaGaugeWidget(this);
	}
}

void UGS_DrakharStaminaGauge::InitializeGauge(float InCurrentGauge)
{
	OnCurrentStaminaGaugeChanged(InCurrentGauge);
}

void UGS_DrakharStaminaGauge::OnCurrentStaminaGaugeChanged(float InCurrentGauge)
{
	DrakharStaminaGauge->SetPercent(InCurrentGauge/Drakhar->GetMaxStaminaGauge());
}
