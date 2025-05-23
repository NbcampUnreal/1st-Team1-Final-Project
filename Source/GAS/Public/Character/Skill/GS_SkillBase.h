#pragma once

#include "CoreMinimal.h"
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
	virtual void DeactiveSkill();
	virtual void ExecuteSkillEffect();
	virtual void OnSkillCommand();
	virtual bool CanActive() const;
	virtual bool IsActive() const;

protected:
	bool bIsActive = false;
	// 쿨타임 관리
	FTimerHandle CooldownHandle;
	float Cooltime=30.0f;
	bool bIsCoolingDown;
	void StartCoolDown();

	// 스킬 공격 애니메이션
	UPROPERTY(EditDefaultsOnly)
	TArray<UAnimMontage*> SkillAnimMontages;

	// 스킬 소유자
	AGS_Character* OwnerCharacter;
};
