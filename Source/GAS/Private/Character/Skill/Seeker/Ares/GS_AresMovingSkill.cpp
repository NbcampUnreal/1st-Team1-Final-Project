// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresMovingSkill.h"
#include "Character/GS_Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"

void UGS_AresMovingSkill::ActiveSkill()
{
	Super::ActiveSkill();

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
	Super::OnSkillCommand();

	// 차징 종료
	bIsCharging = false;
	OwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(ChargingTimerHandle);

	// 돌진 거리 계산
	float Ratio = ChargingTime / MaxChargingTime;
	float DashDistance = FMath::Lerp(MinDashDistance, MaxDashDistance, Ratio);

	if (IsValid(OwnerCharacter))
	{
		DashStartLocation = OwnerCharacter->GetActorLocation();
		DashEndLocation = DashStartLocation + DashDirection * DashDistance;
	}

	ExecuteSkillEffect();
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
	OwnerCharacter->LaunchCharacter(DashDirection * 1500.0f, true, true);

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
	// 경과 시간 측정
	float CurrentTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
	ChargingTime = CurrentTime - ChargingStartTime;
	ChargingTime = FMath::Min(ChargingTime, MaxChargingTime);

	// 현재 바라보는 방향 저장
	if (IsValid(OwnerCharacter))
	{
		DashDirection = OwnerCharacter->GetActorForwardVector();
	}
}
