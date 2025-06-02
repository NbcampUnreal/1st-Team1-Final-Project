// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/Chan/GS_AN_EndAimAttack.h"

void UGS_AN_EndAimAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
}
