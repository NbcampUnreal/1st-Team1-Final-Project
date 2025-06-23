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
#include "Character/Debuff/EDebuffType.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/GS_TpsController.h"
#include "AIController.h"



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

void UGS_ChanUltimateSkill::HandleUltimateCollision(AActor* HitActor)
{
	if (AGS_Guardian* Guardian = Cast<AGS_Guardian>(HitActor))
	{
		ApplyEffectToGuardian(Guardian);
		EndCharge();
	}
	else if (AGS_Monster* Monster = Cast<AGS_Monster>(HitActor))
	{
		if (!HitActors.Contains(Monster))
		{
			HitActors.Add(Monster);
			ApplyEffectToDungeonMonster(Monster);
		}
	}
}

void UGS_ChanUltimateSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	if (!Target) return;

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if (!OwnerPlayer) return;

	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Stun, OwnerCharacter);
	}

	// 현재 프레임의 실시간 방향 계산
	FVector CurrentForward = OwnerPlayer->GetActorForwardVector().GetSafeNormal(); // 실시간
	FVector PlayerToMonster = Target->GetActorLocation() - OwnerPlayer->GetActorLocation();
	FVector RightVector = FVector::CrossProduct(CurrentForward, FVector::UpVector).GetSafeNormal();

	// Dot 비교로 방향 판별
	float DotProduct = FVector::DotProduct(PlayerToMonster, RightVector);
	FVector KnockbackDirection = (DotProduct > 0) ? RightVector : -RightVector;

	FVector KnockbackVelocity = KnockbackDirection * KnockbackForce;
	KnockbackVelocity.Z = 300.0f;

	if (Target->GetCharacterMovement())
	{
		if (AAIController* AICon = Cast<AAIController>(Target->GetController()))
		{
			AICon->StopMovement();
		}

		Target->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		UE_LOG(LogTemp, Warning, TEXT("Monster %s knocked back | Velocity: %s | Mode: %d | RootMotion: %s"),
			*Target->GetName(),
			*KnockbackVelocity.ToString(),
			(int32)Target->GetCharacterMovement()->MovementMode,
			Target->IsPlayingRootMotion() ? TEXT("TRUE") : TEXT("FALSE")
		);

		Target->LaunchCharacter(KnockbackVelocity, true, true);
	}

	// 몬스터 넉백 적용
	if (Target->GetCharacterMovement())
	{
		Target->LaunchCharacter(KnockbackVelocity, true, true);
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
	bIsCharging = true;
	
	// KnockBack Collision On
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	OwnerPlayer->UltimateCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	GetWorld()->GetTimerManager().SetTimer(
		ChargeTimerHandle,
		this,
		&UGS_ChanUltimateSkill::EndCharge,
		2.0f, 
		false
	);

	// 속도 조절
	OwnerCharacter->Server_SetCharacterSpeed(3.0f);

	// 자동 이동 시작
	AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->SetMoveControlValue(true, true);
	Controller->StartAutoMoveForward();
}


void UGS_ChanUltimateSkill::EndCharge()
{
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	// EndCharge 안에서
	OwnerPlayer->UltimateCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 속도 조절
	OwnerCharacter->Server_SetCharacterSpeed(1.0f);

	// 자동 이동 종료
	AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->StopAutoMoveForward();

	GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);

	HitActors.Empty();

	bIsCharging = false;
}
