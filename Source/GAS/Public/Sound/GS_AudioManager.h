// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "GS_AudioManager.generated.h"

/**
 * AudioManager는 게임의 오디오 시스템을 관리하는 서브시스템입니다.
 *
 * 이 클래스는 게임 인스턴스의 서브시스템으로, 다양한 오디오 시스템을 초기화하고 관리합니다.
 * 또한 Wwise 이벤트를 호출하는 기능도 제공합니다.
 */

class UGS_CharacterAudioSystem;
class UGS_UIAudioSystem;
class UGS_EnvironmentAudioSystem;

UCLASS()
class GAS_API UGS_AudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// GameInstanceSubsystem 초기화/종료 오버라이드
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Sub-system 접근
	UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
	class UGS_CharacterAudioSystem* GetCharacterAudio() const { return CharacterAudio; }

	UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
	class UGS_UIAudioSystem* GetUIAudio() const { return UIAudio; }

	UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
	class UGS_EnvironmentAudioSystem* GetEnvironmentAudio() const { return EnvironmentAudio; }

	// Wwise 이벤트 호출 래퍼
	UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
	void PlayEvent(UAkAudioEvent* Event, AActor* Context);

private:
	// 생성·파괴 주기는 GameInstance와 동기화
	UPROPERTY()
	UGS_CharacterAudioSystem* CharacterAudio;

	UPROPERTY()
	UGS_UIAudioSystem* UIAudio;

	UPROPERTY()
	UGS_EnvironmentAudioSystem* EnvironmentAudio;

};
