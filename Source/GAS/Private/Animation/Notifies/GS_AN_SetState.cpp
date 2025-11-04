// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_AN_SetState.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/GS_TpsController.h"
#include "Kismet/GameplayStatics.h"

void UGS_AN_SetState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 여기에서는 Notify 에서 받는 값에 따라서 변경할지에 관한 if 문들로 이루어 져야 한다.
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner());

	if (!Seeker)
	{
		return;
	}

	if (SeekerSlot != ESeekerMontageSlot::End)
	{
		Seeker->Multicast_SetMontageSlot(SeekerSlot);
	}

	if (Seeker->CanChangeSeekerGait != bCanChangeSeekerGait)
	{
		Seeker->CanChangeSeekerGait = bCanChangeSeekerGait;
	}

	if (Seeker->CanAcceptComboInput != bCanAcceptComboInput)
	{
		Seeker->CanAcceptComboInput = bCanAcceptComboInput;
	}
	
	AGS_TpsController* TpsController = Cast<AGS_TpsController>(Seeker->GetController());
	if (!TpsController)
	{
		return;
	}
	
	if (bUseControlValue)
	{
		TpsController->SetMoveControlValue(ControlValue.bCanMoveRight, ControlValue.bCanMoveForward);
		TpsController->SetLookControlValue(ControlValue.bCanLookRight, ControlValue.bCanLookUp);
	}

	if (bChangeSeekerGait)
	{
		Seeker->SetSeekerGait(Gait);
	}
}
