// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_StateReset.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"

void UGS_AN_StateReset::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Seeker* Character = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		if (Character->HasAuthority())
		{
			if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(Character->GetMesh()->GetAnimInstance()))
			{
				Character->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
			}

			Character->CanChangeSeekerGait = true;
			Character->SetMoveControlValue(true, true);
			Character->SetLookControlValue(true, true);

			Character->GetSkillComp()->ResetAllowedSkillsMask();
			
			Character->SetAimState(false);
			Character->SetDrawState(false);
		}
	}
}
