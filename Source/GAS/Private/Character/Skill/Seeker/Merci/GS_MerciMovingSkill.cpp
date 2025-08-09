// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciMovingSkill.h"
#include "Sound/GS_CharacterAudioComponent.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"

UGS_MerciMovingSkill::UGS_MerciMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_MerciMovingSkill::ActiveSkill()
{
	Super::ActiveSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (MerciCharacter)
	{
		if (UGS_CharacterAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_CharacterAudioComponent>())
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
		}
		
		MerciCharacter->SetDrawState(false);

		// 활 당기기
		MerciCharacter->DrawBow(SkillAnimMontages[0]);
	}
}

void UGS_MerciMovingSkill::OnSkillAnimationEnd()
{
}

void UGS_MerciMovingSkill::OnSkillCommand()
{
	if (!CanActive() || !GetIsActive())
	{
		return;
	}

	// 활 놓기
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	bool IsFullyDrawn = MerciCharacter->GetIsFullyDrawn();

	if (MerciCharacter->SmokeArrowClass)
	{
		MerciCharacter->ReleaseArrow(MerciCharacter->SmokeArrowClass);
	}
	
	if (IsFullyDrawn)
	{
		// 쿨타임 측정 시작
		StartCoolDown();
	}

	// 스킬 종료
	DeactiveSkill();
}

void UGS_MerciMovingSkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	SetIsActive(false);
}

void UGS_MerciMovingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();
}

