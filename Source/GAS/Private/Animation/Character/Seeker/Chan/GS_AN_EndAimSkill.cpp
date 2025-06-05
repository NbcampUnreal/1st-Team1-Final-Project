// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/Chan/GS_AN_EndAimSkill.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"

void UGS_AN_EndAimSkill::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		if (Character->HasAuthority())
		{
			// Slot
			Character->Multicast_SetIsFullBodySlot(false);
			Character->Multicast_SetIsUpperBodySlot(false);
			// Control Input Value
			Character->Multicast_SetMustTurnInPlace(false);
			Character->SetLookControlValue(true, true);
			Character->SetMoveControlValue(true, true);
			Character->SetSkillInputControl(true, true);

			Character->Multicast_StopSkillMontage(Character->GetCurrentMontage());
		}
	}
}
