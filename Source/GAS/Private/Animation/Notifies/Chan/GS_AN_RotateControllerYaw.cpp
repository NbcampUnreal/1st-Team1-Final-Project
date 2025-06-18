// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_RotateControllerYaw.h"

#include "Character/GS_TpsController.h"
#include "Character/Player/Seeker/GS_Seeker.h"

void UGS_AN_RotateControllerYaw::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		if (AGS_TpsController* TpsController = Cast<AGS_TpsController>(Seeker->GetController()))
		{
			FRotator YawOnlyRotation(0.f, TpsController->GetControlRotation().Yaw, 0.f);
			Seeker->SetActorRotation(YawOnlyRotation);
		}
	}
}
