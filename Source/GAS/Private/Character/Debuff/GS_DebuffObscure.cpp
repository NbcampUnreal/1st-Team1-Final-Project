// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffObscure.h"
#include "Character/Player/GS_Player.h"

void UGS_DebuffObscure::OnApply()
{
	if(AGS_Player* TargetPlayer = Cast<AGS_Player>(TargetCharacter))
	{
		TargetPlayer->Client_StartVisionObscured();
	}
}

void UGS_DebuffObscure::OnExpire()
{
	if (AGS_Player* TargetPlayer = Cast<AGS_Player>(TargetCharacter))
	{
		TargetPlayer->Client_StopVisionObscured();
	}
}
