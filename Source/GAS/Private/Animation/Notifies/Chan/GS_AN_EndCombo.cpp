// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_EndCombo.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_AN_EndCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	UE_LOG(LogTemp, Warning, TEXT("AN_EndCombo %s"), *UEnum::GetValueAsString(MeshComp->GetOwner()->GetLocalRole())); // SJE
	
	if (AGS_Seeker* Character = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{		
		if (Character->bComboEnded) // 이미 끝난 상태
		{
			Character->CanChangeSeekerGait = false;
			return;
		};
		if (Character->HasAuthority())
		{
			Character->bComboEnded = true;
			UGS_SeekerAnimInstance* SeekerAnimInstance = Cast<UGS_SeekerAnimInstance>(Character->GetMesh()->GetAnimInstance());
			if (SeekerAnimInstance)
			{
				SeekerAnimInstance->IsPlayingFullBodyMontage = false;
				SeekerAnimInstance->IsPlayingUpperBodyMontage = false;
			}
			Character->Multicast_ComboEnd();
			UE_LOG(LogTemp, Warning, TEXT("AN_End Combo HasAuthority"));
			Character->SetMoveControlValue(true, true);
		}
		Character->CanChangeSeekerGait = true;
	}
}