// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GS_Weapon.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "GS_WeaponEquipable.generated.h"

class AGS_Character;
//class AGS_Seeker;

UCLASS()
class GAS_API AGS_WeaponEquipable : public AGS_Weapon
{
	GENERATED_BODY()
public:
	AGS_WeaponEquipable();

	virtual void SetOwningCharacter(AGS_Character* Character);

	// 히트 사운드 재생 함수
	UFUNCTION(BlueprintCallable, Category = "Sound")
	virtual void PlayHitSound();

protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Owner")
	AGS_Character* OwnerCharacter;

	// ======
	// 사운드
	// ======
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* HitSoundEvent;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAkComponent* AkComponent;
	
	// 콜리전 체크 함수 - 기본 필터링만 수행
	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 실제 히트 처리를 위한 가상 함수
	UFUNCTION()
	virtual void ProcessHit(AActor* HitActor);

	// 유효한 타겟인지 확인하는 함수
	UFUNCTION()
	virtual bool IsValidTarget(AActor* Target) const;
};
