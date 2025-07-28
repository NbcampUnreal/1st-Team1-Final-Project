// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "AkRtpc.h"
#include "Engine/TimerHandle.h"
#include "GS_AudioManager.generated.h"

/**
 * AudioManager는 게임의 오디오 시스템을 관리하는 서브시스템입니다.
 *
 * 이 클래스는 게임 인스턴스의 서브시스템으로, 다양한 오디오 시스템을 초기화하고 관리합니다.
 * 또한 Wwise 이벤트를 호출하는 기능도 제공합니다.
 */

class UGS_UIAudioSystem;
class UGS_EnvironmentAudioSystem;

UCLASS()
class GAS_API UGS_AudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 생성자
	UGS_AudioManager();

	// GameInstanceSubsystem 초기화/종료 오버라이드
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Sub-system 접근
	UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
	class UGS_UIAudioSystem* GetUIAudio() const { return UIAudio; }

	UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
	class UGS_EnvironmentAudioSystem* GetEnvironmentAudio() const { return EnvironmentAudio; }

	// Wwise 이벤트 호출 래퍼
	UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
	void PlayEvent(UAkAudioEvent* Event, AActor* Context);

	// === 맵 BGM 관리 함수들 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (DisplayName = "맵 BGM 시작"))
	void StartMapBGM(AActor* Context);

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (DisplayName = "맵 BGM 정지"))
	void StopMapBGM(AActor* Context = nullptr);

	// === 맵 BGM 페이드 전환 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (DisplayName = "맵 BGM 페이드아웃 후 정지"))
	void FadeOutAndStopMapBGM(AActor* Context, float FadeTime = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (DisplayName = "맵 BGM 페이드인 후 재생"))
	void FadeInAndStartMapBGM(AActor* Context, float FadeTime = 2.0f);

	// === 통합 전투 시퀀스 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|Combat", meta = (DisplayName = "전투 시퀀스 시작", ToolTip = "맵 BGM을 페이드아웃/정지하고 전투 BGM을 시작합니다."))
	void StartCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStartEvent, UAkAudioEvent* CombatMusicStopEvent, float FadeTime = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Combat", meta = (DisplayName = "전투 시퀀스 종료", ToolTip = "전투 BGM을 중지하고 맵 BGM을 복원합니다."))
	void EndCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStopEvent = nullptr, float FadeTime = 3.0f);

	// === 멀티플레이어 지원 함수들 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|Multiplayer", meta = (DisplayName = "모든 클라이언트 맵 BGM 시작"))
	void StartMapBGMForAllClients();
	
	// 현재 재생 중인 전투 BGM Stop Event 가져오기
	UAkAudioEvent* GetCurrentCombatMusicStopEvent() const { return CurrentCombatMusicStopEvent; }

protected:
	// === 맵 BGM 관련 에셋들 ===
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Map BGM", meta = (DisplayName = "맵 BGM 이벤트"))
	UAkAudioEvent* MapBGMEvent;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Map BGM", meta = (DisplayName = "맵 BGM 정지 이벤트"))
	UAkAudioEvent* MapBGMStopEvent;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Map BGM", meta = (DisplayName = "맵 BGM 볼륨 RTPC"))
	UAkRtpc* MapBGMVolumeRTPC;

private:
	// 생성된·파괴 주기는 GameInstance와 동기화
	UPROPERTY()
	UGS_UIAudioSystem* UIAudio;

	UPROPERTY()
	UGS_EnvironmentAudioSystem* EnvironmentAudio;

	// 맵 BGM 상태 관리
	bool bIsMapBGMPlaying;

	// 전투 BGM 관리
	UPROPERTY()
	UAkAudioEvent* CurrentCombatMusicStartEvent;

	UPROPERTY()
	UAkAudioEvent* CurrentCombatMusicStopEvent;

	// 기본 전투 BGM 정지 이벤트 (EV_CombatStop)
	UPROPERTY()
	UAkAudioEvent* DefaultCombatStopEvent;

	// 맵 BGM 페이드인/아웃 타이머 핸들
	FTimerHandle MapBGMFadeInTimerHandle;
	FTimerHandle MapBGMFadeOutTimerHandle;

	// RTPC 헬퍼 함수
	void SetRTPCValue(UAkRtpc* RTPC, float Value, AActor* Context, float InterpolationTime = 0.0f);

	/**
	* @brief 오디오 에셋의 유효성을 검사합니다.
	* @return 모든 필수 에셋이 로드되었으면 true, 그렇지 않으면 false
	*/
	bool ValidateAudioAssets();

	/**
	* @brief 현재 환경에서 오디오 처리가 허용되는지 확인합니다.
	* @return 전용 서버가 아닌 경우 true, 전용 서버인 경우 false
	*/
	bool IsAudioProcessingAllowed() const;

	/**
	* @brief 현재 게임 모드(TPS/RTS)에 따라 BGM을 재생하거나 정지할 대상 액터를 결정합니다.
	* @param Context 컨텍스트로 제공된 액터 (옵셔널)
	* @return 결정된 타겟 액터. RTS 모드이거나 적절한 Pawn이 없는 경우 nullptr을 반환할 수 있습니다.
	*/
	AActor* GetTargetActorForPlayback(AActor* Context = nullptr);
};
