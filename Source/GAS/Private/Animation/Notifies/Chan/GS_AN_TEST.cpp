// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_TEST.h"

#include "Character/Player/Seeker/GS_Seeker.h"

void UGS_AN_TEST::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                         const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Fucking TEST : %s | GetName : %s"), *UEnum::GetValueAsString(Seeker->GetLocalRole()), *Seeker->GetName()); // SJE
	}
}
