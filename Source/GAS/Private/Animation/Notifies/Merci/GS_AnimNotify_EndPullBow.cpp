// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Merci/GS_AnimNotify_EndPullBow.h"
#include "Character/Player/Seeker/GS_Merci.h"

void UGS_AnimNotify_EndPullBow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(Owner))
	{
		MerciCharacter->OnDrawMontageEnded();
	}
}
