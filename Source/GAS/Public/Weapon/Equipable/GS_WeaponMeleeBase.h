// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_WeaponEquipable.h"
#include "Components/BoxComponent.h"
#include "GS_WeaponMeleeBase.generated.h"

/*
 * 근접 무기의 기본 클래스
 * 공통된 히트박스 관리와 데미지 처리를 담당
 */

UCLASS(Abstract)
class GAS_API AGS_WeaponMeleeBase : public AGS_WeaponEquipable
{
	GENERATED_BODY()

public:
	AGS_WeaponMeleeBase();

	// 히트 콜리전 활성화/비활성화
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void EnableHit();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void DisableHit();

	// 서버에서 콜리전 상태 설정
	UFUNCTION(Server, Unreliable)
	void Server_SetHitCollision(bool bEnable);

	// 공격 시작 시 호출 (히트 액터 초기화)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartAttack();

	// 공격 종료 시 호출 (히트 콜리전 비활성화)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EndAttack();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UBoxComponent* HitBox;

	// 실제 히트 처리 오버라이드
	virtual void ProcessHit(AActor* HitActor) override;

	// 유효한 타겟인지 확인 (캐릭터만 히트 가능하도록)
	virtual bool IsValidTarget(AActor* Target) const override;
	
	// 히트 사운드 재생
	virtual void PlayHitSound(UAkAudioEvent* SoundEvent);

private:
	// 히트박스 콜리전 설정
	void SetHitCollision(bool bEnable);
};