// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresAimingSkill.h"
#include "Character/Skill/Seeker/Ares/GS_AresUltimateSkill.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Weapon/Projectile/Seeker/GS_SwordAuraProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Character/GS_TpsController.h"
#include "Sound/GS_SeekerAudioComponent.h"

UGS_AresAimingSkill::UGS_AresAimingSkill()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_AresAimingSkill::ActiveSkill()
{
	Super::ActiveSkill();

	// 쿨타임 측정 시작
	StartCoolDown();

	if (AGS_Seeker* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		// 스킬 시작 사운드 재생
		if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
		}

		// 스킬 애니메이션 재생
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);

		// 입력 제한
		OwnerPlayer->SetMoveControlValue(false, false);
	}
	
	// 투사체 1차 발사
	SpawnFirstProjectile();
}

void UGS_AresAimingSkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();
}

void UGS_AresAimingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();
}

void UGS_AresAimingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	SetIsActive(false);
}

void UGS_AresAimingSkill::DeactiveSkill()
{
	/*AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	// 입력 제한 설정
	AresCharacter->SetLookControlValue(true, true);*/

	// SeekerAudioComponent를 통한 스킬 종료 사운드
	if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, false);
		}
	}

	Super::DeactiveSkill();
}

void UGS_AresAimingSkill::SpawnFirstProjectile()
{
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	if (!OwnerCharacter || !AresCharacter || !AresCharacter->AresProjectileClass)
	{
		return;
	}

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World)
	{
		return;
	}

	// 발사 방향: 카메라 정면, 위로 뜨지 않게 Z 제거
	FVector LaunchDirection = OwnerCharacter->GetControlRotation().Vector();
	LaunchDirection.Z = 0.f;
	LaunchDirection = LaunchDirection.GetSafeNormal();

	// 생성 위치: 캐릭터 정면 약간 앞 + 위
	FVector SpawnLocation = OwnerCharacter->GetActorLocation()
		+ FVector(0, 0, 0)
		+ LaunchDirection * -300.f;

	// 첫 번째 발사 (기본 방향)
	FRotator SpawnRotationA = LaunchDirection.Rotation();
	SpawnRotationA.Roll = -45.f;
	FTransform SpawnTransform(SpawnRotationA, SpawnLocation);

	AGS_SwordAuraProjectile* ProjectileA = World->SpawnActorDeferred<AGS_SwordAuraProjectile>(
		AresCharacter->AresProjectileClass,
		SpawnTransform,
		OwnerCharacter,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	// 궁극기 활성화 상태 확인
	UGS_SkillComp* SkillComp = OwnerCharacter->GetSkillComp();
	if (SkillComp)
	{
		if (UGS_AresUltimateSkill* UltimateSkill = Cast<UGS_AresUltimateSkill>(SkillComp->GetSkillFromSkillMap(ESkillSlot::Ultimate)))
		{
			bIsBerserker = UltimateSkill->GetIsActive();
		}

	}

	// 궁극기 활성화 상태에 따라 투사체 모양 설정
	if (ProjectileA)
	{
		ProjectileA->EffectType = bIsBerserker
			? ESwordAuraEffectType::LeftBuff
			: ESwordAuraEffectType::LeftNormal;
		ProjectileA->Multicast_StartSwordSlashVFX();
	}

	// 두 번째 발사 (90도 회전 방향)
	// 두 번째 Projectile은 0.3초 뒤에 발사
	World->GetTimerManager().SetTimer(
		DelaySecondProjectileHandle,
		this,
		&UGS_AresAimingSkill::SpawnSecondProjectile,
		0.3f, // 지연 시간 (초)
		false
	);
}

void UGS_AresAimingSkill::SpawnSecondProjectile()
{
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	if (!OwnerCharacter || !AresCharacter || !AresCharacter->AresProjectileClass) 
	{
		return;
	}

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World)
	{
		return;
	}

	if (!AresCharacter || !World || !AresCharacter->AresProjectileClass) 
	{
		return;
	}

	// 발사 방향: 카메라 정면, 위로 뜨지 않게 Z 제거
	FVector LaunchDirection = OwnerCharacter->GetControlRotation().Vector();
	LaunchDirection.Z = 0.f;
	LaunchDirection = LaunchDirection.GetSafeNormal();

	// 생성 위치: 캐릭터 정면 약간 앞 + 위
	FVector SpawnLocation = OwnerCharacter->GetActorLocation()
		+ FVector(0, 0, 0)
		+ LaunchDirection * -300.f;

	// 두 번째 발사 (90도 회전 방향)
	FRotator SpawnRotationB = LaunchDirection.Rotation();
	SpawnRotationB.Roll = 45.f;
	FTransform SpawnTransform(SpawnRotationB, SpawnLocation);

	AGS_SwordAuraProjectile* ProjectileB = World->SpawnActorDeferred<AGS_SwordAuraProjectile>(
		AresCharacter->AresProjectileClass,
		SpawnTransform,
		OwnerCharacter,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	// 궁극기 활성화 상태에 따라 투사체 모양 설정 
	if (ProjectileB)
	{
		ProjectileB->EffectType = bIsBerserker
			? ESwordAuraEffectType::RightBuff
			: ESwordAuraEffectType::RightNormal;
		UGameplayStatics::FinishSpawningActor(ProjectileB, SpawnTransform);
		ProjectileB->Multicast_StartSwordSlashVFX();
	}
	
	// 스킬 종료
	DeactiveSkill();
}


	
	
