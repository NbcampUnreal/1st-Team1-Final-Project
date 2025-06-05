// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffSlow.h"
#include "Character/GS_Character.h"

void UGS_DebuffSlow::OnApply()
{
	if (TargetCharacter)
	{
		TargetCharacter->Server_SetCharacterSpeed(0.3f);
	}
}

void UGS_DebuffSlow::OnExpire()
{
	if (TargetCharacter)
	{
		TargetCharacter->Server_SetCharacterSpeed(1.0f);
	}
}
