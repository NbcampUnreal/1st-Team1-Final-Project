#pragma once
#include "CoreMinimal.h"
#include "E_SeekerAnim.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Moving,
	Idle,
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk,
	Run,
	Sprint,
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	OrientToMovement,
	Strafe,
};