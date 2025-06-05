#pragma once

#include "CoreMinimal.h"
#include "EDebuffType.generated.h"

UENUM(BlueprintType)
enum class EDebuffType : uint8
{
	None	UMETA(DisplayName = "None"),
	Stun	UMETA(DisplayName = "Stun"),
	Aggro	UMETA(DisplayName = "Aggro"),
	Obscure	UMETA(DisplayName = "Obscure"),
	Confuse	UMETA(DisplayName = "Confuse"),
	Mute	UMETA(DisplayName = "Mute"),
	Slow	UMETA(DisplayName = "Slow")
};
