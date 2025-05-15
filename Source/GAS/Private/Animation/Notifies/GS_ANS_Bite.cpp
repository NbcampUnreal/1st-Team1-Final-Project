// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_ANS_Bite.h"
#include "Character/Player/Monster/GS_SmallClaw.h"
#include "Kismet/KismetSystemLibrary.h"

void UGS_ANS_Bite::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                               float TotalDuration)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AGS_SmallClaw* Claw = Cast<AGS_SmallClaw>(Owner))
		{
			Claw->Server_SetBiteCollision(true);
		}
	}
}

void UGS_ANS_Bite::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AGS_SmallClaw* Claw = Cast<AGS_SmallClaw>(Owner))
		{
			Claw->Server_SetBiteCollision(false);
		}
	}
}
