// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AkAudioEvent.h"
#include "GS_CharacterAudioSystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAS_API UGS_CharacterAudioSystem : public UObject
{
	GENERATED_BODY()
	
public:
	UGS_CharacterAudioSystem();

	// 전투 음악 관련 함수
	void PlayCombatMusic(AActor* Context);
	void StopCombatMusic(AActor* Context);

	// 전투 음악 이벤트
	UPROPERTY(EditDefaultsOnly, Category = "Combat Audio")
	UAkAudioEvent* CombatMusicEvent;

	// 전투 음악 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Audio|Character")
	void PlayFootstepSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Character")
	void PlayCharacterCombatSound(FName CombatEventName, AActor* Context);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Character")
	class UAkAudioEvent* TestEvent;

};
