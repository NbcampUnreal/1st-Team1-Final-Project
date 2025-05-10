// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffStun.h"

void UGS_DebuffStun::OnApply()
{
	if (TargetCharacter)
	{
		// 일반 공격만
		// 움직임도 멈춤
		// 스킬 못쓰고
		// 스킬을 끊지는 않음
	}
}

void UGS_DebuffStun::OnExpire()
{
	if (TargetCharacter)
	{

	}
}
