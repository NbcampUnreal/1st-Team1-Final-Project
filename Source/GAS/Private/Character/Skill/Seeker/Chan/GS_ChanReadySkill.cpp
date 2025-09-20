// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanReadySkill.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Sound/GS_SeekerAudioComponent.h"

UGS_ChanReadySkill::UGS_ChanReadySkill()
{
	CurrentSkillType = ESkillSlot::Ready;
}

void UGS_ChanReadySkill::ActiveSkill()
{
	Super::ActiveSkill();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// Change Slot
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::UpperBody);

		OwnerPlayer->Multicast_SetMustTurnInPlace(true);
		OwnerPlayer->SetSeekerGait(EGait::Walk);

		// Play Montage
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
		OwnerPlayer->CanChangeSeekerGait = false;

		// 스킬 시작 사운드 재생
		if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
		}

		if (OwningComp)
		{
			FVector SkillLocation = OwnerCharacter->GetActorLocation();
			FRotator SkillRotation = OwnerCharacter->GetActorRotation();

			// 스킬 시전 VFX 재생
			OwningComp->Multicast_PlayCastVFX(CurrentSkillType, SkillLocation, SkillRotation);
		}

		// 방어 상태 활성화
		OwnerPlayer->SetDefending(true);
	}
}

void UGS_ChanReadySkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();

	// 방어 상태 비활성화
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		OwnerPlayer->SetDefending(false);
	}
}

void UGS_ChanReadySkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// Change Slot
		OwnerPlayer->Multicast_SetMustTurnInPlace(false);
		OwnerPlayer->SetSeekerGait(EGait::Run);
		// Change Slot
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);

		OwnerPlayer->CanChangeSeekerGait = true;

		OwnerPlayer->SetMoveControlValue(true, true);
		OwnerPlayer->SetLookControlValue(true, true);

		// =======================
		// 스킬 종료 VFX 재생
		// =======================

		if (OwningComp)
		{
			FVector SkillLocation = OwnerCharacter->GetActorLocation();
			FRotator SkillRotation = OwnerCharacter->GetActorRotation();

			// 스킬 종료 VFX 재생
			OwningComp->Multicast_PlayEndVFX(CurrentSkillType, SkillLocation, SkillRotation);
		}
	}
}

void UGS_ChanReadySkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);

	OwnerPlayer->SetLookControlValue(true, true);
	SetIsActive(false);

	// 방어 상태 비활성화 (스킬이 중단될 때)
	OwnerPlayer->SetDefending(false);
}

void UGS_ChanReadySkill::DeactiveSkill()
{
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// Set HitReact
		OwnerPlayer->SetCanHitReact(true);
		OwnerPlayer->CanChangeSeekerGait = true;
		OwnerPlayer->SetSeekerGait(EGait::Run);
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0], FName("LoopEnd"));

		// 방어 상태 비활성화 (스킬 완전 종료 시)
		OwnerPlayer->SetDefending(false);
	}

	// 스킬 상태 업데이트
	Super::DeactiveSkill();
}
