// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AkGameplayStatics.h"
#include "Engine/TimerHandle.h"
#include "GS_AudioManager.generated.h"

/**
 * AudioManager는 게임의 오디오 시스템을 관리하는 서브시스템입니다.
 *
 * 이 클래스는 게임 인스턴스의 서브시스템으로, 다양한 오디오 시스템을 초기화하고 관리합니다.
 * 또한 Wwise 이벤트를 호출하는 기능도 제공합니다.
 */

class UGS_UIAudioSystem;
class UAkComponent;
class UAkAudioEvent;
class UAkRtpc;
class USoundClass;
class USoundMix;

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

	// === BGM 볼륨 설정 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (DisplayName = "BGM 볼륨 설정"))
	void SetBGMVolume(float Volume);

	// 현재 BGM 볼륨 가져오기
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (DisplayName = "현재 BGM 볼륨 가져오기"))
	float GetCurrentBGMVolume() const { return CurrentBGMVolume; }

	// === SFX 볼륨 설정 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|SFX", meta = (DisplayName = "SFX 볼륨 설정"))
	void SetSFXVolume(float Volume);

	// 현재 SFX 볼륨 가져오기
	UFUNCTION(BlueprintCallable, Category = "Audio|SFX", meta = (DisplayName = "현재 SFX 볼륨 가져오기"))
	float GetCurrentSFXVolume() const { return CurrentSFXVolume; }

	// === 통합 전투 시퀀스 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|Combat", meta = (DisplayName = "전투 시퀀스 시작", ToolTip = "맵 BGM을 페이드아웃/정지하고 전투 BGM을 시작합니다."))
	void StartCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStartEvent, UAkAudioEvent* CombatMusicStopEvent, float FadeTime = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Combat", meta = (DisplayName = "전투 시퀀스 종료", ToolTip = "전투 BGM을 중지하고 맵 BGM을 복원합니다."))
	void EndCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStopEvent = nullptr, float FadeTime = 3.0f);

	// === 보스 룸 시퀀스 ===
	UFUNCTION(BlueprintCallable, Category = "Audio|Boss", meta = (DisplayName = "보스 시퀀스 시작"))
	void StartBossSequence(AActor* Context, UAkAudioEvent* InBossMusicStartEvent, UAkAudioEvent* InBossMusicStopEvent);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartBossSequence(AActor* Context, UAkAudioEvent* InBossMusicStartEvent, UAkAudioEvent* InBossMusicStopEvent);

	UFUNCTION(BlueprintCallable, Category = "Audio|Boss", meta = (DisplayName = "보스 시퀀스 종료"))
	void EndBossSequence(AActor* Context, float FadeTime = 3.0f);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndBossSequence(AActor* Context, float FadeTime = 3.0f);

	// === 로컬 전용 보스 시퀀스 (멀티캐스트 없음) ===
	UFUNCTION(BlueprintCallable, Category = "Audio|Boss", meta = (DisplayName = "보스 시퀀스 시작 (로컬)"))
	void StartBossSequenceLocal(AActor* Context, UAkAudioEvent* InBossMusicStartEvent, UAkAudioEvent* InBossMusicStopEvent);

	UFUNCTION(BlueprintCallable, Category = "Audio|Boss", meta = (DisplayName = "보스 시퀀스 종료 (로컬)"))
	void EndBossSequenceLocal(AActor* Context, float FadeTime = 3.0f);

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

	// === SFX 볼륨 관련 에셋들 ===
	UPROPERTY(EditDefaultsOnly, Category = "Audio|SFX", meta = (DisplayName = "SFX 볼륨 RTPC"))
	UAkRtpc* SFXVolumeRTPC;

	// === 네이티브 오디오 시스템 지원 ===
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Map BGM", meta = (DisplayName = "BGM 사운드 클래스"))
	USoundClass* BGMSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Map BGM", meta = (DisplayName = "BGM 사운드 믹스"))
	USoundMix* BGMSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|SFX", meta = (DisplayName = "SFX 사운드 클래스"))
	USoundClass* SFXSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|SFX", meta = (DisplayName = "SFX 사운드 믹스"))
	USoundMix* SFXSoundMix;

private:
	// 생성된·파괴 주기는 GameInstance와 동기화
	UPROPERTY()
	UGS_UIAudioSystem* UIAudio;

	// BGM 전용 AkComponent (오클루전 비활성화)
	UPROPERTY()
	UAkComponent* BGMAkComponent;

	// 맵 BGM 상태 관리
	bool bIsMapBGMPlaying;

	// 전투 BGM 상태 관리
	bool bIsCombatMusicPlaying;

	// 보스룸 BGM 상태 관리
	bool bIsBossMusicPlaying;

	// 전투 BGM 관리
	UPROPERTY()
	UAkAudioEvent* CurrentCombatMusicStartEvent;

	UPROPERTY()
	UAkAudioEvent* CurrentCombatMusicStopEvent;

	// 기본 전투 BGM 정지 이벤트 (EV_CombatStop)
	UPROPERTY()
	UAkAudioEvent* DefaultCombatStopEvent;

	// 보스룸 BGM 관리
	UPROPERTY()
	UAkAudioEvent* CurrentBossMusicStartEvent;

	UPROPERTY()
	UAkAudioEvent* CurrentBossMusicStopEvent;

	// 기본 보스룸 BGM 이벤트
	UPROPERTY()
	UAkAudioEvent* DefaultBossMusicStartEvent;
	
	UPROPERTY()
	UAkAudioEvent* DefaultBossMusicStopEvent;

	// 맵 BGM 페이드인/아웃 타이머 핸들 (멤버 변수로 관리!)
	FTimerHandle MapBGMFadeInTimerHandle;
	FTimerHandle MapBGMFadeOutTimerHandle;
	FTimerHandle MapBGMStopDelayTimerHandle;

	// 현재 BGM 볼륨 (0.0 ~ 1.0)
	float CurrentBGMVolume;

	// 현재 SFX 볼륨 (0.0 ~ 1.0)
	float CurrentSFXVolume;

	// 타이머 콜백용 캐시 변수
	TWeakObjectPtr<AActor> CachedTargetActor;
	float CachedFadeTime;

	// RTPC 헬퍼 함수
	void SetRTPCValue(UAkRtpc* RTPC, float Value, AActor* Context, float InterpolationTime = 0.0f);

	// 네이티브 사운드 클래스 볼륨 조절 헬퍼 함수
	void SetNativeSoundClassVolume(float Volume);

	/**
	 * @brief 타이머를 안전하게 정리합니다.
	 * @param TimerHandle 정리할 타이머 핸들
	 */
	void SafeClearTimer(FTimerHandle& TimerHandle);

	/**
	 * @brief 월드 컨텍스트가 유효한지 검증합니다.
	 * @return 월드가 유효하고 teardown 중이 아니면 true
	 */
	bool IsWorldContextValid() const;

	// === 타이머 콜백 함수들 (UFUNCTION으로 선언) ===

	/**
	 * @brief StopMapBGM 지연 실행 콜백
	 */
	UFUNCTION()
	void OnMapBGMStopDelayCallback();

	/**
	 * @brief FadeOut 완료 후 정지 콜백
	 */
	UFUNCTION()
	void OnMapBGMFadeOutCompleteCallback();

	/**
	 * @brief FadeIn 시작 콜백
	 */
	UFUNCTION()
	void OnMapBGMFadeInStartCallback();

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
	* @brief 맵 로딩 시작 시 호출되는 콜백 (BGM 정지용)
	*/
	void OnPreLoadMap(const FString& MapName);

	/**
	* @brief 창 포커스 손실 시 호출되는 콜백 (모든 오디오 음소거)
	*/
	void OnApplicationDeactivated();

	/**
	* @brief 창 포커스 복원 시 호출되는 콜백 (오디오 복원)
	*/
	void OnApplicationActivated();

	/**
	* @brief 뷰포트 포커스 변경 시 호출되는 콜백 (에디터 PIE용)
	* @param bIsActive 활성화 상태
	*/
	void OnViewportFocusChanged(bool bIsActive);

	/**
	* @brief 현재 게임 모드(TPS/RTS)에 따라 BGM을 재생하거나 정지할 대상 액터를 결정합니다.
	* @param Context 컨텍스트로 제공된 액터 (옵셔널)
	* @return 결정된 타겟 액터
	*/
	AActor* GetTargetActorForPlayback(AActor* Context);

	/**
	* @brief 현재 재생 중인 전투 음악을 정지합니다.
	* @param Context 컨텍스트 액터
	*/
	void StopCurrentCombatMusic(AActor* Context);

	/**
	* @brief 현재 재생 중인 보스 음악을 정지합니다.
	* @param Context 컨텍스트 액터
	 */
	void StopCurrentBossMusic(AActor* Context);

	/**
	 * @brief BGM 전용 AkComponent를 가져오거나 생성합니다.
	 * @return BGM 재생용 AkComponent (오클루전 비활성화됨)
	 */
	UAkComponent* GetOrCreateBGMAkComponent();
};
