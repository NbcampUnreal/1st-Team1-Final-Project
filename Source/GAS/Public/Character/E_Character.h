#pragma once

#include "CoreMinimal.h"
#include "E_Character.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	Ares,
	Chan,
	Merci,
	Reina,
	Drakhar,
	SmallClaw,
	NeedleFang,
	IronFang,
	ShadowFang,
	StoneClaw
};

UENUM(BlueprintType)
enum class EWeaponHandlingState : uint8
{
	Wielding, // 들고 있는 상태
	Sheathing, // 단순 소지 상태
	Aim, // 겨누는 상태
	UnArmd, // 소지 하지 않은 상태
	End,
};