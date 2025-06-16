// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanUltimateSkill.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "AkAudioEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"


UGS_ChanUltimateSkill::UGS_ChanUltimateSkill()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}

void UGS_ChanUltimateSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	
	// 궁극기 사운드 재생
	if (OwnerPlayer && OwnerPlayer->UltimateSkillSound)
	{
		OwnerPlayer->Multicast_PlaySkillSound(OwnerPlayer->UltimateSkillSound);
	}
	
	// 애니메이션 재생
	//OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);

	// 돌진 시작 (약간 딜레이)
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, this, &UGS_ChanUltimateSkill::StartCharge, 0.5f, false);
}

void UGS_ChanUltimateSkill::ExecuteSkillEffect()
{

}

void UGS_ChanUltimateSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	if (!Target) return;

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if (!OwnerPlayer) return;

	// 플레이어에서 몬스터로의 벡터
	FVector PlayerToMonster = Target->GetActorLocation() - OwnerPlayer->GetActorLocation();

	// 돌진 방향과 직각인 좌우 방향 계산
	FVector RightVector = FVector::CrossProduct(ChargeDirection, FVector::UpVector).GetSafeNormal();

	// 몬스터가 돌진 경로의 왼쪽인지 오른쪽인지 판단
	float DotProduct = FVector::DotProduct(PlayerToMonster, RightVector);

	// 넉백 방향 결정 (좌우로만)
	FVector KnockbackDirection;
	if (DotProduct > 0)
	{
		// 오른쪽에 있으면 오른쪽으로 밀어냄
		KnockbackDirection = RightVector;
	}
	else
	{
		// 왼쪽에 있으면 왼쪽으로 밀어냄
		KnockbackDirection = -RightVector;
	}

	// 넉백 속도 계산
	FVector KnockbackVelocity = KnockbackDirection * KnockbackForce;

	// 몬스터 넉백 적용
	if (Target->GetCharacterMovement())
	{
		Target->LaunchCharacter(KnockbackVelocity, true, false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Monster %s knocked back to %s!"),
		*Target->GetName(),
		DotProduct > 0 ? TEXT("RIGHT") : TEXT("LEFT"));

	//UGameplayStatics::ApplyDamage(Target, Damage, OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());
}

void UGS_ChanUltimateSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	if (!Target) return;
    
    AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
    if (!OwnerPlayer) return;
    
    // 가디언용 넉백 (더 약한 힘)
    FVector KnockbackDirection = (Target->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();
    FVector KnockbackVelocity = KnockbackDirection * GuardianKnockbackForce;
    
    if (Target->GetCharacterMovement())
    {
        Target->LaunchCharacter(KnockbackVelocity, true, false);
    }
}

void UGS_ChanUltimateSkill::StartCharge()
{
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if (!OwnerPlayer || bIsCharging)
	{
		return;
	}

	OwnerPlayer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	bIsCharging = true;
	ChargeStartLocation = OwnerPlayer->GetActorLocation(); // 돌진 시작 위치
	ChargeDirection = OwnerPlayer->GetActorForwardVector().GetSafeNormal(); // 돌진 방향
	HitActors.Empty();

	//OwnerPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	OwnerPlayer->GetCharacterMovement()->MaxFlySpeed = ChargeSpeed;

	GetWorld()->GetTimerManager().SetTimer(ChargeTimerHandle, this, &UGS_ChanUltimateSkill::UpdateCharge, ChargeTickInterval, true);

	UE_LOG(LogTemp, Warning, TEXT("Shield Charge Started with Enhanced Input!"));
}

void UGS_ChanUltimateSkill::UpdateCharge()
{
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if (!OwnerPlayer || !bIsCharging)
	{
		return;
	}

	// 돌진 거리 초과 시 종료
	float CurrentDistance = FVector::Dist(ChargeStartLocation, OwnerPlayer->GetActorLocation());
	if (CurrentDistance >= ChargeDistance)
	{
		EndCharge();
		return;
	}

	// 총 이동 벡터 계산
	FVector ForwardMovement = ChargeDirection * ChargeSpeed * ChargeTickInterval;
	FVector TotalMovement = ForwardMovement;

	// Sweep으로 충돌 감지
	TArray<FHitResult> HitResults;
	FVector Start = OwnerPlayer->GetActorLocation();
	FVector End = Start + TotalMovement;
	FCollisionShape Capsule = FCollisionShape::MakeCapsule(
		OwnerPlayer->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
		OwnerPlayer->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()
	);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, Start, End, OwnerPlayer->GetActorQuat(), ECC_Pawn, Capsule);

	if (bHit)
	{
		for (const FHitResult & Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

			if (HitActor == nullptr || HitActor == OwnerPlayer)
			{
				// 자기 자신이나 유효하지 않은 대상은 무시
			}
			else if (AGS_Guardian* Guardian = Cast<AGS_Guardian>(HitActor))
			{
				UE_LOG(LogTemp, Warning, TEXT("Charge ended: Hit Guardian %s"), *Guardian->GetName());
				ApplyEffectToGuardian(Guardian);
				EndCharge();
				return;
			}
			else if (AGS_Monster* Monster = Cast<AGS_Monster>(HitActor))
			{
				if (!HitActors.Contains(Monster))
				{
					UE_LOG(LogTemp, Warning, TEXT("Hit Monster: %s"), *Monster->GetName());
					//HitActors.Add(Monster);
					ApplyEffectToDungeonMonster(Monster);
				}
				// 돌진 계속함
			}
			else if (HitActor->ActorHasTag("Wall") || HitActor->ActorHasTag("Obstacle") || HitActor->IsA<AStaticMeshActor>())
			{
				UE_LOG(LogTemp, Warning, TEXT("Charge ended: Hit Wall-like Object %s"), *HitActor->GetName());
				EndCharge();
				return;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit Unknown Actor: %s"), *HitActor->GetName());
				// 그 외는 무시
			}
		}
	}

	// [MM 대응] 이동 적용: Velocity 직접 설정
	FVector ChargeVelocity = TotalMovement / ChargeTickInterval;
	if (OwnerPlayer->GetCharacterMovement())
	{
		OwnerPlayer->GetCharacterMovement()->Velocity = ChargeVelocity; // [MM 대응] LaunchCharacter 대신 Velocity 설정

		// [MM 대응] 필요 시 감속 제어 설정
		OwnerPlayer->GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
		OwnerPlayer->GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
	}

	// 회전 보정
	if (!TotalMovement.IsZero())
	{
		FRotator TargetRotation = TotalMovement.Rotation();
		FRotator NewRotation = FMath::RInterpTo(OwnerPlayer->GetActorRotation(), TargetRotation, ChargeTickInterval, 3.0f);
		OwnerPlayer->SetActorRotation(NewRotation);
	}
}


void UGS_ChanUltimateSkill::EndCharge()
{
	if (!bIsCharging) return;

	bIsCharging = false;

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if (OwnerPlayer)
	{
		OwnerPlayer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		OwnerPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		CurrentLateralInput = 0.0f;
	}

	GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("Shield Charge Ended!"));
}
