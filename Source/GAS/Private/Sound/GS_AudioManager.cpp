// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_AudioManager.h"
#include "Sound/GS_UIAudioSystem.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "AkRtpc.h"
#include "UObject/UObjectGlobals.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"

UGS_AudioManager::UGS_AudioManager()
{
	// 맵 BGM 상태 초기화
	bIsMapBGMPlaying = false;

	// 전투 BGM 상태 초기화
	bIsCombatMusicPlaying = false;

	// 보스룸 BGM 상태 초기화
	bIsBossMusicPlaying = false;

	// BGM 볼륨 초기화
	CurrentBGMVolume = 1.0f;

	// SFX 볼륨 초기화
	CurrentSFXVolume = 1.0f;

	// 포인터 멤버 초기화
	UIAudio = nullptr;
	BGMAkComponent = nullptr;
	CachedTargetActor = nullptr;
	CachedFadeTime = 0.0f;

	// 맵 BGM 멤버 초기화
	MapBGMEvent = nullptr;
	MapBGMStopEvent = nullptr;
	MapBGMVolumeRTPC = nullptr;

	// 전투 BGM 멤버 초기화
	CurrentCombatMusicStartEvent = nullptr;
	CurrentCombatMusicStopEvent = nullptr;

	// 보스룸 BGM 멤버 초기화
	CurrentBossMusicStartEvent = nullptr;
	CurrentBossMusicStopEvent = nullptr;
	DefaultBossMusicStartEvent = nullptr;
	DefaultBossMusicStopEvent = nullptr;

	// 기본 전투 BGM StopEvent 로드
	DefaultCombatStopEvent = nullptr;

	// Wwise 에셋 로드
	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> MapBGMEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_MapBGM_Play.EV_MapBGM_Play"));
	if (MapBGMEventFinder.Succeeded())
	{
		MapBGMEvent = MapBGMEventFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> MapBGMStopEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_MapBGM_Stop.EV_MapBGM_Stop"));
	if (MapBGMStopEventFinder.Succeeded())
	{
		MapBGMStopEvent = MapBGMStopEventFinder.Object;
	}

	// 기본 전투 BGM 정지 이벤트 로드
	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> CombatStopEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_CombatStop.EV_CombatStop"));
	if (CombatStopEventFinder.Succeeded())
	{
		DefaultCombatStopEvent = CombatStopEventFinder.Object;
	}

	// 기본 보스룸 BGM 이벤트 로드
	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> BossStartEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_BossRoom_Play.EV_BossRoom_Play"));
	if (BossStartEventFinder.Succeeded())
	{
		DefaultBossMusicStartEvent = BossStartEventFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> BossStopEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_BossRoom_Stop.EV_BossRoom_Stop"));
	if (BossStopEventFinder.Succeeded())
	{
		DefaultBossMusicStopEvent = BossStopEventFinder.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UAkRtpc> MapBGMVolumeRTPCFinder(TEXT("/Game/WwiseAudio/Game_Parameters/Default_Work_Unit/MapBGMVolume.MapBGMVolume"));
	if (MapBGMVolumeRTPCFinder.Succeeded())
	{
		MapBGMVolumeRTPC = MapBGMVolumeRTPCFinder.Object;
	}

	// SFX 볼륨 RTPC 로드
	static ConstructorHelpers::FObjectFinder<UAkRtpc> SFXVolumeRTPCFinder(TEXT("/Game/WwiseAudio/Game_Parameters/Default_Work_Unit/SFXVolume.SFXVolume"));
	if (SFXVolumeRTPCFinder.Succeeded())
	{
		SFXVolumeRTPC = SFXVolumeRTPCFinder.Object;
	}


	// 네이티브 BGM 사운드 클래스 로드
	static ConstructorHelpers::FObjectFinder<USoundClass> BGMSoundClassFinder(TEXT("/Game/WwiseAudio/SC_BGM.SC_BGM"));
	if (BGMSoundClassFinder.Succeeded())
	{
		BGMSoundClass = BGMSoundClassFinder.Object;
	}
	else
	{
		BGMSoundClass = nullptr;
	}

	// 네이티브 BGM 사운드 믹스 로드
	static ConstructorHelpers::FObjectFinder<USoundMix> BGMSoundMixFinder(TEXT("/Game/WwiseAudio/SM_BGM.SM_BGM"));
	if (BGMSoundMixFinder.Succeeded())
	{
		BGMSoundMix = BGMSoundMixFinder.Object;
	}
	else
	{
		BGMSoundMix = nullptr;
	}

	// 네이티브 SFX 사운드 클래스 로드
	static ConstructorHelpers::FObjectFinder<USoundClass> SFXSoundClassFinder(TEXT("/Game/WwiseAudio/SC_SFX.SC_SFX"));
	if (SFXSoundClassFinder.Succeeded())
	{
		SFXSoundClass = SFXSoundClassFinder.Object;
	}
	else
	{
		SFXSoundClass = nullptr;
	}

	// 네이티브 SFX 사운드 믹스 로드
	static ConstructorHelpers::FObjectFinder<USoundMix> SFXSoundMixFinder(TEXT("/Game/WwiseAudio/SM_SFX.SM_SFX"));
	if (SFXSoundMixFinder.Succeeded())
	{
		SFXSoundMix = SFXSoundMixFinder.Object;
	}
	else
	{
		SFXSoundMix = nullptr;
	}
}

void UGS_AudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 오디오 시스템 인스턴스 생성
	UIAudio = NewObject<UGS_UIAudioSystem>(this);

	// 맵 BGM 상태 초기화
	bIsMapBGMPlaying = false;

	// 전투 BGM 상태 초기화
	bIsCombatMusicPlaying = false;

	// 보스룸 BGM 상태 초기화
	bIsBossMusicPlaying = false;

	// 오디오 에셋 유효성 검사
	if (!ValidateAudioAssets())
	{
		UE_LOG(LogTemp, Warning, TEXT("일부 오디오 에셋이 누락되었지만 시스템을 계속 진행합니다."));
	}

	// 맵 전환 시 BGM 정지를 위한 델리게이트 바인딩
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UGS_AudioManager::OnPreLoadMap);

	// 창 포커스 이벤트 바인딩 (패키징된 빌드용)
	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &UGS_AudioManager::OnApplicationDeactivated);
	FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject(this, &UGS_AudioManager::OnApplicationActivated);

	// 에디터 뷰포트 포커스 이벤트 바인딩 (PIE용)
	// 서버에서는 Slate가 초기화되지 않으므로 체크 필요
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject(this, &UGS_AudioManager::OnViewportFocusChanged);
	}
}

void UGS_AudioManager::Deinitialize()
{
	// 델리게이트 해제
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreDelegates::ApplicationWillDeactivateDelegate.RemoveAll(this);
	FCoreDelegates::ApplicationHasReactivatedDelegate.RemoveAll(this);
	
	// Slate 델리게이트 해제 (초기화된 경우에만)
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().RemoveAll(this);
	}

	// 모든 타이머 정리 (레벨 전환 안전성 보장)
	SafeClearTimer(MapBGMFadeInTimerHandle);
	SafeClearTimer(MapBGMFadeOutTimerHandle);
	SafeClearTimer(MapBGMStopDelayTimerHandle);

	// BGM AkComponent 정리
	if (BGMAkComponent && BGMAkComponent->IsValidLowLevel())
	{
		BGMAkComponent->Stop();
		BGMAkComponent->DestroyComponent();
		BGMAkComponent = nullptr;
	}

	// 메모리 해제 처리
	UIAudio = nullptr;
	CachedTargetActor = nullptr;

	// 맵 BGM 상태 정리
	bIsMapBGMPlaying = false;

	// 전투 BGM 상태 정리
	CurrentCombatMusicStartEvent = nullptr;
	CurrentCombatMusicStopEvent = nullptr;

	// 보스룸 BGM 상태 정리
	CurrentBossMusicStartEvent = nullptr;
	CurrentBossMusicStopEvent = nullptr;

	Super::Deinitialize();
}

// === BGM 전용 AkComponent 생성/관리 ===

UAkComponent* UGS_AudioManager::GetOrCreateBGMAkComponent()
{
	// 이미 생성되어 있고 유효하면 반환
	if (BGMAkComponent && BGMAkComponent->IsValidLowLevel())
	{
		return BGMAkComponent;
	}

	// World 가져오기
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] GetOrCreateBGMAkComponent: World가 nullptr입니다!"));
		return nullptr;
	}

	// PlayerController 가져오기 (AkComponent를 붙일 액터)
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] GetOrCreateBGMAkComponent: PlayerController를 찾을 수 없습니다!"));
		return nullptr;
	}

	// AkComponent 생성
	BGMAkComponent = NewObject<UAkComponent>(PC, UAkComponent::StaticClass(), TEXT("BGMAkComponent"));
	if (!BGMAkComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] GetOrCreateBGMAkComponent: AkComponent 생성 실패!"));
		return nullptr;
	}

	// 컴포넌트 등록 및 초기화
	BGMAkComponent->RegisterComponent();

	// 오클루전/오브스트럭션 비활성화 (BGM은 공간 감쇠 없음)
	BGMAkComponent->OcclusionRefreshInterval = 0.0f; // 오클루전 계산 비활성화

	// 2D 사운드로 설정 (감쇠 없음)
	BGMAkComponent->bUseReverbVolumes = false;
	BGMAkComponent->EnableSpotReflectors = false;

	return BGMAkComponent;
}

// Wwise 이벤트 호출 함수
void UGS_AudioManager::PlayEvent(UAkAudioEvent* Event, AActor* Context)
{
	if (!Event || !Context)
	{
		return;
	}
	FOnAkPostEventCallback DummyCallback;
	UAkGameplayStatics::PostEvent(Event, Context, 0, DummyCallback);
}

// === 타겟 액터 결정 헬퍼 함수 ===
AActor* UGS_AudioManager::GetTargetActorForPlayback(AActor* Context)
{
	// 컨텍스트가 명시적으로 제공된 경우, 해당 컨텍스트를 사용
	if (Context)
	{
		return Context;
	}

	// 월드나 플레이어 컨트롤러가 유효하지 않으면 전역 재생 (nullptr)
	if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
	{
		return nullptr;
	}

	// 플레이어 컨트롤러로부터 Pawn을 가져옴
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* PlayerPawn = PC->GetPawn();

	// Pawn이 유효하면 해당 Pawn을 타겟으로, 그렇지 않으면 전역 재생 (nullptr)
	return PlayerPawn;
}

// === 오디오 에셋 유효성 검사 ===
bool UGS_AudioManager::ValidateAudioAssets()
{
	bool bAllAssetsValid = true;
	
	if (!MapBGMEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("필수 에셋 누락: MapBGMEvent"));
		bAllAssetsValid = false;
	}
	
	if (!MapBGMStopEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("필수 에셋 누락: MapBGMStopEvent"));
		bAllAssetsValid = false;
	}
	
	if (!MapBGMVolumeRTPC)
	{
		UE_LOG(LogTemp, Error, TEXT("필수 에셋 누락: MapBGMVolumeRTPC"));
		bAllAssetsValid = false;
	}
	
	if (!bAllAssetsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("오디오 시스템 초기화 실패 - 필수 에셋이 누락되었습니다."));
		return false;
	}
	
	return true;
}

void UGS_AudioManager::OnPreLoadMap(const FString& MapName)
{
	if (!IsAudioProcessingAllowed())
	{
		return;
	}
		
	AActor* TargetActor = GetTargetActorForPlayback(nullptr);
	
	// 1. 맵 BGM 정지
	if (bIsMapBGMPlaying)
	{
		StopMapBGM(nullptr);
	}

	// 2. 전투 BGM 정지 (헬퍼 함수 활용)
	if (CurrentCombatMusicStartEvent)
	{
		StopCurrentCombatMusic(TargetActor);

		// 상태 초기화
		CurrentCombatMusicStartEvent = nullptr;
		CurrentCombatMusicStopEvent = nullptr;
		bIsCombatMusicPlaying = false;  // 전투 BGM 플래그 리셋
	}

	// 보스룸 BGM 정지
	if (CurrentBossMusicStartEvent)
	{
		StopCurrentBossMusic(TargetActor);
		CurrentBossMusicStartEvent = nullptr;
		CurrentBossMusicStopEvent = nullptr;
		bIsBossMusicPlaying = false;
	}
	
	// 3. RTPC를 현재 사용자 설정 볼륨으로 유지 (다음 맵에서도 동일한 볼륨 유지)
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, 0.0f);
	}

	// 4. BGMAkComponent 정리
	if (BGMAkComponent && BGMAkComponent->IsValidLowLevel())
	{
		BGMAkComponent->Stop();
		BGMAkComponent->DestroyComponent();
		BGMAkComponent = nullptr;
	}
}

// === 멀티플레이어 지원 헬퍼 ===
bool UGS_AudioManager::IsAudioProcessingAllowed() const
{
	// 전용 서버에서는 오디오를 처리하지 않음
	return GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer;
}


// === 맵 BGM 관리 시스템 ===
void UGS_AudioManager::StartMapBGM(AActor* Context)
{
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	if (!MapBGMEvent)
	{
		return;
	}

	if (bIsMapBGMPlaying)
	{
		return; // 중복 재생 방지
	}

	if (!FAkAudioDevice::Get())
	{
		return;
	}
	
	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetTargetActorForPlayback(Context);

	// RTPC 볼륨을 현재 볼륨으로 설정
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, 0.0f);
	}

	// 실제 BGM 시작
	// BGM은 전용 AkComponent를 사용 (오클루전 비활성화)
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (!BGMComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] StartMapBGM: BGM AkComponent를 생성할 수 없습니다!"));
		return;
	}

	FOnAkPostEventCallback DummyCallback;
	int32 PlayingID = BGMComponent->PostAkEvent(MapBGMEvent, 0, DummyCallback);

	if (PlayingID != AK_INVALID_PLAYING_ID)
	{
		bIsMapBGMPlaying = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] 맵 BGM 재생 실패!"));
	}
}

void UGS_AudioManager::StopMapBGM(AActor* Context)
{
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	if (!bIsMapBGMPlaying)
	{
		return;
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetTargetActorForPlayback(Context);

	// Wwise Stop 이벤트를 사용한 부드러운 정지
	// BGM 전용 AkComponent 사용
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (MapBGMStopEvent && BGMComponent)
	{
		FOnAkPostEventCallback DummyCallback;
		BGMComponent->PostAkEvent(MapBGMStopEvent, 0, DummyCallback);
		bIsMapBGMPlaying = false;
	}
	else if (!MapBGMStopEvent)
	{
		// Stop 이벤트가 없다면 MapBGM만 선택적으로 정지
		if (MapBGMVolumeRTPC)
		{
			SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);
		}

		// 볼륨을 0으로 만든 후 짧은 지연으로 정지 (UFUNCTION 멤버 함수 사용)
		if (IsWorldContextValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
				MapBGMStopDelayTimerHandle,
				this,
				&UGS_AudioManager::OnMapBGMStopDelayCallback,
				0.1f,
				false
			);
		}
	}
}



// === RTPC 헬퍼 함수 ===

void UGS_AudioManager::SetRTPCValue(UAkRtpc* RTPC, float Value, AActor* Context, float InterpolationTime)
{
	if (!RTPC)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] SetRTPCValue: RTPC가 nullptr입니다!"));
		return;
	}

	// 0.0~1.0 범위를 0~100 범위로 변환
	float WwiseValue = Value * 100.0f;

	// Wwise 오디오 디바이스를 통해 RTPC 값 설정
	// BGM RTPC는 Global로 설정 (Context를 nullptr로 전달)
	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		int32 InterpolationTimeMs = FMath::RoundToInt(InterpolationTime);

		// BGM RTPC는 항상 Global로 적용 (Context 무시)
		AKRESULT Result = AudioDevice->SetRTPCValue(RTPC, WwiseValue, InterpolationTimeMs, nullptr);

		if (Result != AK_Success)
		{
			UE_LOG(LogTemp, Error, TEXT("[AudioManager] RTPC 설정 실패: %s = %.0f (Result: %d)"),
				   *RTPC->GetName(), WwiseValue, (int32)Result);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] Wwise AudioDevice를 찾을 수 없습니다!"));
	}
}

// === BGM 볼륨 설정 ===

void UGS_AudioManager::SetBGMVolume(float Volume)
{
	// 볼륨 값을 0.0~1.0 범위로 클램프하고 저장
	CurrentBGMVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	// === 1. Wwise BGM 볼륨 조절 ===
	if (MapBGMVolumeRTPC)
	{
		// 게임 모드에 따른 조건부 타겟 액터 결정
		AActor* TargetActor = GetTargetActorForPlayback(nullptr);

		// RTPC 값 설정 (SetRTPCValue가 0~100 범위로 자동 변환함)
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, 0.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AudioManager] MapBGMVolumeRTPC가 설정되지 않았습니다. Wwise BGM 볼륨 조절 건너뜀."));
	}

	// === 2. 네이티브 오디오 시스템 BGM 볼륨 조절 ===
	SetNativeSoundClassVolume(CurrentBGMVolume);
}

void UGS_AudioManager::SetSFXVolume(float Volume)
{
	// 볼륨 값을 0.0~1.0 범위로 클램프하고 저장
	CurrentSFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	// === 1. Wwise SFX 볼륨 조절 ===
	if (SFXVolumeRTPC)
	{
		// 게임 모드에 따른 조건부 타겟 액터 결정
		AActor* TargetActor = GetTargetActorForPlayback(nullptr);

		// RTPC 값 설정 (SetRTPCValue가 0~100 범위로 자동 변환함)
		SetRTPCValue(SFXVolumeRTPC, CurrentSFXVolume, TargetActor, 0.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AudioManager] SFXVolumeRTPC가 설정되지 않았습니다. Wwise SFX 볼륨 조절 건너뜀."));
	}

	// === 2. 네이티브 오디오 시스템 SFX 볼륨 조절 ===
	if (SFXSoundClass && SFXSoundMix)
	{
		if (UWorld* World = GetWorld())
		{
			if (FAudioDevice* AudioDevice = World->GetAudioDeviceRaw())
			{
				AudioDevice->SetSoundMixClassOverride(SFXSoundMix, SFXSoundClass, CurrentSFXVolume, 1.0f, 0.0f, true);
				AudioDevice->PushSoundMixModifier(SFXSoundMix);
			}
		}
	}
}

// === 통합 전투 시스템 ===

void UGS_AudioManager::StopCurrentCombatMusic(AActor* Context)
{
	// 기존 전투 음악이 없으면 조기 종료
	if (!CurrentCombatMusicStartEvent)
	{
		return;
	}

	// BGM 전용 AkComponent 가져오기
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (!BGMComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] StopCurrentCombatMusic: BGM AkComponent를 찾을 수 없습니다!"));
		return;
	}

	// StopEvent가 있으면 사용
	if (CurrentCombatMusicStopEvent)
	{
		BGMComponent->PostAkEvent(CurrentCombatMusicStopEvent, 0, FOnAkPostEventCallback());
	}
	else
	{
		// StopEvent가 없으면 BGM Component 전체 정지
		BGMComponent->Stop();
	}
}

void UGS_AudioManager::StartCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStartEvent, UAkAudioEvent* CombatMusicStopEvent, float FadeTime)
{
	if (!Context || !CombatMusicStartEvent)
	{
		return;
	}

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		// 서버에서는 전투 음악 상태만 저장
		CurrentCombatMusicStartEvent = CombatMusicStartEvent;
		CurrentCombatMusicStopEvent = CombatMusicStopEvent;
		bIsCombatMusicPlaying = true;
		return;
	}

	// 중복 재생 방지: 이미 같은 전투 BGM이 재생 중이면 중단
	if (bIsCombatMusicPlaying && CurrentCombatMusicStartEvent == CombatMusicStartEvent)
	{
		return;
	}

	// 다른 전투 BGM이 재생 중이면 교체 허용 (다른 몬스터 종류)
	if (bIsCombatMusicPlaying && CurrentCombatMusicStartEvent != CombatMusicStartEvent)
	{
		// 기존 BGM 정지 후 새 BGM 재생 (아래 로직 계속 진행)
	}

	// 보스룸 BGM이 재생중이면 정지
	if (bIsBossMusicPlaying)
	{
		StopCurrentBossMusic(Context);
		bIsBossMusicPlaying = false;
		CurrentBossMusicStartEvent = nullptr;
		CurrentBossMusicStopEvent = nullptr;
	}

	// 1. 기존 전투 음악 정지
	StopCurrentCombatMusic(Context);

	// 2. 전투 음악 상태 저장
	CurrentCombatMusicStartEvent = CombatMusicStartEvent;
	CurrentCombatMusicStopEvent = CombatMusicStopEvent;

	// 3. 맵 BGM 즉시 정지
	AActor* TargetActor = GetTargetActorForPlayback(nullptr);
	if (bIsMapBGMPlaying)
	{
		StopMapBGM(TargetActor);
	}

	// 4. 전투 BGM 즉시 시작
	// 전투 BGM도 전용 AkComponent 사용 (오클루전 비활성화)
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (BGMComponent && CombatMusicStartEvent)
	{
		int32 PlayingID = BGMComponent->PostAkEvent(CombatMusicStartEvent, 0, FOnAkPostEventCallback());
		if (PlayingID != AK_INVALID_PLAYING_ID)
		{
			bIsCombatMusicPlaying = true;  // 전투 BGM 재생 상태로 설정
		}
	}

	// 5. 전투 BGM에 현재 볼륨 적용 (Wwise에서 Music Bus에 RTPC가 연결되어 있어야 함)
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, 0.0f);
	}
}

void UGS_AudioManager::EndCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStopEvent, float FadeTime)
{
	if (!Context)
	{
		return;
	}

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		CurrentCombatMusicStartEvent = nullptr;
		CurrentCombatMusicStopEvent = nullptr;
		return;
	}

	// 1. 전투 BGM 정지
	AActor* TargetActor = GetTargetActorForPlayback(Context);

	// 제공된 StopEvent 우선, 없으면 저장된 StopEvent 사용
	UAkAudioEvent* StopEventToUse = CombatMusicStopEvent ? CombatMusicStopEvent : CurrentCombatMusicStopEvent;

	// BGM 전용 AkComponent 가져오기
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (!BGMComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] EndCombatSequence: BGM AkComponent를 찾을 수 없습니다!"));
		return;
	}

	if (StopEventToUse)
	{
		// BGM 전용 컴포넌트로 정지
		BGMComponent->PostAkEvent(StopEventToUse, 0, FOnAkPostEventCallback());
	}
	else if (CurrentCombatMusicStartEvent)
	{
		// StopEvent가 없으면 BGM Component 전체 정지
		BGMComponent->Stop();
	}

	// 2. 전투 음악 상태 초기화
	CurrentCombatMusicStartEvent = nullptr;
	CurrentCombatMusicStopEvent = nullptr;
	bIsCombatMusicPlaying = false;  // 전투 BGM 정지 상태로 설정

	// 3. MapBGMVolume RTPC를 현재 볼륨으로 설정 (맵 BGM이 들리도록)
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, FadeTime * 1000.0f);
	}

	// 4. 맵 BGM 복원 (RTPC가 이미 올라가고 있으므로 즉시 시작)
	if (!bIsMapBGMPlaying)
	{
		StartMapBGM(TargetActor);
	}
}

// === 보스룸 시퀀스 ===

void UGS_AudioManager::StopCurrentBossMusic(AActor* Context)
{
	// 기존 보스룸 음악이 없으면 조기 종료
	if (!CurrentBossMusicStartEvent)
	{
		return;
	}

	// BGM 전용 AkComponent 가져오기
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (!BGMComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] StopCurrentBossMusic: BGM AkComponent를 찾을 수 없습니다!"));
		return;
	}

	// StopEvent가 있으면 사용, 없으면 기본 이벤트 사용
	UAkAudioEvent* StopEventToUse = CurrentBossMusicStopEvent ? CurrentBossMusicStopEvent : DefaultBossMusicStopEvent;
	if (StopEventToUse)
	{
		BGMComponent->PostAkEvent(StopEventToUse, 0, FOnAkPostEventCallback());
	}
	else
	{
		// StopEvent가 없으면 BGM Component 전체 정지
		BGMComponent->Stop();
	}
}

void UGS_AudioManager::StartBossSequence(AActor* Context, UAkAudioEvent* InBossMusicStartEvent, UAkAudioEvent* InBossMusicStopEvent)
{
	// 서버에서 멀티캐스트로 모든 클라이언트에 전파
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone)
	{
		Multicast_StartBossSequence(Context, InBossMusicStartEvent, InBossMusicStopEvent);
	}
	else
	{
		// 스탠드얼론이면 직접 실행
		Multicast_StartBossSequence(Context, InBossMusicStartEvent, InBossMusicStopEvent);
	}
}

void UGS_AudioManager::Multicast_StartBossSequence_Implementation(AActor* Context, UAkAudioEvent* InBossMusicStartEvent, UAkAudioEvent* InBossMusicStopEvent)
{
	UAkAudioEvent* BossStartEvent = InBossMusicStartEvent ? InBossMusicStartEvent : DefaultBossMusicStartEvent;
	UAkAudioEvent* BossStopEvent = InBossMusicStopEvent ? InBossMusicStopEvent : DefaultBossMusicStopEvent;

	if (!Context || !BossStartEvent)
	{
		return;
	}

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		// 서버에서는 전투 음악 상태만 저장
		CurrentBossMusicStartEvent = BossStartEvent;
		CurrentBossMusicStopEvent = BossStopEvent;
		bIsBossMusicPlaying = true;
		return;
	}

	// 중복 재생 방지: 이미 같은 보스룸 BGM이 재생 중이면 중단
	if (bIsBossMusicPlaying && CurrentBossMusicStartEvent == BossStartEvent)
	{
		return;
	}

	// 1. 기존 전투/보스룸 음악 정지
	if (bIsCombatMusicPlaying)
	{
		StopCurrentCombatMusic(Context);
		bIsCombatMusicPlaying = false;
		CurrentCombatMusicStartEvent = nullptr;
		CurrentCombatMusicStopEvent = nullptr;
	}
	if (bIsBossMusicPlaying)
	{
		StopCurrentBossMusic(Context);
	}

	// 2. 보스룸 음악 상태 저장
	CurrentBossMusicStartEvent = BossStartEvent;
	CurrentBossMusicStopEvent = BossStopEvent;

	// 3. 맵 BGM 즉시 정지
	AActor* TargetActor = GetTargetActorForPlayback(nullptr);
	if (bIsMapBGMPlaying)
	{
		StopMapBGM(TargetActor);
	}

	// 4. 보스룸 BGM 즉시 시작
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (BGMComponent && CurrentBossMusicStartEvent)
	{
		int32 PlayingID = BGMComponent->PostAkEvent(CurrentBossMusicStartEvent, 0, FOnAkPostEventCallback());
		if (PlayingID != AK_INVALID_PLAYING_ID)
		{
			bIsBossMusicPlaying = true;
		}
	}

	// 5. 전투 BGM에 현재 볼륨 적용
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, 0.0f);
	}
}

void UGS_AudioManager::EndBossSequence(AActor* Context, float FadeTime)
{
	// 서버에서 멀티캐스트로 모든 클라이언트에 전파
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone)
	{
		Multicast_EndBossSequence(Context, FadeTime);
	}
	else
	{
		// 스탠드얼론이면 직접 실행
		Multicast_EndBossSequence(Context, FadeTime);
	}
}

void UGS_AudioManager::Multicast_EndBossSequence_Implementation(AActor* Context, float FadeTime)
{
	if (!IsAudioProcessingAllowed())
	{
		CurrentBossMusicStartEvent = nullptr;
		CurrentBossMusicStopEvent = nullptr;
		bIsBossMusicPlaying = false;
		return;
	}

	// 1. 보스룸 BGM 정지
	AActor* TargetActor = GetTargetActorForPlayback(Context);
	StopCurrentBossMusic(TargetActor);
	
	// 2. 보스룸 음악 상태 초기화
	CurrentBossMusicStartEvent = nullptr;
	CurrentBossMusicStopEvent = nullptr;
	bIsBossMusicPlaying = false;

	// 3. MapBGMVolume RTPC를 현재 볼륨으로 설정 (맵 BGM이 들리도록)
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, FadeTime * 1000.0f);
	}

	// 4. 맵 BGM 복원
	if (!bIsMapBGMPlaying)
	{
		StartMapBGM(TargetActor);
	}
}

// === 로컬 전용 보스 시퀀스 (멀티캐스트 없음) ===

void UGS_AudioManager::StartBossSequenceLocal(AActor* Context, UAkAudioEvent* InBossMusicStartEvent, UAkAudioEvent* InBossMusicStopEvent)
{
	// 기본 이벤트 사용 (파라미터가 nullptr이면)
	UAkAudioEvent* BossStartEvent = InBossMusicStartEvent ? InBossMusicStartEvent : DefaultBossMusicStartEvent;
	UAkAudioEvent* BossStopEvent = InBossMusicStopEvent ? InBossMusicStopEvent : DefaultBossMusicStopEvent;

	if (!Context || !BossStartEvent)
	{
		return;
	}

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		// 서버에서는 전투 음악 상태만 저장
		CurrentBossMusicStartEvent = BossStartEvent;
		CurrentBossMusicStopEvent = BossStopEvent;
		bIsBossMusicPlaying = true;
		return;
	}

	// 중복 재생 방지: 이미 같은 보스룸 BGM이 재생 중이면 중단
	if (bIsBossMusicPlaying && CurrentBossMusicStartEvent == BossStartEvent)
	{
		return;
	}

	// 1. 기존 전투/보스룸 음악 정지
	if (bIsCombatMusicPlaying)
	{
		StopCurrentCombatMusic(Context);
		bIsCombatMusicPlaying = false;
		CurrentCombatMusicStartEvent = nullptr;
		CurrentCombatMusicStopEvent = nullptr;
	}
	if (bIsBossMusicPlaying)
	{
		StopCurrentBossMusic(Context);
	}

	// 2. 보스룸 음악 상태 저장
	CurrentBossMusicStartEvent = BossStartEvent;
	CurrentBossMusicStopEvent = BossStopEvent;

	// 3. 맵 BGM 즉시 정지
	AActor* TargetActor = GetTargetActorForPlayback(nullptr);
	if (bIsMapBGMPlaying)
	{
		StopMapBGM(TargetActor);
	}

	// 4. 보스룸 BGM 즉시 시작
	UAkComponent* BGMComponent = GetOrCreateBGMAkComponent();
	if (BGMComponent && CurrentBossMusicStartEvent)
	{
		int32 PlayingID = BGMComponent->PostAkEvent(CurrentBossMusicStartEvent, 0, FOnAkPostEventCallback());
		if (PlayingID != AK_INVALID_PLAYING_ID)
		{
			bIsBossMusicPlaying = true;
		}
	}

	// 5. 전투 BGM에 현재 볼륨 적용
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, 0.0f);
	}
}

void UGS_AudioManager::EndBossSequenceLocal(AActor* Context, float FadeTime)
{
	if (!IsAudioProcessingAllowed())
	{
		CurrentBossMusicStartEvent = nullptr;
		CurrentBossMusicStopEvent = nullptr;
		bIsBossMusicPlaying = false;
		return;
	}

	// 1. 보스룸 BGM 정지
	AActor* TargetActor = GetTargetActorForPlayback(Context);
	StopCurrentBossMusic(TargetActor);

	// 2. 보스룸 음악 상태 초기화
	CurrentBossMusicStartEvent = nullptr;
	CurrentBossMusicStopEvent = nullptr;
	bIsBossMusicPlaying = false;

	// 3. MapBGMVolume RTPC를 현재 볼륨으로 설정 (맵 BGM이 들리도록)
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, FadeTime * 1000.0f);
	}

	// 4. 맵 BGM 복원
	if (!bIsMapBGMPlaying)
	{
		StartMapBGM(TargetActor);
	}
}

// === 멀티플레이어 지원 함수들 ===

void UGS_AudioManager::StartMapBGMForAllClients()
{
	StartMapBGM(nullptr);
}

void UGS_AudioManager::FadeOutAndStopMapBGM(AActor* Context, float FadeTime)
{
	if (!bIsMapBGMPlaying)
	{
		return;
	}

	AActor* TargetActor = GetTargetActorForPlayback(Context);

	// RTPC가 없거나 FadeTime이 0이면 즉시 정지
	if (!MapBGMVolumeRTPC || FadeTime <= 0.0f)
	{
		StopMapBGM(TargetActor);
		return;
	}

	// 볼륨 페이드 아웃
	SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, FadeTime * 1000.0f);

	// 타이머 콜백용 캐시 저장
	CachedTargetActor = TargetActor;

	// 기존 타이머 취소
	SafeClearTimer(MapBGMFadeOutTimerHandle);

	// FadeTime 후 정지 (UFUNCTION 멤버 함수 사용)
	if (IsWorldContextValid())
	{
		GetWorld()->GetTimerManager().SetTimer(
			MapBGMFadeOutTimerHandle,
			this,
			&UGS_AudioManager::OnMapBGMFadeOutCompleteCallback,
			FadeTime,
			false
		);
	}
}

void UGS_AudioManager::FadeInAndStartMapBGM(AActor* Context, float FadeTime)
{
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	AActor* TargetActor = GetTargetActorForPlayback(Context);

	// 1. BGM 시작 (아직 재생 중이 아니면)
	if (!bIsMapBGMPlaying)
	{
		StartMapBGM(TargetActor);
	}

	// 2. 볼륨 페이드인 (RTPC가 있고 BGM이 재생 중이면)
	if (!bIsMapBGMPlaying || !MapBGMVolumeRTPC)
	{
		return;
	}

	// 타이머 콜백용 캐시 저장
	CachedTargetActor = TargetActor;
	CachedFadeTime = FadeTime;

	// 기존 타이머 취소
	SafeClearTimer(MapBGMFadeInTimerHandle);

	// 볼륨 0으로 설정 후 페이드인 (UFUNCTION 멤버 함수 사용)
	SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);

	if (IsWorldContextValid())
	{
		GetWorld()->GetTimerManager().SetTimer(
			MapBGMFadeInTimerHandle,
			this,
			&UGS_AudioManager::OnMapBGMFadeInStartCallback,
			0.1f,
			false
		);
	}
}

// === 네이티브 사운드 클래스 볼륨 조절 ===

void UGS_AudioManager::SetNativeSoundClassVolume(float Volume)
{
	// Sound Class와 Sound Mix가 설정되지 않았으면 조기 종료
	if (!BGMSoundClass || !BGMSoundMix)
	{
		return;
	}

	// 월드 유효성 검사
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Sound Class 볼륨 설정 (0.0 ~ 1.0 범위)
	const float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	// UE5 표준 방식: Sound Mix를 통해 Sound Class 볼륨 조절
	// FadeInTime = 0.1초, Duration = -1 (영구 적용)
	UGameplayStatics::SetSoundMixClassOverride(
		World,
		BGMSoundMix,
		BGMSoundClass,
		ClampedVolume,  // Volume
		1.0f,           // Pitch (변경 안 함)
		0.1f,           // FadeInTime
		true            // bApplyToChildren (자식 SoundClass에도 적용)
	);

	// Sound Mix 활성화
	UGameplayStatics::PushSoundMixModifier(World, BGMSoundMix);
}

// === 타이머 관리 헬퍼 함수 ===

void UGS_AudioManager::SafeClearTimer(FTimerHandle& TimerHandle)
{
	if (!TimerHandle.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	TimerHandle.Invalidate();
}

bool UGS_AudioManager::IsWorldContextValid() const
{
	UWorld* World = GetWorld();
	return World &&
		   World->IsValidLowLevel() &&
		   !World->bIsTearingDown &&
		   IsValid(World);
}

// === 타이머 콜백 함수들 (UFUNCTION) ===

void UGS_AudioManager::OnMapBGMStopDelayCallback()
{
	if (!IsWorldContextValid())
	{
		return;
	}

	bIsMapBGMPlaying = false;
	UE_LOG(LogTemp, Warning, TEXT("MapBGM 강제 정지됨 - StopEvent 없음"));
}

void UGS_AudioManager::OnMapBGMFadeOutCompleteCallback()
{
	if (!IsWorldContextValid())
	{
		return;
	}

	AActor* TargetActor = CachedTargetActor.Get();
	StopMapBGM(TargetActor);
	CachedTargetActor = nullptr;
}

void UGS_AudioManager::OnMapBGMFadeInStartCallback()
{
	if (!IsWorldContextValid())
	{
		return;
	}

	if (MapBGMVolumeRTPC && bIsMapBGMPlaying)
	{
		AActor* TargetActor = CachedTargetActor.Get();
		SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, CachedFadeTime * 1000.0f);
	}

	CachedTargetActor = nullptr;
	CachedFadeTime = 0.0f;
}

void UGS_AudioManager::OnApplicationDeactivated()
{
	// 창 포커스 손실 시 모든 오디오 음소거
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	// Wwise 오디오 음소거 (모든 RTPC를 0으로 설정)
	AActor* TargetActor = GetTargetActorForPlayback(nullptr);
	
	// BGM 볼륨 0으로
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);
	}
	
	// SFX 볼륨 0으로
	if (SFXVolumeRTPC)
	{
		SetRTPCValue(SFXVolumeRTPC, 0.0f, TargetActor, 0.0f);
	}

	// 네이티브 오디오 음소거
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UWorld* World = GameInstance->GetWorld();
		if (World)
		{
			if (FAudioDevice* AudioDevice = World->GetAudioDeviceRaw())
			{
				AudioDevice->SetTransientPrimaryVolume(0.0f);
			}
		}
	}
}

void UGS_AudioManager::OnApplicationActivated()
{
	// 창 포커스 복원 시 오디오 복원
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	// Wwise 오디오 복원 (개별 RTPC를 사용자 설정 값으로 복원)
	AActor* TargetActor = GetTargetActorForPlayback(nullptr);
	
	// BGM 볼륨 복원
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, CurrentBGMVolume, TargetActor, 0.0f);
	}
	
	// SFX 볼륨 복원
	if (SFXVolumeRTPC)
	{
		SetRTPCValue(SFXVolumeRTPC, CurrentSFXVolume, TargetActor, 0.0f);
	}

	// 네이티브 오디오 복원
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UWorld* World = GameInstance->GetWorld();
		if (World)
		{
			if (FAudioDevice* AudioDevice = World->GetAudioDeviceRaw())
			{
				AudioDevice->SetTransientPrimaryVolume(1.0f);
				
				// 네이티브 SFX 볼륨도 복원
				if (SFXSoundClass && SFXSoundMix)
				{
					AudioDevice->SetSoundMixClassOverride(SFXSoundMix, SFXSoundClass, CurrentSFXVolume, 1.0f, 0.0f, true);
					AudioDevice->PushSoundMixModifier(SFXSoundMix);
				}
			}
		}
	}
}

void UGS_AudioManager::OnViewportFocusChanged(bool bIsActive)
{
	// 에디터 PIE에서 뷰포트 포커스 변경 시 호출
	if (bIsActive)
	{
		OnApplicationActivated();
	}
	else
	{
		OnApplicationDeactivated();
	}
}
