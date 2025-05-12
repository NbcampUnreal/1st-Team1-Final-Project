// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_AudioManager.h"
#include "Sound/GS_CharacterAudioSystem.h"
#include "Sound/GS_UIAudioSystem.h"
#include "Sound/GS_EnvironmentAudioSystem.h"

void UGS_AudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 오디오 시스템 인스턴스 생성
	CharacterAudio = NewObject<UGS_CharacterAudioSystem>(this);
	UIAudio = NewObject<UGS_UIAudioSystem>(this);
	EnvironmentAudio = NewObject<UGS_EnvironmentAudioSystem>(this);
}

void UGS_AudioManager::Deinitialize()
{
	// 메모리 해제 처리
	CharacterAudio = nullptr;
	UIAudio = nullptr;
	EnvironmentAudio = nullptr;

	Super::Deinitialize();
}

// Wwise 이벤트 호출 함수
void UGS_AudioManager::PlayEvent(UAkAudioEvent* Event, AActor* Context)
{
	if (!Event || !Context) // Wwise 이벤트나 컨텍스트가 유효하지 않은 경우
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayEvent 호출 실패: Invalid Event or Context."));
		return;
	}
	FOnAkPostEventCallback DummyCallback;
	// Wwise의 이벤트 포스트
	UAkGameplayStatics::PostEvent(Event, Context, /*CallbackMask=*/0, /*PostEventCallback=*/DummyCallback);
}
