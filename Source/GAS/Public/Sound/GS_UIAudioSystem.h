// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_UIAudioSystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAS_API UGS_UIAudioSystem : public UObject
{
	GENERATED_BODY()
	
public:
	// UI 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Audio|UI")
	void PlayButtonClick(APlayerController* Controller);

	UFUNCTION(BlueprintCallable, Category = "Audio|UI")
	void PlayMenuOpen(APlayerController* Controller);
};
