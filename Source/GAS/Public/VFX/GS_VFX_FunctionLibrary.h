// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GS_VFX_FunctionLibrary.generated.h"

class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class GAS_API UGS_VFX_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "VFX")
	static void PlayBloodEffect(UObject* WorldContextObject, UNiagaraSystem* BloodEffectSystem, const FVector& Location, const FRotator& Rotation, float Scale = 1.0f);
	
};
