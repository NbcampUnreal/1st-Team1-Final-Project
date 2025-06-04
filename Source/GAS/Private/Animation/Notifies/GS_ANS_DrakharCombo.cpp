// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_ANS_DrakharCombo.h"

#include "Character/Player/Guardian/GS_Drakhar.h"

void UGS_ANS_DrakharCombo::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(MeshComp->GetOwner());
	if (IsValid(Drakhar))
	{
		Drakhar->SetNextComboAttackSection(SectionName);
	}
}

void UGS_ANS_DrakharCombo::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(MeshComp->GetOwner());
	if (IsValid(Drakhar))
	{
		Drakhar->ResetComboAttackSection();
	}
}
