// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_AN_PlayOtherMontage.h"
#include "Character/Player/GS_Player.h"
#include "Animation/Character/GS_CharacterAnimInstance.h"

void UGS_AN_PlayOtherMontage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AGS_Player* Player = Cast<AGS_Player>(MeshComp->GetOwner());

	UGS_CharacterAnimInstance* AnimInstance = Cast<UGS_CharacterAnimInstance>(MeshComp->GetAnimInstance());
	
	if (NextMontage != nullptr && AnimInstance != nullptr)
	{
		if (Player->HasAuthority())
		{
			Player->Multicast_PlaySkillMontage(NextMontage);
		}
	}
}
