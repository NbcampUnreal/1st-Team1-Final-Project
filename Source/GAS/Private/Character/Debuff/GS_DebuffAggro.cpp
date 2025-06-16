// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffAggro.h"
#include "AI/GS_AIController.h"
#include "Character/GS_Character.h"
#include "Character/Player/Monster/GS_Monster.h"


void UGS_DebuffAggro::OnApply()
{
	Super::OnApply();
	
	if (TargetCharacter)
	{
		// 락온 
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (AIController)
		{
			AGS_Character* Cuaser = Cast<AGS_Character>(CauserActor);
			AIController->LockTarget(Cuaser);
		}

		// rts 조종 불가
		if (AGS_Monster* Monster = Cast<AGS_Monster>(TargetCharacter))
		{
			Monster->bCommandLocked = true;
		}
	}
}

void UGS_DebuffAggro::OnExpire()
{
	if (TargetCharacter)
	{
		// 락온 처리 해제 
		AGS_AIController* AIController = Cast<AGS_AIController>(TargetCharacter->GetController());
		if (AIController)
		{
			AIController->UnlockTarget();
		}

		// rts 조종 불가 해제 
		if (AGS_Monster* Monster = Cast<AGS_Monster>(TargetCharacter))
		{
			Monster->bCommandLocked = false;
		}
	}

	Super::OnExpire();
}
