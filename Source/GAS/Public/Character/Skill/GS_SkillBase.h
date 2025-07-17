#pragma once

#include "CoreMinimal.h"
#include "GS_SkillComp.h"
#include "NiagaraSystem.h"
#include "GS_SkillBase.generated.h"

class AGS_Player;
class UGS_SkillComp;

UCLASS()
class GAS_API UGS_SkillBase : public UObject
{
	GENERATED_BODY()

public:
	ESkillSlot CurrentSkillType;
	
	float Cooltime;
	float Damage;
	
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
	void InitSkill(AGS_Player* InOwner, UGS_SkillComp* InOwningComp, ESkillSlot InSlot);

	// 로컬 VFX 재생 함수들
	void PlayCastVFX(FVector Location, FRotator Rotation);
	void PlayRangeVFX(FVector Location, float Radius);
	void PlayImpactVFX(FVector Location); // 월드 위치에 생성
	void PlayImpactVFXOnTarget(AActor* Target); // 타겟에 부착
	void PlayEndVFX(FVector Location, FRotator Rotation);

	// 스킬 작동
	virtual void ActiveSkill(); // 스킬 시작(서버 권한에서만 호출)
	virtual void OnSkillCanceledByDebuff(); // 스킬 중도 종료(Mute 디버프)
	virtual void OnSkillAnimationEnd(); // 애니메이션 종료 시 호출
	virtual void ExecuteSkillEffect();
	virtual void DeactiveSkill(); // 스킬 종료
	virtual void OnSkillCommand(); // 보조 스킬 발동
	virtual bool CanActive() const; // 사용 가능한지 반환
	virtual bool GetIsActive() const; // 사용중인지 반환
	virtual void InterruptSkill(); // 다른 스킬 사용으로 인한 스킬 중단
	virtual void SetIsActive(bool bInIsActive);

	// 쿨타임 
	void SetCoolingDown(bool bInCoolingDown) { bIsCoolingDown = bInCoolingDown; }
	
protected:
	bool bIsActive = false;
	bool bIsCoolingDown;

	// 스킬 소유자
	AGS_Player* OwnerCharacter;
	UGS_SkillComp* OwningComp;
	
	void StartCoolDown();
	
	// 데이터 테이블에서 현재 스킬 정보 가져오기
	const FSkillInfo* GetCurrentSkillInfo() const;
};
