// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresAimingSkill.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Weapon/Projectile/Seeker/GS_SwordAuraProjectile.h"
#include "Character/GS_Character.h"

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
	}

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
		+ FVector(0, 0, 50)
		+ LaunchDirection * 100.f;

	FRotator SpawnRotation = LaunchDirection.Rotation();

	// Projectile 생성
	AGS_SwordAuraProjectile* SpawnedProjectile = World->SpawnActor<AGS_SwordAuraProjectile>(
		AresCharacter->AresProjectileClass,
		SpawnLocation,
		SpawnRotation
	);

	// 스킬 종료 처리
	bIsActive = false;
}

bool UGS_AresAimingSkill::IsActive() const
{
	Super::IsActive();
	return bIsActive;
}
	
	
