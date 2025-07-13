// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_EnvironmentAudioSystem.h"
#include "Sound/GS_AudioManager.h"
#include "AkGameplayStatics.h"

UGS_EnvironmentAudioSystem::UGS_EnvironmentAudioSystem()
{
}

// 기존 환경 오디오 함수들
void UGS_EnvironmentAudioSystem::SetEnvironmentZone(FName ZoneName)
{
    // 환경 존 설정 구현
    UE_LOG(LogTemp, Log, TEXT("환경 존 변경: %s"), *ZoneName.ToString());
    // 예: UAkGameplayStatics::SetState(ZoneStateGroup, ZoneName);
}

void UGS_EnvironmentAudioSystem::SetWeatherState(FName WeatherName)
{
    // 날씨 상태 설정 구현  
    UE_LOG(LogTemp, Log, TEXT("날씨 상태 변경: %s"), *WeatherName.ToString());
    // 예: UAkGameplayStatics::SetState(WeatherStateGroup, WeatherName);
    // 또는 관련 RTPC 설정: UAkGameplayStatics::SetRTPCValue(WeatherIntensityRTPC, Value, ...);
}

void UGS_EnvironmentAudioSystem::PlayAmbientSound()
{
    // 앰비언트 사운드 재생 구현
    UE_LOG(LogTemp, Log, TEXT("앰비언트 사운드 재생 - 구체적인 Event 필요"));
    // 예: if(SpecificAmbientEvent) UAkGameplayStatics::PostEvent(SpecificAmbientEvent, GetOuterAActor());
    // GetOuterAActor()는 UObject에서 AActor 컨텍스트를 얻는 한 방법입니다.
    // 더 나은 방법은 AActor* Context를 파라미터로 받는 것입니다.
}

void UGS_EnvironmentAudioSystem::PlayWeatherSound()
{
    // 날씨 사운드 재생 구현
    UE_LOG(LogTemp, Log, TEXT("날씨 사운드 재생 - 구체적인 Event 필요"));
    // 예: if(SpecificWeatherEvent) UAkGameplayStatics::PostEvent(SpecificWeatherEvent, GetOuterAActor());
}

void UGS_EnvironmentAudioSystem::LoadEnvironmentSoundBank()
{
    // 필요하다면 Wwise API로 SoundBank 동적 로드
    UE_LOG(LogTemp, Log, TEXT("환경 사운드뱅크 로드 - 구체적인 Bank 에셋 필요"));
    // 예: if(EnvironmentSoundBank) UAkGameplayStatics::LoadBank(EnvironmentSoundBank, EnvironmentSoundBank->GetFName(), DummyCallback, AK_DEFAULT_POOL_ID, false);
}