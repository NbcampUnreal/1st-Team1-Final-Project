// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_CheckAimSkillReady.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"

void UGS_AN_CheckAimSkillReady::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AGS_Chan* Character = Cast<AGS_Chan>(MeshComp->GetOwner()))
	{
		if (Character->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("AimSkillReady %s"), *UEnum::GetValueAsString(Character->GetLocalRole()));
			
			// State
			Character->SetSeekerGait(EGait::Walk);
			
			// Input Control
			Character->SetSkillInputControl(true, false, true);
			Character->Multicast_SetMustTurnInPlace(true);
			Character->SetLookControlValue(true, true);
			Character->SetMoveControlValue(true, true);
		}
	}
}
