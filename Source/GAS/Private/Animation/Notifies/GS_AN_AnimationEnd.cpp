// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_AN_AnimationEnd.h"
#include "Character/Player/Seeker/GS_Seeker.h"

void UGS_AN_AnimationEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AGS_Seeker* SeekerOwner = Cast<AGS_Seeker>(MeshComp->GetOwner());
	
	if (SeekerOwner)
	{
		if (SeekerOwner->GetLocalRole() == ENetRole::ROLE_Authority)
		{
			SeekerOwner->GetSkillComp()->TrySkillAnimationEnd(SkillType);
		}
	}
}
