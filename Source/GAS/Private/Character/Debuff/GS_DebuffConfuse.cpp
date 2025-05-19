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
		if (!AIController) return;

		UBlackboardComponent* BB = AIController->GetBlackboardComponent();
		if (!BB) return;

		BB->ClearValue(AGS_AIController::TargetActorKey);

		// Todo: Target이 비어 있도록 Lock

		// 선택 불가
		// Todo: 몬스터를 선택할 수 없습니다.
	}
}

void UGS_DebuffConfuse::OnExpire()
{
	if (TargetCharacter)
	{
		// Todo: Lock 해제
		
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (!AIController)
		{
			return;
		}

		UBlackboardComponent* BB = AIController->GetBlackboardComponent();
		if (!BB)
		{
			return;
		}
		// Todo: 타겟을 잃어버리기 전 타겟을 다시 가진다.

		// Todo: 몬스터를 선택할 수 있습니다.
	}
}
