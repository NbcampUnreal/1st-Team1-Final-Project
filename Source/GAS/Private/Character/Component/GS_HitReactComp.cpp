// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_HitReactComp.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Skill/Seeker/GS_HealSkill.h"


// Sets default values for this component's properties
UGS_HitReactComp::UGS_HitReactComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	AM_HitReacts.Init(nullptr, static_cast<int>(EHitReactType::TypeNum));
}

void UGS_HitReactComp::PlayHitReact(EHitReactType ReactType, FVector HitDirection)
{
	FName Section = CalculateHitDirection(HitDirection);
	AGS_Player* OwnerCharacter = Cast<AGS_Player>(GetOwner());
	AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter);
	if (OwnerCharacter)
	{
		if (ReactType == EHitReactType::Interrupt)
		{			
			if (OwnerSeeker)
			{
				OwnerSeeker->GetSkillComp()->SkillsInterrupt();

				UGS_SeekerAnimInstance* SeekerAnimInstance = Cast<UGS_SeekerAnimInstance>(OwnerSeeker->GetMesh()->GetAnimInstance());

				OwnerSeeker->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
				
				UAnimMontage* AM_HitReact = AM_HitReacts[static_cast<int>(ReactType)];
				if (AM_HitReact)
				{
					OwnerCharacter->Multicast_PlaySkillMontage(AM_HitReact, Section);
					HitReactEndDelegate.BindUObject(this, &UGS_HitReactComp::OnEndDelegate);
					SeekerAnimInstance->Montage_SetEndDelegate(HitReactEndDelegate, AM_HitReact);
				}
			}
			
			OwnerCharacter->DisableHitReact(4.0f);
		}
		else if (ReactType == EHitReactType::Additive)
		{
			if (OwnerSeeker) // 추후 수정. // SJE
			{
				OwnerSeeker->StateReset();
			}
		}
		else if (ReactType == EHitReactType::DamageOnly)
		{
			if (OwnerSeeker)
			{
				OwnerSeeker->StateReset();
			}
		}


		if (OwnerSeeker)
		{
			OwnerSeeker->SetAimState(false);
			OwnerSeeker->SetDrawState(false);
		}
	}
}


void UGS_HitReactComp::StopHitReact(UAnimMontage* TargetMontage)
{
	if (AGS_Player* OwnerCharacter = Cast<AGS_Player>(GetOwner()))
	{
		OwnerCharacter->Multicast_StopSkillMontage(TargetMontage);
	}
}

FName UGS_HitReactComp::CalculateHitDirection(FVector HitDirection)
{
	FName Section = NAME_None;
	
	if (AGS_Player* OwnerCharacter = Cast<AGS_Player>(GetOwner()))
	{
		FVector Front = OwnerCharacter->GetActorRotation().Vector();
		FVector Right = OwnerCharacter->GetActorRightVector();

		float FrontDot = FVector::DotProduct(Front, HitDirection);
		float RightDot = FVector::DotProduct(Right, HitDirection);

		if (FrontDot > 0.7f)
		{
			Section = FName("Front");
		}
		else if (FrontDot < -0.7f)
		{
			Section = FName("Back");
		}
		else if (RightDot > 0.0f)
		{
			Section = FName("Right");
		}
		else
		{
			Section = FName("Left");
		}
	}
	
	return Section;
}

void UGS_HitReactComp::OnEndDelegate(UAnimMontage* Montage, bool bInterrupted)
{	
	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(GetOwner()))
	{
		UGS_HealSkill* HealSkill = Cast<UGS_HealSkill>(Seeker->GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::HealPotion));
		if (HealSkill)
		{
			UAnimMontage* AM_Wielding = HealSkill->SkillAnimMontages[2];

			if (AM_Wielding)
			{
				Seeker->TransWeaponHandingState(
				EWeaponHandlingState::Sheathing,
				EWeaponHandlingState::Wielding,
				AM_Wielding,
				ESeekerMontageSlot::UpperBody);
			}
		}
	}
}

// Called when the game starts
void UGS_HitReactComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

