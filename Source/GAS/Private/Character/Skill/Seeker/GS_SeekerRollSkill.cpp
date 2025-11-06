// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/GS_SeekerRollSkill.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Skill/Seeker/GS_HealSkill.h"
#include "Components/CapsuleComponent.h"
#include "Sound/GS_SeekerAudioComponent.h"

UGS_SeekerRollSkill::UGS_SeekerRollSkill()
{
	CurrentSkillType = ESkillSlot::Rolling;
}

void UGS_SeekerRollSkill::ActiveSkill()
{
	Super::ActiveSkill();

	UE_LOG(LogTemp, Warning, TEXT("SeekerRollSkill")); // SJE
	
	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (Seeker->HasAuthority())
		{
			Seeker->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
			Seeker->CanChangeSeekerGait = false;

			const FName RollDirection = CalRollDirection();
			UAnimMontage* AM_Roll = SkillAnimMontages[0];
			if (AM_Roll)
			{
				if (RollDirection == FName("00"))
				{
					Seeker->Multicast_PlaySkillMontage(AM_Roll, FName("F0"));
				}
				else
				{
					Seeker->Multicast_PlaySkillMontage(AM_Roll, RollDirection);
				}
			}
			
			EndDelegate.BindUObject(this, &UGS_SeekerRollSkill::OnRollMontageEnded);
			UGS_SeekerAnimInstance* SeekerAnimInstance = Cast<UGS_SeekerAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance());
			if (SeekerAnimInstance)
			{
				SeekerAnimInstance->Montage_SetEndDelegate(EndDelegate, AM_Roll);
			}

			Seeker->Multicast_SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

			// 스킬 시작 사운드 재생 (멀티캐스트)
			if (UGS_SeekerAudioComponent* AudioComp = Seeker->SeekerAudioComponent)
			{
				AudioComp->RequestSkillAudio(CurrentSkillType, 0);
			}
		}
		StartCoolDown();
	}
}

void UGS_SeekerRollSkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();
}

void UGS_SeekerRollSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();
}

void UGS_SeekerRollSkill::InterruptSkill()
{
	Super::InterruptSkill();
}

void UGS_SeekerRollSkill::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		UGS_HealSkill* HealSkill = Cast<UGS_HealSkill>(Seeker->GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::HealPotion));
		if (HealSkill)
		{
			UAnimMontage* AM_Wielding = HealSkill->SkillAnimMontages[2]; // Hard coding // SJE

			Seeker->TransWeaponHandingState(
			EWeaponHandlingState::Sheathing,
			EWeaponHandlingState::Wielding,
			AM_Wielding,
			ESeekerMontageSlot::UpperBody);
		}
	}

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		Seeker->Multicast_SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}