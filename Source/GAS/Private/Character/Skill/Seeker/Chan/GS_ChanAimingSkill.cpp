// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Debuff/EDebuffType.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Kismet/GameplayStatics.h"

UGS_ChanAimingSkill::UGS_ChanAimingSkill()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_ChanAimingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
	StartHoldUp();
}

void UGS_ChanAimingSkill::OnSkillCommand()
{
	if (!bIsHoldingUp || CurrentStamina < SlamStaminaCost)
	{
		return;
	}
	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
	OnShieldSlam();
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
		TSet<AActor*> HitActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			UPrimitiveComponent* HitComponent = Hit.GetComponent();

			if (!HitActor || HitActors.Contains(HitActor))
			{
				continue;
			}

			HitActors.Add(HitActor);

			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s, Hit Component: %s"),
				*HitActor->GetName(),
				*HitComponent->GetName());

			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor))
			{
				ApplyEffectToDungeonMonster(TargetMonster);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor))
			{
				ApplyEffectToGuardian(TargetGuardian);
			}
			else if(AGS_Character* Target = Cast<AGS_Character>(HitActor))
			{
				// 넉백
				const FVector LaunchDirection = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				Target->LaunchCharacter(LaunchDirection * 1000.0f + FVector(0, 0, 500.0f), true, true);

				// 경직 디버프
				if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
				{
					Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Stun, OwnerCharacter);
				}
			}
		}
	}
}

bool UGS_ChanAimingSkill::IsActive() const
{
	return bIsHoldingUp;
}

void UGS_ChanAimingSkill::OnShieldSlam()
{
	UE_LOG(LogTemp, Warning, TEXT("Slam!!!!!!!"));
	CurrentStamina -= SlamStaminaCost;
	// UI 업데이트
	UpdateProgressBar(CurrentStamina);
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
	UpdateProgressBar(CurrentStamina);

	UE_LOG(LogTemp, Warning, TEXT("Stamina : %f"), CurrentStamina);
	if (CurrentStamina <= 0.f)
	{
		EndHoldUp();
	}
}

void UGS_ChanAimingSkill::StartHoldUp()
{
	bIsHoldingUp = true;
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, true);
	}
	CurrentStamina = MaxStamina;

	// UI 표시
	ShowProgressBar(true);
	UpdateProgressBar(CurrentStamina);

	UE_LOG(LogTemp, Warning, TEXT("Start Hold Up!!!!!!!"));
	OwnerCharacter->GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &UGS_ChanAimingSkill::TickDrainStamina, 1.0f, true);
}

void UGS_ChanAimingSkill::EndHoldUp()
{
	bIsHoldingUp = false;
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, false);
	}
	// UI 숨기기
	ShowProgressBar(false);
	OwnerCharacter->GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
	UE_LOG(LogTemp, Warning, TEXT("End Hold Up!!!!!!"));
}

void UGS_ChanAimingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	if (!Target) return;

	// 넉백
	const FVector LaunchDirection = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
	Target->LaunchCharacter(LaunchDirection * 500.f + FVector(0, 0, 200.f), true, true);

	// 데미지
	UGameplayStatics::ApplyDamage(Target, Damage, OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());

	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Stun, OwnerCharacter);
	}
	
}

void UGS_ChanAimingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 데미지
	UGameplayStatics::ApplyDamage(Target, Damage, OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());

	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Stun, OwnerCharacter);
	}
}

void UGS_ChanAimingSkill::UpdateProgressBar(float InStamina)
{
	AGS_Chan* OwnerChan = Cast<AGS_Chan>(OwnerCharacter);
	OwnerChan->Client_UpdateChanAimingSkillBar(InStamina / MaxStamina);
}

void UGS_ChanAimingSkill::ShowProgressBar(bool bShow)
{
	AGS_Chan* OwnerChan = Cast<AGS_Chan>(OwnerCharacter);
	OwnerChan->Client_ChanAimingSkillBar(bShow);
}
