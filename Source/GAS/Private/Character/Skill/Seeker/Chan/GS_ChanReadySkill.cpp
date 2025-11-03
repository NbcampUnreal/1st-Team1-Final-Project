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

		if (UAnimInstance* AnimInstance = OwnerPlayer->GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &UGS_ChanReadySkill::OnMontageEnded);
			ActiveMontage = TargetMontage; // 추적용 변수
		}

		// 스테미나 이벤트 구독
		OwnerPlayer->OnStaminaDepleted.AddUniqueDynamic(this, &UGS_ChanReadySkill::HandleStaminaDepleted);
	}

	// DeactiveMontageIndex 초기화
	DeactiveMontageIndex = 0;
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
	UE_LOG(LogTemp, Error, TEXT("OnSkillAnimationEnd - ChanReadySkill"));
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

void UGS_ChanReadySkill::HandleStaminaDepleted(bool bByDamage)
{
	if (bByDamage)
	{
		UE_LOG(LogTemp, Error, TEXT("Stamina 0 - Cause Damage"));
		// 기본 애니메이션
		DeactiveMontageIndex = 1;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Stamina 0 - Cause Drain"));
		// 기본 애니메이션
		DeactiveMontageIndex = 0;
	}
}

void UGS_ChanReadySkill::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!OwnerCharacter) return;

	if (SkillAnimMontages.Contains(Montage))
	{
		UE_LOG(LogTemp, Warning, TEXT("Montage Ended: %s (Interrupted: %s)"),
			*Montage->GetName(),
			bInterrupted ? TEXT("True") : TEXT("False"));

		UE_LOG(LogTemp, Warning, TEXT("Ended Montage Name: %s | Path: %s | Addr: %p"),
			*Montage->GetName(),
			*Montage->GetPathName(),
			Montage);

		if (SkillAnimMontages.IsValidIndex(1))
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillAnimMontages[1] Name: %s | Path: %s | Addr: %p"),
				*SkillAnimMontages[1]->GetName(),
				*SkillAnimMontages[1]->GetPathName(),
				SkillAnimMontages[1]);
		}

		FName CurrentMontageName = NAME_None;
		if (UAnimMontage* CurrentMontage = OwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())
		{
			CurrentMontageName = CurrentMontage->GetFName();
		}
		UE_LOG(LogTemp, Warning, TEXT("AnimationEnded 현재 애니메이션 몽타주: %s"), *CurrentMontageName.ToString());

		
		// 애니메이션 종료 처리 (Notify가 빠졌을 경우에도 안전하게)
		if (Montage == SkillAnimMontages[1])
		{
			OnSkillAnimationEnd();
			if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
			{
				AnimInstance->OnMontageEnded.RemoveDynamic(this, &UGS_ChanReadySkill::OnMontageEnded);
			}
		}

		
	}
}

void UGS_ChanReadySkill::DeactiveSkill()
{
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// Set HitReact
		OwnerPlayer->SetCanHitReact(true);
		OwnerPlayer->CanChangeSeekerGait = true;
		OwnerPlayer->SetSeekerGait(EGait::Run);

		// 애니메이션 재생
		FName SectionName = NAME_None;
		if (DeactiveMontageIndex == 0)
		{
			SectionName = FName("LoopEnd");
		}
		else if (DeactiveMontageIndex == 1)
		{
			OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
		}

		
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[DeactiveMontageIndex], SectionName);

		// 현재 재생 중인 몽타주가 있으면
		FName CurrentMontageName = NAME_None;
		if (UAnimMontage* CurrentMontage = OwnerCharacter->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())
		{
			CurrentMontageName = CurrentMontage->GetFName();
		}
		UE_LOG(LogTemp, Warning, TEXT("현재 애니메이션 몽타주: %s"), *CurrentMontageName.ToString());

		// 방어 상태 비활성화 (스킬 완전 종료 시)
		OwnerPlayer->SetDefending(false);
	}

	// 스킬 상태 업데이트
	Super::DeactiveSkill();
}

