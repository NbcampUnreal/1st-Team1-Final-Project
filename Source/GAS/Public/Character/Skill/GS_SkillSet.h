#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
//#include "Character/Skill/GS_SkillBase.h"
#include "NiagaraSystem.h"
#include "GS_SkillSet.generated.h"

class UGS_SkillBase;
class UImage;
class UAkAudioEvent;

USTRUCT(BlueprintType)
struct GAS_API FSkillInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_SkillBase> SkillClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooltime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimMontage*> Montages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Image;

	// 스킬 사운드 이벤트들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillStartSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillCastSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent* SkillEndSound;

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
	ECharacterType CharacterType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo ReadySkill;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo AimingSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo MovingSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkillInfo UltimateSkill;

	FGS_SkillSet()
		: CharacterType(ECharacterType::Chan)
		, ReadySkill(nullptr)
		, AimingSkill(nullptr)
		, MovingSkill(nullptr)
		, UltimateSkill(nullptr)
	{
	}
};
