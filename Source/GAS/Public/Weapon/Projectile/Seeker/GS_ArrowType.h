// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_ArrowType.generated.h"

UENUM(BlueprintType)
enum class EArrowType : uint8
{
	Normal	UMETA(DisplayName="Normal"),
	Axe		UMETA(DisplayName = "Axe"),
	Child	UMETA(DisplayName = "Child")
}; 