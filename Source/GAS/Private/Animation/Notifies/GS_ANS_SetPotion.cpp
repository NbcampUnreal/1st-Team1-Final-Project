// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_ANS_SetPotion.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Props/Item/SeekerItem/GS_HP_Potion.h"
#include "Character/Skill/Seeker/GS_HealSkill.h"


void UGS_ANS_SetPotion::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner());

	if (!Seeker)
	{
		return;
	}

	UWorld* World = Seeker->GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Seeker; // 생성 주체 지정
	
	AGS_HP_Potion* Potion = World->SpawnActor<AGS_HP_Potion>(AGS_HP_Potion::StaticClass(), SpawnParams);
	if (!Potion)
	{
		return;
	}

	Potion->SetItemData(Seeker->GetItemData(EItemType::HP_Potion));
	Potion->SetMesh(FName(TEXT("HP_Potion_Full")));

	Seeker->Items.Add(EItemType::HP_Potion, Potion);

	Potion->AttachToComponent(Seeker->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Potion"));
}

void UGS_ANS_SetPotion::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner());
	if (!Seeker)
	{
		return;
	}
	AGS_HP_Potion* Potion = Cast<AGS_HP_Potion>(Seeker->GetItem(EItemType::HP_Potion));
	if (!Potion)
	{
		return;
	}
	
	Potion->DropFromSocket();
	
	UStaticMeshComponent* Mesh = Potion->GetMeshComp();
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	UGS_SkillComp* SkillComp = Seeker->GetSkillComp();
	if (!SkillComp)
	{
		return;
	}

	UGS_HealSkill* HealSkill = Cast<UGS_HealSkill>(SkillComp->GetSkillFromSkillMap(ESkillSlot::HealPotion));
	if (!HealSkill)
	{
		return;
	}
	HealSkill->DeactiveSkill();
}
