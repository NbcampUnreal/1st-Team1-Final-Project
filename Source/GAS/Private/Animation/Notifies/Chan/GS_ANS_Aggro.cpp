// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_ANS_Aggro.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"

void UGS_ANS_Aggro::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		if (Character->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("Chan Roar in server")); // SJE
		}
	}
}

void UGS_ANS_Aggro::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		
		Character->GetSkillComp()->TryDeactiveSkill(ESkillSlot::Moving);
	}
}
