// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponEquipable.h"

AGS_WeaponEquipable::AGS_WeaponEquipable()
{
	OwnerChar = nullptr;
	bReplicates = true;
}

void AGS_WeaponEquipable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Set Server option
	SetReplicateMovement(true); // Replicate Actor Rotation & Transition
}

// 헬퍼 함수 구현
bool AGS_WeaponEquipable::IsValidForLevelTransition() const
{
	return IsValid(this) && GetWorld() && !GetWorld()->bIsTearingDown;
}

bool AGS_WeaponEquipable::IsOwnerCharValid() const
{
	return OwnerChar != nullptr;
}

void AGS_WeaponEquipable::ClearHitActors()
{
	HitActors.Empty();
}

void AGS_WeaponEquipable::ClearSafetyTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SafetyTimerHandle);
	}
}

FHitResult AGS_WeaponEquipable::CreateCorrectHitResult(const FHitResult& OriginalResult, bool bFromSweep) const
{
	FHitResult CorrectHitResult = OriginalResult;
	if (!bFromSweep)
	{
		CorrectHitResult.ImpactPoint = GetActorLocation();
		CorrectHitResult.Location = GetActorLocation();
		CorrectHitResult.ImpactNormal = FVector::UpVector;
		CorrectHitResult.Normal = FVector::UpVector;
	}
	return CorrectHitResult;
}
