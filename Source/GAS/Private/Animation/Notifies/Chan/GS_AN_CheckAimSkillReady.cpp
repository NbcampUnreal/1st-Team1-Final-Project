// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_CheckAimSkillReady.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_AN_CheckAimSkillReady::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("CheckAimSkillReady : %s"), *UEnum::GetValueAsString(Character->GetLocalRole())); // SJE
	}
}
