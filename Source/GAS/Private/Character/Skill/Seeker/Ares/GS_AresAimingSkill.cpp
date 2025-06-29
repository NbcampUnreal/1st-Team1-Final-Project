// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresAimingSkill.h"
#include "Character/Skill/Seeker/Ares/GS_AresUltimateSkill.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Weapon/Projectile/Seeker/GS_SwordAuraProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Character/GS_Character.h"
#include "Character/GS_TpsController.h"

UGS_AresAimingSkill::UGS_AresAimingSkill()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_AresAimingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);

		// 스킬 시작 사운드 재생
		const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
		if (SkillInfo && SkillInfo->SkillStartSound)
		{
			OwnerPlayer->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
		}
	}

	AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->SetLookControlValue(false, false);

	ExecuteSkillEffect();
}

void UGS_AresAimingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();
}

void UGS_AresAimingSkill::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();

	UE_LOG(LogTemp, Warning, TEXT("AresAimingSkill ExecuteSkillEffect"));

	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	if (!OwnerCharacter || !AresCharacter || !AresCharacter->AresProjectileClass) return;

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World) return;

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


	UGS_SkillComp* SkillComp = OwnerCharacter->GetSkillComp();


	if (SkillComp)
	{
		if (UGS_AresUltimateSkill* UltimateSkill = Cast<UGS_AresUltimateSkill>(SkillComp->GetSkillFromSkillMap(ESkillSlot::Ultimate)))
		{
			bIsBerserker = UltimateSkill->IsActive();
		}
		
	}

	if (ProjectileA)
	{
		ProjectileA->EffectType = bIsBerserker
			? ESwordAuraEffectType::LeftBuff
			: ESwordAuraEffectType::LeftNormal;
		ProjectileA->Multicast_StartSwordSlashVFX();
	}

	// 두 번째 발사 (90도 회전 방향)
	// 두 번째 Projectile은 0.1초 뒤에 발사
	World->GetTimerManager().SetTimer(
		DelaySecondProjectileHandle,
		this,
		&UGS_AresAimingSkill::SpawnSecondProjectile,
		0.3f, // 지연 시간 (초)
		false
	);
}

void UGS_AresAimingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	if (AresCharacter->GetSkillComp())
	{
		AresCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, false);
	}
}

bool UGS_AresAimingSkill::IsActive() const
{
	Super::IsActive();
	return bIsActive;
}

void UGS_AresAimingSkill::SpawnSecondProjectile()
{
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	if (!OwnerCharacter || !AresCharacter || !AresCharacter->AresProjectileClass) return;

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World) return;

	if (!AresCharacter || !World || !AresCharacter->AresProjectileClass) return;

	// 발사 방향: 카메라 정면, 위로 뜨지 않게 Z 제거
	FVector LaunchDirection = OwnerCharacter->GetControlRotation().Vector();
	LaunchDirection.Z = 0.f;
	LaunchDirection = LaunchDirection.GetSafeNormal();

	// 생성 위치: 캐릭터 정면 약간 앞 + 위
	FVector SpawnLocation = OwnerCharacter->GetActorLocation()
		+ FVector(0, 0, 0)
		+ LaunchDirection * -300.f;

	// 첫 번째 발사 (기본 방향)
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

	UE_LOG(LogTemp, Warning, TEXT("CharacterLocation: %s | SpawnLocation: %s"),
		*OwnerCharacter->GetActorLocation().ToString(),
		*SpawnLocation.ToString());

	if (ProjectileB)
	{
		ProjectileB->EffectType = bIsBerserker
			? ESwordAuraEffectType::RightBuff
			: ESwordAuraEffectType::RightNormal;
		UGameplayStatics::FinishSpawningActor(ProjectileB, SpawnTransform);
		ProjectileB->Multicast_StartSwordSlashVFX();
	}
	
	// 스킬 종료 처리
	AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->SetLookControlValue(true, true);
	bIsActive = false;
}
	
	
