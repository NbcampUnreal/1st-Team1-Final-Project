#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
//#include "Character/Skill/GS_SkillBase.h"
#include "ESkill.h"
#include "NiagaraSystem.h"
#include "GS_SkillSet.generated.h"

class UGS_SkillBase;
class UImage;
class UAkAudioEvent;

USTRUCT(BlueprintType)
struct FSkillAllow
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Skill")
	ESkillSlot Slot;

	UPROPERTY(EditAnywhere, Category = "Skill")
	bool bAllow = false;
};


USTRUCT(BlueprintType)
struct GAS_API FSkillInfo
{
	GENERATED_BODY()

	FSkillInfo()
	{
		AllowSkillList.Empty();
		for (int32 i =0; i < static_cast<int32>(ESkillSlot::End); i++)
		{
			FSkillAllow AllowSkill;
			AllowSkill.Slot = static_cast<ESkillSlot>(i);
			AllowSkillList.Add(AllowSkill);
		}
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_SkillBase> SkillClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooltime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimMontage*> Montages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkillAllow> AllowSkillList; // SJE

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FControlValue AllowControlValue; // SJE
	// 움직임이나 시선에 관한 flag 도 여기에서 관리해야하는거 아닌가 결국?

	
	// 스킬 사운드 이벤트들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillStartSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillEndSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillLoopSound; // 스킬 가동 중 루프 사운드 (궁극기용)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillLoopStopSound; // 스킬 루프 사운드 정지 이벤트 (궁극기용)

	// 충돌별 특수 사운드 (주로 궁극기 스킬용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Collision")
	UAkAudioEvent* WallCollisionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Collision")
	UAkAudioEvent* MonsterCollisionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Collision")
	UAkAudioEvent* GuardianCollisionSound;

	// 스킬 VFX 이벤트들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillCastVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillRangeVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillImpactVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> SkillEndVFX;

	// VFX 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector SkillVFXScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float SkillVFXDuration = 3.0f;

	// VFX 위치 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Offset")
	FVector CastVFXOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Offset")
	FVector RangeVFXOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Offset")
	FVector ImpactVFXOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Offset")
	FVector EndVFXOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct GAS_API FGS_SkillSet : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterType CharacterType = ECharacterType::Chan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo ReadySkill;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo AimingSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo MovingSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo UltimateSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo RollingSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo ComboSkill; // SJE

	FGS_SkillSet()
		/*: CharacterType(ECharacterType::Chan)
		, ReadySkill(nullptr)
		, AimingSkill(nullptr)
		, MovingSkill(nullptr)
		, UltimateSkill(nullptr)
		, RollingSkill(nullptr)*/
	{
	}
};
