#pragma once

#include "RTSCommand.generated.h"

// 명령 모드 
UENUM(BlueprintType)
enum class ERTSCommand : uint8
{
	None    UMETA(DisplayName = "None"),
	Move    UMETA(DisplayName = "Move"),
	Stop	UMETA(DisplayName = "Stop"),
	Hold	UMETA(DisplayName = "Hold"),
	Attack  UMETA(DisplayName = "Attack"), 
	Skill   UMETA(DisplayName = "Skill") 
};