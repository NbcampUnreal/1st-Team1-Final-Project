// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Character/GS_Character.h"

void UGS_ChanAimingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();

	StartHoldUp();
}

void UGS_ChanAimingSkill::ExecuteSkillEffect()
{
	TArray<AActor*> HitActors;

	const FVector Start = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	const float Radius = 200.f;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	/*if (OwnerCharacter->GetWorld()->SweepMultiByChannel(HitActors, Start, Start + Forward * 100.f, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		for (AActor* Actor : HitActors)
		{
			
		}
	}*/
}

void UGS_ChanAimingSkill::OnShieldSlam()
{
	if (!bIsHoldingUp || CurrentStamina < SlamStaminaCost) return;

	CurrentStamina -= SlamStaminaCost;
	// UI 업데이트

	ExecuteSkillEffect();

	if (CurrentStamina <= 0.f)
	{
		EndHoldUp();
	}
}

void UGS_ChanAimingSkill::TickDrainStamina()
{
	CurrentStamina -= StaminaDrainRate;
	// UI 업데이트

	if (CurrentStamina <= 0.f)
	{
		EndHoldUp();
	}
}

void UGS_ChanAimingSkill::StartHoldUp()
{
	bIsHoldingUp = true;
	CurrentStamina = MaxStamina;

	// UI 표시

	OwnerCharacter->GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &UGS_ChanAimingSkill::TickDrainStamina, 1.0f, true);
}

void UGS_ChanAimingSkill::EndHoldUp()
{
	bIsHoldingUp = false;
	// UI 숨기기
	OwnerCharacter->GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
}

void UGS_ChanAimingSkill::ApplyEffectToDungeonMonster()
{
}

void UGS_ChanAimingSkill::ApplyEffectToBoss()
{
}
