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
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Character/GS_TpsController.h"


UGS_AresMovingSkill::UGS_AresMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_AresMovingSkill::ActiveSkill()
{
	Super::ActiveSkill();

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		// 스킬 애니메이션 재생
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);

		// 차징 루프 사운드는 SeekerAudioComponent를 통해 처리됨

		OwnerPlayer->SetMoveControlValue(false, false);
	}

	// 차징 시작
	ChargingStartTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
	ChargingTime = 0.0f;

	// 일정 주기로 방향과 차징 시간 갱신
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(ChargingTimerHandle, this, &UGS_AresMovingSkill::UpdateCharging, 0.05f, true);

}

void UGS_AresMovingSkill::OnSkillCanceledByDebuff()
{
}

void UGS_AresMovingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();
}

void UGS_AresMovingSkill::OnSkillCommand()
{
	if (!CanActive() || !GetIsActive())
	{
		return;
	}

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[1]);
	}

	Super::OnSkillCommand();

	// 차징 루프 사운드 정지 및 돌진 사운드 재생
	// 차징 사운드 정지 및 돌진 사운드 재생
	if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
		{
			AudioComp->StopSkillLoopSoundFromDataTable(CurrentSkillType);
		}
	}
	PlaySkillStartSound();

	// 차징 종료
	OwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(ChargingTimerHandle);

	// 돌진 거리 계산
	float Ratio = ChargingTime / MaxChargingTime;
	float DashDistance = FMath::Lerp(MinDashDistance, MaxDashDistance, Ratio);

	//UE_LOG(LogTemp, Log, TEXT("[AresDash] ChargingTime: %.2f / %.2f (%.0f%%), DashDistance: %.0f units"),
	//	ChargingTime,
	//	MaxChargingTime,
	//	Ratio * 100.f,
	//	DashDistance);

	// 대시 시작
	if (IsValid(OwnerCharacter))
	{
		DashDirection = OwnerCharacter->GetActorForwardVector().GetSafeNormal();
		DashStartLocation = OwnerCharacter->GetActorLocation();
		DashEndLocation = DashStartLocation + DashDirection * DashDistance;
		DashInterpAlpha = 0.0f;
		StartDash();
	}

	// 쿨다운 시작
	StartCoolDown();
}

void UGS_AresMovingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	//AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	SetIsActive(false);
}

void UGS_AresMovingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	// 데미지
	UGameplayStatics::ApplyDamage(Target, 50.0f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
	
	// 몬스터 충돌 사운드는 SeekerAudioComponent를 통해 처리됨
}

void UGS_AresMovingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 데미지
	UGameplayStatics::ApplyDamage(Target, 50.0f, OwnerCharacter->GetController(), OwnerCharacter, nullptr);
	
	// 가디언 충돌 사운드는 SeekerAudioComponent를 통해 처리됨
}

void UGS_AresMovingSkill::UpdateCharging()
{
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	// 차징 시간 계산
	float CurrentTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
	ChargingTime = FMath::Min(CurrentTime - ChargingStartTime, MaxChargingTime);
}

void UGS_AresMovingSkill::StartDash()
{
	// 충돌 몬스터 초기화
	DamagedActors.Empty();

	// 몬스터는 충돌 막지 않도록 설정
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &UGS_AresMovingSkill::UpdateDash, 0.01f, true);
	
	// =======================
	// VFX 재생 - 컴포넌트 RPC 사용
	// =======================
	if (OwningComp)
	{
		FVector SkillLocation = OwnerCharacter->GetActorLocation();
		FRotator SkillRotation = FRotator(0.f, 0.f, 0.f);


		// 스킬 시전 VFX 재생
		OwningComp->Multicast_PlayCastVFX(CurrentSkillType, SkillLocation, SkillRotation);
	}
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
			if (!HitActor || DamagedActors.Contains(HitActor))
			{
				continue;
			}

			DamagedActors.Add(HitActor);

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

	// 대시 거리만큼 이동 하면
	if (DashInterpAlpha >= 1.f)
	{
		// 스킬 종료
		DeactiveSkill();
	}
}

void UGS_AresMovingSkill::DeactiveSkill()
{
	// 타이머 초기화
	GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);

	// 입력 제한 설정
	/*AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->SetMoveControlValue(true, true);*/
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	AresCharacter->SetMoveControlValue(true, true);
	//OwnerCharacter->SetSkillInputControl(true, true, true);

	// 원래대로 Block으로 되돌리기
	OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	OwnerCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// 스킬 종료 사운드 재생
	PlaySkillEndSound();

	Super::DeactiveSkill();
}