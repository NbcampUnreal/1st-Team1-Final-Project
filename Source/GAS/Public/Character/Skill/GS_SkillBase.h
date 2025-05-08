// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_SkillBase.generated.h"

class AGS_Character;
/**
 * 
 */
UCLASS()
class GAS_API UGS_SkillBase : public UObject
{
	GENERATED_BODY()

public:
	// 쿨타임 관리
	float GetCoolTime();

	// 스킬 초기화
	void InitSkill(AGS_Character* InOwner);

	// 스킬 작동
	virtual void ActiveSkill(); // 서버 권한에서만 호출
	virtual void ExecuteSkillEffect();
	virtual bool CanActive() const;

protected:
	// 쿨타임 관리
	FTimerHandle CooldownHandle;
	float Cooltime;
	bool bIsCoolingDown;
	void StartCoolDown();

	// 스킬 공격 애니메이션
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* SkillAnimMontage;

	// 스킬 소유자
	AGS_Character* OwnerCharacter;
};
