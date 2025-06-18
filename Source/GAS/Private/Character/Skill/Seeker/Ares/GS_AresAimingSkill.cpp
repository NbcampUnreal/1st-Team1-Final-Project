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

	UE_LOG(LogTemp, Warning, TEXT("AresAimingSkill ActiveSkill"));
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	if (!OwnerCharacter || !AresCharacter->AresProjectileClass) return;

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World) return;

	// 중앙에 생성
	FVector SpawnLocation = OwnerCharacter->GetActorLocation() + FVector(0, 0, 50); // 캐릭터 중심 높이 조정

	FRotator SpawnRotation = OwnerCharacter->GetControlRotation(); // 카메라 방향

	// 스폰
	CachedProjectile = World->SpawnActor<AGS_SwordAuraProjectile>(
		AresCharacter->AresProjectileClass,
		SpawnLocation,
		SpawnRotation
	);

	bIsActive = CachedProjectile != nullptr;

	
	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
	}
}

void UGS_AresAimingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();
}

void UGS_AresAimingSkill::OnSkillCommand()
{
	Super::OnSkillCommand();

	UE_LOG(LogTemp, Warning, TEXT("AresAimingSkill OnSkillCommand"));
	if (!CachedProjectile || !OwnerCharacter) return;

	// 카메라 방향으로 발사
	const FVector LaunchDirection = OwnerCharacter->GetControlRotation().Vector();
	const float LaunchSpeed = 1200.f;

	// 컴포넌트가 움직이게
	if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(CachedProjectile->GetRootComponent()))
	{
		RootComp->SetSimulatePhysics(true);
		RootComp->SetPhysicsLinearVelocity(LaunchDirection * LaunchSpeed);
	}
	else
	{
		// 또는 직접 위치 이동
		CachedProjectile->SetActorLocation(
			CachedProjectile->GetActorLocation() + LaunchDirection * 100.f
		);
	}

	// 더 이상 저장 안 함
	CachedProjectile = nullptr;
	bIsActive = false;
	
}

void UGS_AresAimingSkill::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();
}

bool UGS_AresAimingSkill::IsActive() const
{

	Super::IsActive();
	return bIsActive;
}
	
	
