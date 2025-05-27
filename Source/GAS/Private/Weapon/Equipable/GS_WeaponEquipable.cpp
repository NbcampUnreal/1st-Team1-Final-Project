// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponEquipable.h"
#include "Character/GS_Character.h"

AGS_WeaponEquipable::AGS_WeaponEquipable()
{
	OwnerCharacter = nullptr;

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	AkComponent->SetupAttachment(RootComponent);
}

void AGS_WeaponEquipable::SetOwningCharacter(AGS_Character* Character)
{
	OwnerCharacter = Character;
}

void AGS_WeaponEquipable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Set Server option
	SetReplicateMovement(true); // Replicate Actor Rotation & Transition
}

void AGS_WeaponEquipable::PlayHitSound()
{
	if (HitSoundEvent && AkComponent)
	{
		AkComponent->PostAkEvent(HitSoundEvent);
	}
}

bool AGS_WeaponEquipable::IsValidTarget(AActor* Target) const
{
	// 기본 필터링: 자기 자신, 소유자는 대상 제외
	if (!Target || Target == this || Target == OwnerCharacter)
	{
		return false;
	}
	
	return true;
}

void AGS_WeaponEquipable::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Weapon Hit Detected - Actor: %s"), OtherActor ? *OtherActor->GetName() : TEXT("None"));
	
	// 유효한 타겟인지 확인
	if (!IsValidTarget(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Target"));
		return;
	}

	// 히트 처리
	ProcessHit(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("Hit Processed Successfully"));
}

void AGS_WeaponEquipable::ProcessHit(AActor* HitActor)
{
	// TODO - 자식 클래스에서 구체적인 히트 처리를 구현 (데미지 처리, 이펙트 재생 등)
}