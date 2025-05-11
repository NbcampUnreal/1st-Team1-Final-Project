// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_EnvironmentAudioSystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAS_API UGS_EnvironmentAudioSystem : public UObject
{
	GENERATED_BODY()

public:
	// 환경 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void SetEnvironmentZone(FName ZoneName);

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void SetWeatherState(FName WeatherName);

	// 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void PlayAmbientSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void PlayWeatherSound();

	// 사운드뱅크 로드
	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void LoadEnvironmentSoundBank();
	
};
