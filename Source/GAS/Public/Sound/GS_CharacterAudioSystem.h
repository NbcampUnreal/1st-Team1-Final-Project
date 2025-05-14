// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_CharacterAudioSystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAS_API UGS_CharacterAudioSystem : public UObject
{
	GENERATED_BODY()
	
public:
	// 소리 재생 인터페이스
	UFUNCTION(BlueprintCallable, Category = "Audio|Character")
	void PlayFootstepSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Character")
	void PlayCharacterCombatSound(FName CombatEventName, AActor* Context);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Character")
	class UAkAudioEvent* TestEvent;


};
