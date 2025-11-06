// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GS_Weapon.h"
#include "Weapon/Component/GS_WeaponVFXComponent.h"
#include "Character/GS_Character.h"
#include "GS_WeaponEquipable.generated.h"

class AGS_Seeker;

UCLASS()
class GAS_API AGS_WeaponEquipable : public AGS_Weapon
{
	GENERATED_BODY()
public:
	AGS_WeaponEquipable();
	
	AGS_Character* GetOwnerChar() { return OwnerChar; };

protected:
	virtual void PostInitializeComponents() override;

	// 헬퍼 함수들
	bool IsValidForLevelTransition() const;
	bool IsOwnerCharValid() const;
	void ClearHitActors();
	virtual void ClearSafetyTimer();
	virtual FHitResult CreateCorrectHitResult(const FHitResult& OriginalResult, bool bFromSweep) const;

protected:
	// 공통 멤버 변수들
	UPROPERTY()
	class AGS_Character* OwnerChar;

	UPROPERTY()
	TSet<AActor*> HitActors;

	FTimerHandle SafetyTimerHandle;

	// 무기 VFX 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGS_WeaponVFXComponent* WeaponVFXComponent;

public:
	// 무기 VFX 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void TriggerHitAuraOnHit(class AGS_Character* HitTarget);

	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	ESeekerAuraType GetSeekerAuraType(class AGS_Character* SeekerChar) const;

protected:
	// 아우라 트리거 조건 확인
	virtual bool ShouldTriggerAuraOnHit(class AGS_Character* HitTarget) const;
};
