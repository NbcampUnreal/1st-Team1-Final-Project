#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
//#include "Character/Skill/GS_SkillBase.h"
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
