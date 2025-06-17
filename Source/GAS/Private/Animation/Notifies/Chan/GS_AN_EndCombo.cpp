// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_EndCombo.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_AN_EndCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{		
		if (Character->bComboEnded) // 이미 끝난 상태
		{
			Character->CanChangeSeekerGait = false;
			return;
		};
		
		if (Character->HasAuthority())
		{
			Character->bComboEnded = true;
			Character->Multicast_ComboEnd();
		}

		Character->CanChangeSeekerGait = true;
	}
}