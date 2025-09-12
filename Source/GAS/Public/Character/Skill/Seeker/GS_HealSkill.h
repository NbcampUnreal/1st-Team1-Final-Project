// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_HealSkill.generated.h"

/**
 * 모든 시커가 사용할 수 있는 치유 스킬
 */
UCLASS()
class GAS_API UGS_HealSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()

public:
	UGS_HealSkill();

	virtual void ActiveSkill() override;
	virtual bool CanActive() const override; // 포션 개수와 체력 상태를 고려한 활성화 가능 여부

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Heal")
	float HealAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heal")
	int32 MaxHealCount;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealCount, BlueprintReadOnly, Category = "Heal")
	int32 CurrentHealCount;

	UPROPERTY(BlueprintReadOnly, Category = "Heal")
	bool bIsPotionDepletedOrHealthFull;

protected:
	// 초기화 함수
	void InitializeDamageBinding();

public:
	// 리플리케이션 함수
	UFUNCTION()
	void OnRep_CurrentHealCount();

	// 피해를 입었을 때 호출될 함수
	UFUNCTION()
	void OnOwnerDamaged(AActor* DamagedActor, float DamageAmount, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// 포션 개수 확인
	UFUNCTION(BlueprintCallable, Category = "Heal")
	int32 GetCurrentHealCount() const { return CurrentHealCount; }

	// 최대 포션 개수 확인
	UFUNCTION(BlueprintCallable, Category = "Heal")
	int32 GetMaxHealCount() const { return MaxHealCount; }

	// 포션 개수 설정
	UFUNCTION(BlueprintCallable, Category = "Heal")
	void SetCurrentHealCount(int32 NewCount);

	// 포션 사용 가능 여부 확인
	UFUNCTION(BlueprintCallable, Category = "Heal")
	bool CanUseHeal() const;
	
	// 체력이 가득 찼는지 확인
	UFUNCTION(BlueprintCallable, Category = "Heal")
	bool IsHealthFull() const;
	
	// 힐 스킬 사용 가능 여부 확인 (포션 + 체력 체크)
	UFUNCTION(BlueprintCallable, Category = "Heal")
	bool CanActivateHealSkill() const;

	// 포션 부족 시 쿨다운 효과 표시
	void ShowPotionDepletedEffect();

};
