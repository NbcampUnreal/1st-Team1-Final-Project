// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/Chan/GS_AN_CheckStaminaEnd.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"

void UGS_AN_CheckStaminaEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Chan* Chan = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		if (UGS_ChanAimingSkill* Skill = Cast<UGS_ChanAimingSkill>(Chan->GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Aiming)))
		{
			
			if (Skill->GetCurrentStamina() <= 0)
			{
				Chan->ToIdle();
			}
		}
	}
	
}
