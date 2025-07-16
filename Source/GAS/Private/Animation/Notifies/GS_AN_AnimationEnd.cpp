// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_AN_AnimationEnd.h"
#include "Character/Player/Seeker/GS_Seeker.h"

void UGS_AN_AnimationEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AGS_Seeker* SeekerOwner = Cast<AGS_Seeker>(MeshComp->GetOwner());
	
	if (SeekerOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGS_AN_AnimationEnd!!!!!!!!!!!!!!!!!! | %s"), *UEnum::GetValueAsString(SeekerOwner->GetLocalRole()));
		if (SeekerOwner->GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
		{
			SeekerOwner->GetSkillComp()->Server_TrySkillAnimationEnd(SkillType);
		}
	}
}
