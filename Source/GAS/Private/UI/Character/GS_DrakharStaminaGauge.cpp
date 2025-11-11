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
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DelayBeforeInterpTimerHandle);
	}

	TargetStaminaPercent = InCurrentGauge/Drakhar->GetMaxStaminaGauge();
	DrakharStaminaGauge->SetPercent(TargetStaminaPercent);

	if (DelayedStaminaPercent < TargetStaminaPercent)
	{
		DelayedStaminaPercent = TargetStaminaPercent;
		if (DrakharStaminaDelayGauge)
		{
			DrakharStaminaDelayGauge->SetPercent(DelayedStaminaPercent);
		}
	}
	else if (DelayedStaminaPercent > TargetStaminaPercent)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(DelayBeforeInterpTimerHandle, this, &UGS_DrakharStaminaGauge::StartDelayBarInterp, 0.1f, false);
		}
	}
}

void UGS_DrakharStaminaGauge::StartDelayBarInterp()
{
	if (!GetWorld()) return;

	if (DelayedStaminaPercent > TargetStaminaPercent)
	{
		const float Duration = 0.03f;
		float Delta = FMath::Abs(DelayedStaminaPercent - TargetStaminaPercent);
		InterpSpeed = FMath::Max(Delta / Duration, 0.5f);
		GetWorld()->GetTimerManager().SetTimer(InterpTimerHandle, this, &UGS_DrakharStaminaGauge::UpdateDelayedStamina, 0.01f, true);
	}
}

void UGS_DrakharStaminaGauge::UpdateDelayedStamina()
{
	if (DelayedStaminaPercent > TargetStaminaPercent
		&& !FMath::IsNearlyEqual(DelayedStaminaPercent, TargetStaminaPercent, 0.001f))
	{
		DelayedStaminaPercent = FMath::FInterpTo(DelayedStaminaPercent, TargetStaminaPercent, 0.01f, InterpSpeed);
		if (DrakharStaminaDelayGauge)
		{
			DrakharStaminaDelayGauge->SetPercent(DelayedStaminaPercent);
		}
	}
	else
	{
		DelayedStaminaPercent = TargetStaminaPercent;
		if (DrakharStaminaDelayGauge)
		{
			DrakharStaminaDelayGauge->SetPercent(DelayedStaminaPercent);
		}
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
		}
	}
}
