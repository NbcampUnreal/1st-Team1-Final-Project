// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_EndCombo.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_AN_EndCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (AGS_Seeker* Character = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		if (Character->HasAuthority())
		{
			Character->Multicast_SetIsFullBodySlot(false);
			Character->Multicast_SetIsUpperBodySlot(false);
			Character->SetMoveControlValue(true, true);
			Character->SetSkillInputControl(true, true, true);
			Character->CanChangeSeekerGait = true;
			Character->CurrentComboIndex = 0;
			Character->ComboInputOpen();
		}
	}
}