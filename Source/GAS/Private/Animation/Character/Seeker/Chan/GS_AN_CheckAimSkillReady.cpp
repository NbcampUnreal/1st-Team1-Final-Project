// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/Chan/GS_AN_CheckAimSkillReady.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"

void UGS_AN_CheckAimSkillReady::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		Character->OnReadyAimSkill();
		Character->Multicast_SetMustTurnInPlace(true);
		Character->Multicast_SetLookControlValue(true, true);
	}
}
