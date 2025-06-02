// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/Chan/GS_AN_SetPlayRate.h"
#include "Character/Player/Seeker/GS_Seeker.h"

void UGS_AN_SetPlayRate::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp) return;

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance) return;

	UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
	if (CurrentMontage)
	{
		float NewPlayRate = 1.0f;
		if (AGS_Seeker* Character = Cast<AGS_Seeker>(MeshComp->GetOwner()))
		{
			NewPlayRate = Character->NewPlayRate;
		}
		AnimInstance->Montage_SetPlayRate(CurrentMontage, NewPlayRate);
	}
}
