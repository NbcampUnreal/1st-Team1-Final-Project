#pragma once

#include "CoreMinimal.h"
#include "GS_SkillComp.h"
#include "NiagaraSystem.h"
#include "GS_SkillBase.generated.h"

class AGS_Character;
class UGS_SkillComp;

UCLASS()
class GAS_API UGS_SkillBase : public UObject
{
	GENERATED_BODY()

public:
	ESkillSlot CurrentSkillType;
	
	float Cooltime = 30.0f;
	float Damage = 0.f;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<UAnimMontage*> SkillAnimMontages;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* SkillImage;

	// VFX 관련 속성들 (데이터 테이블에서 설정됨)
	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillCastVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillRangeVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillImpactVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillEndVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	FVector SkillVFXScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	float SkillVFXDuration = 3.0f;

	// VFX 위치 오프셋
	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	FVector CastVFXOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	FVector RangeVFXOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	FVector ImpactVFXOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	FVector EndVFXOffset = FVector::ZeroVector;

	UTexture2D* GetSkillImage();
	
	// 쿨타임 관리
	float GetCoolTime();

	// 스킬 초기화
	void InitSkill(AGS_Character* InOwner, UGS_SkillComp* InOwningComp);

	// 로컬 VFX 재생 함수들
	void PlayCastVFX(FVector Location, FRotator Rotation);
	void PlayRangeVFX(FVector Location, float Radius);
	void PlayImpactVFX(FVector Location); // 월드 위치에 생성
	void PlayImpactVFXOnTarget(AActor* Target); // 타겟에 부착
	void PlayEndVFX(FVector Location, FRotator Rotation);

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
	FTimerHandle LogTimerHandle;

	float LeftCoolTime;
	
	bool bIsCoolingDown;
	void StartCoolDown();
	void LogRemainingTime();
	void SetCoolTime(float InCoolTime);
	
	// 스킬 소유자
	AGS_Character* OwnerCharacter;
	UGS_SkillComp* OwningComp;
};
