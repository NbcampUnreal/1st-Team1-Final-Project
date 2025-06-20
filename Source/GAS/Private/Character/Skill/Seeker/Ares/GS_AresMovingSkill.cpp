// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresMovingSkill.h"
#include "Character/GS_Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/Player/Seeker/GS_Ares.h"


UGS_AresMovingSkill::UGS_AresMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_AresMovingSkill::ActiveSkill()
{
	if (!CanActiveInternally())
	{
		bPressedDuringCooldown = true;
		return;
	}

	Super::ActiveSkill();

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
	}

	bPressedDuringCooldown = false;

	// 차징 시작
	ChargingStartTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
	ChargingTime = 0.0f;
	bIsCharging = true;

	// 일정 주기로 방향과 차징 시간 갱신
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(ChargingTimerHandle, this, &UGS_AresMovingSkill::UpdateCharging, 0.05f, true);

}

void UGS_AresMovingSkill::DeactiveSkill()
{
}

void UGS_AresMovingSkill::OnSkillCommand()
{
	if (!CanActiveInternally() || bPressedDuringCooldown)
	{
		return;
	}

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[1]);
	}

	Super::OnSkillCommand();

	// 차징 종료
	bIsCharging = false;
	OwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(ChargingTimerHandle);

	// 돌진 거리 계산
	float Ratio = ChargingTime / MaxChargingTime;
	float DashDistance = FMath::Lerp(MinDashDistance, MaxDashDistance, Ratio);

	UE_LOG(LogTemp, Log, TEXT("[AresDash] ChargingTime: %.2f / %.2f (%.0f%%), DashDistance: %.0f units"),
		ChargingTime,
		MaxChargingTime,
		Ratio * 100.f,
		DashDistance);

	if (IsValid(OwnerCharacter))
	{
		DashDirection = OwnerCharacter->GetActorForwardVector().GetSafeNormal();
		DashStartLocation = OwnerCharacter->GetActorLocation();
		DashEndLocation = DashStartLocation + DashDirection * DashDistance;
		DashInterpAlpha = 0.0f;
		StartDash();
	}

	StartCoolDown();
}

void UGS_AresMovingSkill::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();

	if (!IsValid(OwnerCharacter)) return;

	// 돌진 경로에 적 감지(캡슐 스윕 사용)
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->SweepMultiByChannel(HitResults, DashStartLocation, DashEndLocation,
		FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(100.0f, 100.0f), Params);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!IsValid(HitActor))
			{
				return;
			}

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

	// 캐릭터를 앞으로 밀어냄
	float Ratio = ChargingTime / MaxChargingTime;
	float DashDistance = FMath::Lerp(MinDashDistance, MaxDashDistance, Ratio);
	float DashImpulseStrength = DashDistance * 10.0f; // 튜닝 가능

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->StopMovementImmediately();
		MoveComp->AddImpulse(DashDirection * DashImpulseStrength, true); // true: mass 고려
	}

	// 디버그 선 그리기
	/*DrawDebugCapsule(GetWorld(), DashStartLocation, 100.0f, 100.0f, FQuat::Identity, FColor::Red, false, 2.0f);
	DrawDebugLine(GetWorld(), DashStartLocation, DashEndLocation, FColor::Blue, false, 2.0f, 0, 3.0f);*/
}

bool UGS_AresMovingSkill::IsActive() const
{
	return bIsCharging;
}

void UGS_AresMovingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	// 데미지
	UGameplayStatics::ApplyDamage(Target, 50.0f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
}

void UGS_AresMovingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 데미지
	UGameplayStatics::ApplyDamage(Target, 50.0f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
}

void UGS_AresMovingSkill::UpdateCharging()
{
	if (!IsValid(OwnerCharacter)) return;

	float CurrentTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
	ChargingTime = FMath::Min(CurrentTime - ChargingStartTime, MaxChargingTime);
}

void UGS_AresMovingSkill::StartDash()
{
	// 몬스터는 충돌 막지 않도록 설정
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &UGS_AresMovingSkill::UpdateDash, 0.01f, true);
}

void UGS_AresMovingSkill::UpdateDash()
{
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	float Step = 0.01f / DashDuration;
	DashInterpAlpha += Step;

	// 공격 판정
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	GetWorld()->SweepMultiByChannel(HitResults, DashStartLocation, DashEndLocation,
		FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(100.0f, 100.0f), Params);

	for (const FHitResult& Hit : HitResults)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
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

	// 위치 보간 이동
	FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, DashInterpAlpha);
	OwnerCharacter->SetActorLocation(NewLocation, true); // Sweep = true로 충돌 적용
	DashStartLocation = NewLocation;

	if (DashInterpAlpha >= 1.f)
	{
		StopDash();
	}
}

bool UGS_AresMovingSkill::CanActiveInternally() const
{
	return OwnerCharacter && !bIsCoolingDown;
}

void UGS_AresMovingSkill::StopDash()
{
	GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);

	// 원래대로 Block으로 되돌리기
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}
