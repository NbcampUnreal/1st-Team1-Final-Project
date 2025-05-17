// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffAggro.h"
#include "AI/GS_AIController.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/GS_Character.h"
#include "BehaviorTree/BlackboardComponent.h"


void UGS_DebuffAggro::OnApply()
{
	Super::OnApply();

	if (TargetCharacter)
	{
		// 타겟 바꾸기
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (!AIController) return;

		UBlackboardComponent* BB = AIController->GetBlackboardComponent();
		if (!BB) return;

		BB->SetValueAsObject(AGS_AIController::TargetActorKey, OwnerCharacter);

		// 조종 불가
	}
}

void UGS_DebuffAggro::OnExpire()
{
	if (TargetCharacter)
	{
		// 타겟 없애기
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (AIController)
		{
			UBlackboardComponent* BB = AIController->GetBlackboardComponent();
			if (BB)
			{
				// 강제로 설정했던 TargetActor 제거
				BB->ClearValue(AGS_AIController::TargetActorKey);
			}
		}

		// 조종 불가 
	}

	Super::OnExpire();
}
