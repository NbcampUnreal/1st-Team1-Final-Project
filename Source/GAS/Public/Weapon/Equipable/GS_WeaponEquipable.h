// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GS_Weapon.h"
#include "GS_WeaponEquipable.generated.h"

class AGS_Seeker;

UCLASS()
class GAS_API AGS_WeaponEquipable : public AGS_Weapon
{
	GENERATED_BODY()
public:
	AGS_WeaponEquipable();

protected:
	virtual void PostInitializeComponents() override;

	// 헬퍼 함수들
	bool IsValidForLevelTransition() const;
	bool IsOwnerCharValid() const;
	void ClearHitActors();
	void ClearSafetyTimer();
	virtual FHitResult CreateCorrectHitResult(const FHitResult& OriginalResult, bool bFromSweep) const;

protected:
	// 공통 멤버 변수들
	UPROPERTY()
	class AGS_Character* OwnerChar;

	UPROPERTY()
	TSet<AActor*> HitActors;

	FTimerHandle SafetyTimerHandle;
};
