// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffConfuse.h"
#include "AI/GS_AIController.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/GS_Character.h"
#include "BehaviorTree/BlackboardComponent.h"

void UGS_DebuffConfuse::OnApply() 
{
	Super::OnApply();

	if (TargetCharacter)
	{
		// 타겟 잃어버리기
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (AIController)
		{
			AIController->EnterConfuseState();
		}

		// rts 몬스터 선택 불가
		if (AGS_Monster* Monster = Cast<AGS_Monster>(TargetCharacter))
		{
			Monster->bSelectionLocked = true;
		}
	}
}

void UGS_DebuffConfuse::OnExpire()
{
	if (TargetCharacter)
	{
		// 타겟을 잃어버리기 전 타겟 설정 
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (AIController)
		{
			AIController->ExitConfuseState();
		}

		// rts 몬스터 선택 불가 해제 
		if (AGS_Monster* Monster = Cast<AGS_Monster>(TargetCharacter))
		{
			Monster->bSelectionLocked = false;
		}
	}
}
