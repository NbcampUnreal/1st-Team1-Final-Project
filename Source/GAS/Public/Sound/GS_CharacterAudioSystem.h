// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_CharacterAudioSystem.generated.h"

class UAkAudioEvent;

UCLASS(BlueprintType)
class GAS_API UGS_CharacterAudioSystem : public UObject
{
	GENERATED_BODY()
	
public:
	UGS_CharacterAudioSystem();

	// === 캐릭터 사운드 함수들 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|Character")
	void PlayFootstepSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Character")
	void PlayCharacterCombatSound(FName CombatEventName, AActor* Context);

protected:
	// === 캐릭터 사운드 이벤트들 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Character")
	UAkAudioEvent* TestEvent;
};
