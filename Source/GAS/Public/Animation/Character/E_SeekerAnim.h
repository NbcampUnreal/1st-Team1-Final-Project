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
	Crawl,  // 빈사 상태 기어다니기
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	OrientToMovement,
	Strafe,
};