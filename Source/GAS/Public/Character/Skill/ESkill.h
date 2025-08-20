#pragma once

#include "CoreMinimal.h"
#include "ESkill.generated.h"

UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Ready,
	Moving,
	Aiming,
	Ultimate,
	Rolling,
	Combo,
	End
};

USTRUCT(BlueprintType)
struct FControlValue
{
	GENERATED_BODY()
public:
	FControlValue()
	{
		bCanLookUp = true;
		bCanLookRight = true;
		bCanMoveForward = true;
		bCanMoveRight = true;
	}

	bool CanMove() const { return bCanMoveForward || bCanMoveRight; }
	bool CanLook() const { return bCanLookUp || bCanLookRight; }
	
	UPROPERTY(EditAnywhere)
	bool bCanLookUp;

	UPROPERTY(EditAnywhere)
	bool bCanLookRight;

	UPROPERTY(EditAnywhere)
	bool bCanMoveForward;

	UPROPERTY(EditAnywhere)
	bool bCanMoveRight;
};