// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_AN_ChangePotionType.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Props/Item/SeekerItem/GS_HP_Potion.h"
#include "Components/Capsulecomponent.h"

void UGS_AN_ChangePotionType::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner());

	if (!Seeker)
	{
		return;
	}

	AGS_HP_Potion* Potion = Cast<AGS_HP_Potion>(Seeker->GetItem(EItemType::HP_Potion));
	if (Potion)
	{
		Potion->SetMesh(PotionStaticName);
		
		Potion->AttachToComponent(
			Seeker->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			FName("Potion"));

		Potion->GetMeshComp()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}
