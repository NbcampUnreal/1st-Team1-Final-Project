#pragma once

#include "CoreMinimal.h"
#include "GS_MarkerTypes.generated.h"

UENUM(BlueprintType)
enum class EMarkerType : uint8
{
	X           UMETA(DisplayName = "X"),
	ArrowUp     UMETA(DisplayName = "Arrow Up"),
	ArrowDown   UMETA(DisplayName = "Arrow Down"),
	ArrowLeft   UMETA(DisplayName = "Arrow Left"),
	ArrowRight  UMETA(DisplayName = "Arrow Right"),
	Check       UMETA(DisplayName = "Check")
};

