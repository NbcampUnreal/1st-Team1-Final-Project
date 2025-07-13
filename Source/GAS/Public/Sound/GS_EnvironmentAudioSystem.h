// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_EnvironmentAudioSystem.generated.h"

/**
 * 환경 오디오 시스템
 * 앰비언트 사운드, 날씨 효과음 등을 관리합니다.
 */
UCLASS(BlueprintType)
class GAS_API UGS_EnvironmentAudioSystem : public UObject
{
	GENERATED_BODY()

public:
	UGS_EnvironmentAudioSystem();

	// 환경 오디오 함수들
	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void SetEnvironmentZone(FName ZoneName);

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void SetWeatherState(FName WeatherName);

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void PlayAmbientSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void PlayWeatherSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void LoadEnvironmentSoundBank();

protected:
	// 환경 사운드 관련 에셋들 (예시: 필요에 따라 추가)
	// UPROPERTY(EditDefaultsOnly, Category = "Audio|Environment Sounds")
	// UAkAudioEvent* RainSoundEvent;

	// UPROPERTY(EditDefaultsOnly, Category = "Audio|Environment Sounds")
	// UAkAudioEvent* WindSoundEvent;

private:
	// 환경 효과음 관련 내부 변수들 (필요시 추가)
};
