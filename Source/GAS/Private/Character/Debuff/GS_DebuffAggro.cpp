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
		if (!AIController) 
		{
			return;
		}

		UBlackboardComponent* BB = AIController->GetBlackboardComponent();
		if (!BB) 
		{
			return;
		}

		BB->SetValueAsObject(AGS_AIController::TargetActorKey, OwnerCharacter);

		// Todo: Target이 바뀌지 않도록 Lock 처리할 수 있을까요?

		// 조종 불가
		// Todo: 몬스터를 선택할 수 있습니다. 하지만 원하는 곳으로 이동시키는 등 명령을 내릴 수 없습니다.
	}
}

void UGS_DebuffAggro::OnExpire()
{
	if (TargetCharacter)
	{
		// Todo: Target이 바뀌지 않도록 Lock 처리 해제
		
		// 타겟 없애기 => 안해도 될 것 같음(그냥 그대로 방패병으로 두고 Lock만 해제)
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (AIController)
		{
			UBlackboardComponent* BB = AIController->GetBlackboardComponent();
			if (BB)
			{
				// 강제로 설정했던 TargetActor 제거
				//BB->ClearValue(AGS_AIController::TargetActorKey);
			}
		}

		// 조종 불가
		// Todo: 몬스터를 선택할 수 있으며 이제 원하는 곳으로 이동시키는 등 명령을 내릴 수 있습니다.
	}

	Super::OnExpire();
}
