// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffBase.h"

void UGS_DebuffBase::Initialize(AGS_Character* InTarget, float InDuration, int32 InPriority)
{
	TargetCharacter = InTarget;
	Duration = InDuration;
	Priority = InPriority;
}

void UGS_DebuffBase::OnApply()
{
}

void UGS_DebuffBase::OnExpire()
{
}
