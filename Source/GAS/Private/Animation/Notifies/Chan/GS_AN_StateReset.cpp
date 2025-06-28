// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_StateReset.h"
#include "Character/Player/Seeker/GS_Seeker.h"

void UGS_AN_StateReset::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Seeker* Character = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		if (Character->HasAuthority())
		{
			Character->Multicast_SetIsFullBodySlot(false);
			Character->Multicast_SetIsUpperBodySlot(false);
			Character->Multicast_SetMustTurnInPlace(false);
			Character->Multicast_SetCanHitReact(true);
			Character->CanChangeSeekerGait = true;
			Character->SetSkillInputControl(true, true, true);
			Character->SetMoveControlValue(true, true);
			Character->SetLookControlValue(true, true);

			UE_LOG(LogTemp, Warning, TEXT("Aiming Skill Active? %d"), Character->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming));
		}
	}
}
