// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Debuff/EDebuffType.h"

void UGS_ChanAimingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();

	StartHoldUp();
}

void UGS_ChanAimingSkill::ExecuteSkillEffect()
{
	TArray<FHitResult> HitResults;

	const FVector Start = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	const float Radius = 200.f;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	if (OwnerCharacter->GetWorld()->SweepMultiByChannel(HitResults, Start, Start + Forward * 100.f, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor) continue;

			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor))
			{
				ApplyEffectToDungeonMonster(TargetMonster);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor))
			{
				ApplyEffectToGuardian(TargetGuardian);
			}

			
		}
	}
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
	UE_LOG(LogTemp, Warning, TEXT("Stamina : %f"), CurrentStamina);
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
	UE_LOG(LogTemp, Warning, TEXT("Start Hold Up!!!!!!!"));
	OwnerCharacter->GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &UGS_ChanAimingSkill::TickDrainStamina, 1.0f, true);
}

void UGS_ChanAimingSkill::EndHoldUp()
{
	bIsHoldingUp = false;
	// UI 숨기기
	OwnerCharacter->GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
	UE_LOG(LogTemp, Warning, TEXT("End Hold Up!!!!!!"));
}

void UGS_ChanAimingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	if (!Target) return;

	// 넉백
	const FVector LaunchDirection = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
	Target->LaunchCharacter(LaunchDirection * 500.f + FVector(0, 0, 200.f), true, true);

	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->DebuffComp->ApplyDebuff(EDebuffType::Stun);
	}
	
}

void UGS_ChanAimingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->DebuffComp->ApplyDebuff(EDebuffType::Stun);
	}
}
