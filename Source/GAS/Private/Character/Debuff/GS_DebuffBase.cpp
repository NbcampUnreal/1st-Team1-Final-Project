// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffBase.h"

void UGS_DebuffBase::Initialize(AGS_Character* InTarget, float InDuration, int32 InPriority, EDebuffType InDebuffType)
{
	TargetCharacter = InTarget;
	Duration = InDuration;
	Priority = InPriority;
	DebuffType = InDebuffType;
}

void UGS_DebuffBase::OnApply()
{
}

void UGS_DebuffBase::OnExpire()
{
}

float UGS_DebuffBase::GetRemainingTime(float CurrentTime) const
{
	UE_LOG(LogTemp, Warning, TEXT("RemainingTime : %f"), Duration - (CurrentTime - StartTime));
	return FMath::Max(0.0f, Duration - (CurrentTime - StartTime));
}
