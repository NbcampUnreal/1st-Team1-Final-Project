// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_AudioManager.h"
#include "Sound/GS_UIAudioSystem.h"
#include "Sound/GS_EnvironmentAudioSystem.h"
#include "AkAudioDevice.h"
#include "System/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UGS_AudioManager::UGS_AudioManager()
{
	// 맵 BGM 상태 초기화
	bIsMapBGMPlaying = false;

	// 캐시 초기화
	CachedWorld = nullptr;
	CachedNetMode = NM_Standalone;
	LastNetModeCheckTime = -1.0f;
	CachedPlayerController = nullptr;
	LastPlayerControllerCheckTime = -1.0f;

	// 포인터 멤버 초기화
	UIAudio = nullptr;
	EnvironmentAudio = nullptr;

	// 맵 BGM 멤버 초기화
	MapBGMEvent = nullptr;
	MapBGMStopEvent = nullptr;
	MapBGMVolumeRTPC = nullptr;

	// 전투 BGM 멤버 초기화
	CurrentCombatMusicStartEvent = nullptr;
	CurrentCombatMusicStopEvent = nullptr;

	// 기본 전투 BGM StopEvent 로드
	DefaultCombatStopEvent = nullptr;

	// Wwise 에셋 로드
	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> MapBGMEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_MapBGM_Play.EV_MapBGM_Play"));
	if (MapBGMEventFinder.Succeeded())
	{
		MapBGMEvent = MapBGMEventFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("맵 BGM 이벤트 로드 실패 - 경로 확인!"));
	}

	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> MapBGMStopEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_MapBGM_Stop.EV_MapBGM_Stop"));
	if (MapBGMStopEventFinder.Succeeded())
	{
		MapBGMStopEvent = MapBGMStopEventFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("맵 BGM 정지 이벤트 로드 실패 - 경로 확인!"));
	}

	// 기본 전투 BGM 정지 이벤트 로드
	static ConstructorHelpers::FObjectFinder<UAkAudioEvent> CombatStopEventFinder(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_CombatStop.EV_CombatStop"));
	if (CombatStopEventFinder.Succeeded())
	{
		DefaultCombatStopEvent = CombatStopEventFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("기본 전투 BGM 정지 이벤트 로드 실패 - 경로 확인: /Game/WwiseAudio/Events/Default_Work_Unit/StateSound/EV_CombatStop"));
	}
	
	static ConstructorHelpers::FObjectFinder<UAkRtpc> MapBGMVolumeRTPCFinder(TEXT("/Game/WwiseAudio/Game_Parameters/Default_Work_Unit/MapBGMVolume.MapBGMVolume"));
	if (MapBGMVolumeRTPCFinder.Succeeded())
	{
		MapBGMVolumeRTPC = MapBGMVolumeRTPCFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("맵 BGM 볼륨 RTPC 로드 실패 - 경로 확인!"));
	}
}

void UGS_AudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 오디오 시스템 인스턴스 생성
	UIAudio = NewObject<UGS_UIAudioSystem>(this);
	EnvironmentAudio = NewObject<UGS_EnvironmentAudioSystem>(this);

	// 맵 BGM 상태 초기화
	bIsMapBGMPlaying = false;

	// 초기 캐시 업데이트
	UpdateCachedValues();
}

void UGS_AudioManager::Deinitialize()
{
	// 메모리 해제 처리
	UIAudio = nullptr;
	EnvironmentAudio = nullptr;

	// 맵 BGM 상태 정리
	bIsMapBGMPlaying = false;

	// 전투 BGM 상태 정리
	CurrentCombatMusicStartEvent = nullptr;
	CurrentCombatMusicStopEvent = nullptr;

	// 캐시 클리어
	CachedWorld = nullptr;
	CachedPlayerController = nullptr;

	Super::Deinitialize();
}

// Wwise 이벤트 호출 함수
void UGS_AudioManager::PlayEvent(UAkAudioEvent* Event, AActor* Context)
{
	if (!Event || !Context)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayEvent 호출 실패: Invalid Event or Context."));
		return;
	}
	FOnAkPostEventCallback DummyCallback;
	UAkGameplayStatics::PostEvent(Event, Context, 0, DummyCallback);
}

// === 맵 BGM 관리 시스템 ===

void UGS_AudioManager::StartMapBGM(AActor* Context)
{
	UpdateCachedValues();
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (ShouldSkipAudioProcessing())
	{
		UE_LOG(LogTemp, Log, TEXT("UGS_AudioManager::StartMapBGM - Skipping audio on dedicated server"));
		return;
	}

	if (!MapBGMEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("UGS_AudioManager::StartMapBGM - MapBGMEvent is null!"));
		return;
	}

	if (bIsMapBGMPlaying)
	{
		return; // 중복 재생 방지
	}

	if (!FAkAudioDevice::Get())
	{
		UE_LOG(LogTemp, Error, TEXT("UGS_AudioManager::StartMapBGM - Wwise AudioDevice is null!"));
		return;
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetOptimalTargetActor(Context);

	// RTPC 볼륨을 먼저 1.0으로 설정
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, 0.0f);
	}

	// 실제 BGM 시작
	FOnAkPostEventCallback DummyCallback;
	UAkGameplayStatics::PostEvent(MapBGMEvent, TargetActor, 0, DummyCallback);
	bIsMapBGMPlaying = true;
}

void UGS_AudioManager::StopMapBGM(AActor* Context)
{
	UpdateCachedValues();
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (ShouldSkipAudioProcessing())
	{
		return;
	}

	if (!bIsMapBGMPlaying)
	{
		return;
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetOptimalTargetActor(Context);

	// Wwise Stop 이벤트를 사용한 부드러운 정지
	if (MapBGMStopEvent)
	{
		FOnAkPostEventCallback DummyCallback;
		UAkGameplayStatics::PostEvent(MapBGMStopEvent, TargetActor, 0, DummyCallback);
		bIsMapBGMPlaying = false;
	}
	else if (!MapBGMStopEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("MapBGMStopEvent가 null입니다! 강제 정지로 대체합니다."));
		
		// Stop 이벤트가 없다면 MapBGM만 선택적으로 정지
		if (MapBGMVolumeRTPC)
		{
			SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);
		}
		
		// 볼륨을 0으로 만든 후 짧은 지연으로 정지
		FTimerHandle StopHandle;
		GetWorld()->GetTimerManager().SetTimer(StopHandle, [this]()
		{
			bIsMapBGMPlaying = false;
			// TargetActor 기반으로 MapBGM만 정지
			UE_LOG(LogTemp, Warning, TEXT("MapBGM 강제 정지됨 - StopEvent 없음"));
		}, 0.1f, false);
	}
}

void UGS_AudioManager::FadeMapBGMForCombat(AActor* Context, float FadeTime)
{
	if (!bIsMapBGMPlaying)
	{
		return;
	}
	
	UpdateCachedValues();
	if (ShouldSkipAudioProcessing()) return;

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetOptimalTargetActor(Context);

	// 맵 BGM 볼륨을 30%로 감소
	float CombatVolume = 0.3f;
	SetRTPCValue(MapBGMVolumeRTPC, CombatVolume, TargetActor, FadeTime * 1000.0f);
}

void UGS_AudioManager::RestoreMapBGMFromCombat(AActor* Context, float FadeTime)
{
	if (!bIsMapBGMPlaying)
	{
		return;
	}

	UpdateCachedValues();
	if (ShouldSkipAudioProcessing()) return;

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetOptimalTargetActor(Context);

	// 맵 BGM 볼륨을 100%로 복원
	SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, FadeTime * 1000.0f);
}

void UGS_AudioManager::SetMapBGMVolume(float Volume, AActor* Context, float FadeTime)
{
	Volume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (!bIsMapBGMPlaying)
	{
		return;
	}

	SetRTPCValue(MapBGMVolumeRTPC, Volume, Context, FadeTime * 1000.0f);
}

// === RTPC 헬퍼 함수 ===

void UGS_AudioManager::SetRTPCValue(UAkRtpc* RTPC, float Value, AActor* Context, float InterpolationTime)
{
	if (!RTPC)
	{
		return;
	}

	// 0.0~1.0 범위를 0~100 범위로 변환
	float WwiseValue = Value * 100.0f;

	// Wwise 오디오 디바이스를 통해 RTPC 값 설정
	if (auto* AudioDevice = FAkAudioDevice::Get())
	{
		int32 InterpolationTimeMs = FMath::RoundToInt(InterpolationTime);
		AKRESULT Result = AudioDevice->SetRTPCValue(RTPC, WwiseValue, InterpolationTimeMs, Context);
		
		if (Result != AK_Success)
		{
			UE_LOG(LogTemp, Warning, TEXT("RTPC 설정 실패: %s = %.0f (Result: %d)"), 
				   *RTPC->GetName(), WwiseValue, (int32)Result);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetRTPCValue: Wwise AudioDevice를 찾을 수 없습니다."));
	}
}

// === 통합 전투 시스템 ===

void UGS_AudioManager::StartCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStartEvent, UAkAudioEvent* CombatMusicStopEvent, float FadeTime)
{
	if (!Context || !CombatMusicStartEvent)
	{
		return;
	}

	UpdateCachedValues();
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (ShouldSkipAudioProcessing())
	{
		CurrentCombatMusicStartEvent = CombatMusicStartEvent;
		CurrentCombatMusicStopEvent = CombatMusicStopEvent;
		return;
	}

	// 기존에 재생 중인 전투 음악이 있다면 정지
	if (CurrentCombatMusicStartEvent && CurrentCombatMusicStopEvent)
	{
		UAkGameplayStatics::PostEvent(CurrentCombatMusicStopEvent, Context, 0, FOnAkPostEventCallback());
	}
	else if (CurrentCombatMusicStartEvent)
	{
		UAkGameplayStatics::StopActor(Context);
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정 (맵 BGM 정지용)
	AActor* MapBGMTargetActor = GetOptimalTargetActor();

	// 멀티플레이어 환경에서는 맵 BGM을 즉시 강제 정지
	if (CachedNetMode != NM_Standalone)
	{
		if (MapBGMVolumeRTPC)
		{
			SetRTPCValue(MapBGMVolumeRTPC, 0.0f, MapBGMTargetActor, 0.0f);
		}
		StopMapBGM(MapBGMTargetActor);
	}
	else
	{
		FadeOutAndStopMapBGM(Context, FadeTime);
	}

	CurrentCombatMusicStartEvent = CombatMusicStartEvent;
	CurrentCombatMusicStopEvent = CombatMusicStopEvent;

	float DelayTime = (CachedNetMode != NM_Standalone) ? 0.1f : FadeTime;

	FTimerHandle CombatBGMHandle;
	GetWorld()->GetTimerManager().SetTimer(CombatBGMHandle, [this, Context, CombatMusicStartEvent]()
	{
		if (Context && CombatMusicStartEvent)
		{
			UAkGameplayStatics::PostEvent(CombatMusicStartEvent, Context, 0, FOnAkPostEventCallback());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UGS_AudioManager::StartCombatSequence - Timer: Failed to start combat BGM (Context: %s, Event: %s)"), 
				   Context ? *Context->GetName() : TEXT("NULL"), 
				   CombatMusicStartEvent ? *CombatMusicStartEvent->GetName() : TEXT("NULL"));
		}
	}, DelayTime, false);
}

void UGS_AudioManager::EndCombatSequence(AActor* Context, UAkAudioEvent* CombatMusicStopEvent, float FadeTime)
{
	if (!Context)
	{
		return;
	}

	UpdateCachedValues();
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (ShouldSkipAudioProcessing())
	{
		CurrentCombatMusicStartEvent = nullptr;
		CurrentCombatMusicStopEvent = nullptr;
		return;
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetOptimalTargetActor(Context);

	// 전투 BGM 정지 처리
	if (CombatMusicStopEvent)
	{
		UAkGameplayStatics::PostEvent(CombatMusicStopEvent, TargetActor, 0, FOnAkPostEventCallback());
	}
	else if (CurrentCombatMusicStopEvent)
	{
		UAkGameplayStatics::PostEvent(CurrentCombatMusicStopEvent, TargetActor, 0, FOnAkPostEventCallback());
	}
	else if (CurrentCombatMusicStartEvent)
	{
		if (TargetActor)
		{
			UAkGameplayStatics::StopActor(TargetActor);
		}
	}

	// 현재 전투 시퀀스 정보 초기화
	CurrentCombatMusicStartEvent = nullptr;
	CurrentCombatMusicStopEvent = nullptr;

	// 맵 BGM 복원
	FadeInAndStartMapBGM(Context, FadeTime);
}

// === 멀티플레이어 지원 함수들 ===

void UGS_AudioManager::StartMapBGMForAllClients()
{
	StartMapBGM(nullptr);
}

bool UGS_AudioManager::IsClientReadyForAudio() const
{
	bool bWwiseReady = (FAkAudioDevice::Get() != nullptr);
	bool bAssetsReady = (MapBGMEvent != nullptr && MapBGMVolumeRTPC != nullptr);
	
	return bWwiseReady && bAssetsReady;
}

void UGS_AudioManager::LogCurrentBGMStatus() const
{
	UWorld* World = GetWorld();
	FString NetModeString = TEXT("알 수 없음");
	
	if (World)
	{
		switch (World->GetNetMode())
		{
		case NM_Standalone: 
			NetModeString = TEXT("단독 실행"); 
			break;
		case NM_DedicatedServer: 
			NetModeString = TEXT("전용 서버"); 
			break;
		case NM_ListenServer: 
			NetModeString = TEXT("리슨 서버"); 
			break;
		case NM_Client: 
			NetModeString = TEXT("클라이언트"); 
			break;
		case NM_MAX:
			NetModeString = TEXT("MAX (잘못된 값)");
			break;
		default:
			NetModeString = TEXT("알 수 없는 네트워크 모드");
			break;
		}
	}
}

void UGS_AudioManager::FadeOutAndStopMapBGM(AActor* Context, float FadeTime)
{
	if (!bIsMapBGMPlaying)
	{
		return;
	}
	
	UpdateCachedValues();
	if (ShouldSkipAudioProcessing()) return;

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetOptimalTargetActor(Context);
	
	if (!MapBGMVolumeRTPC)
	{
		StopMapBGM(TargetActor);
		return;
	}
	
	SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, FadeTime * 1000.0f);
	
	if (FadeTime <= 0.0f)
	{
		StopMapBGM(TargetActor);
		return;
	}
	
	FTimerHandle FadeOutHandle;
	GetWorld()->GetTimerManager().SetTimer(FadeOutHandle, [this, TargetActor]()
	{
		StopMapBGM(TargetActor);
	}, FadeTime, false);
}

void UGS_AudioManager::FadeInAndStartMapBGM(AActor* Context, float FadeTime)
{
	UpdateCachedValues();
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (ShouldSkipAudioProcessing())
	{
		return;
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetOptimalTargetActor(Context);

	// 맵 BGM이 이미 재생 중인지 확인
	if (!bIsMapBGMPlaying)
	{
		StartMapBGM(TargetActor);
	}

	// 기존 페이드인 타이머가 있다면 취소
	if (GetWorld()->GetTimerManager().IsTimerActive(MapBGMFadeInTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(MapBGMFadeInTimerHandle);
	}

	// 볼륨 페이드인
	if (bIsMapBGMPlaying && MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);
		
		GetWorld()->GetTimerManager().SetTimer(MapBGMFadeInTimerHandle, [this, TargetActor, FadeTime]()
		{
			if (MapBGMVolumeRTPC && bIsMapBGMPlaying)
			{
				SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, FadeTime * 1000.0f);
			}
		}, 0.1f, false);
	}
}

// === 성능 최적화 함수 구현 ===

void UGS_AudioManager::UpdateCachedValues()
{
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld) return;

	const float CurrentTime = CurrentWorld->GetTimeSeconds();

	// 월드 캐싱
	if (!CachedWorld.IsValid())
	{
		CachedWorld = CurrentWorld;
	}

	// 네트워크 모드 캐싱 (일정 간격으로)
	if (CurrentTime - LastNetModeCheckTime > NET_MODE_CHECK_INTERVAL)
	{
		CachedNetMode = CurrentWorld->GetNetMode();
		LastNetModeCheckTime = CurrentTime;
	}
	
	// 플레이어 컨트롤러 캐싱 (일정 간격으로)
	if (CurrentTime - LastPlayerControllerCheckTime > PLAYER_CONTROLLER_CHECK_INTERVAL)
	{
		CachedPlayerController = UGameplayStatics::GetPlayerController(CurrentWorld, 0);
		LastPlayerControllerCheckTime = CurrentTime;
	}
}

AActor* UGS_AudioManager::GetOptimalTargetActor(AActor* Context)
{
	if (Context)
	{
		return Context;
	}
	
	if (CachedPlayerController.IsValid())
	{
		// Pawn이 있으면 Pawn을, 없으면(RTS 모드 등) nullptr(전역) 반환
		return CachedPlayerController->GetPawn();
	}
	
	// 어떤 조건도 맞지 않으면 전역으로 처리
	return nullptr;
}

bool UGS_AudioManager::ShouldSkipAudioProcessing() const
{
	// 전용 서버이거나, 월드가 유효하지 않거나, 오디오 디바이스가 없는 경우 오디오 처리 건너뛰기
	return CachedNetMode == NM_DedicatedServer || !CachedWorld.IsValid() || !FAkAudioDevice::Get();
}
