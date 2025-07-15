// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_EndAimSkill.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Skill/GS_SkillComp.h"

void UGS_AN_EndAimSkill::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		if (Character->HasAuthority())
		{
			Character->Multicast_StopSkillMontage(Character->GetCurrentMontage());
			Character->GetSkillComp()->Server_TrySkillAnimationEnd(ESkillSlot::Aiming);
		}
	}
}