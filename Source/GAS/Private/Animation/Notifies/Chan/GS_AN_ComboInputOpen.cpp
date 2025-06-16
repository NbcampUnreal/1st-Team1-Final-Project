// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_ComboInputOpen.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_AN_ComboInputOpen::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		Character->ComboInputOpen();
	}
}
