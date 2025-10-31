// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GS_ManualDataInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGS_ManualDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAS_API IGS_ManualDataInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Manual")
	FName GetManualRowName() const;
};
