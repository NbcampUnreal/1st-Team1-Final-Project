// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_ChanReleaseBlockState.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"

void UGS_AN_ChanReleaseBlockState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		if (UGS_SeekerAnimInstance* SeekerAnim = Cast<UGS_SeekerAnimInstance>(MeshComp->GetAnimInstance()))
		{
			//SeekerAnim->ChooserInputObj->IsBlock = false;
		}
		if (Seeker->HasAuthority())
		{
			Seeker->Multicast_SetMustTurnInPlace(false);
		}
	}
}
