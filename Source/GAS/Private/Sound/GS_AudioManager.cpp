// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_AudioManager.h"
#include "Sound/GS_UIAudioSystem.h"
#include "Sound/GS_EnvironmentAudioSystem.h"
#include "AkAudioDevice.h"

UGS_AudioManager::UGS_AudioManager()
{
	// 맵 BGM 상태 초기화
	bIsMapBGMPlaying = false;

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
	
	static ConstructorHelpers::FObjectFinder<UAkRtpc> MapBGMVolumeRTPCFinder(TEXT("/Game/WwiseAudio/Game_Parameters/Default_Work_Unit/MapBGMVolume.MapBGMVolume"));
	if (MapBGMVolumeRTPCFinder.Succeeded())
	{
		MapBGMVolumeRTPC = MapBGMVolumeRTPCFinder.Object;
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
	
	// 오디오 에셋 유효성 검사
	if (!ValidateAudioAssets())
	{
		UE_LOG(LogTemp, Warning, TEXT("일부 오디오 에셋이 누락되었지만 시스템을 계속 진행합니다."));
	}
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

	Super::Deinitialize();
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
	
	UE_LOG(LogTemp, Log, TEXT("모든 오디오 에셋이 성공적으로 로드되었습니다."));
	return true;
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

	// RTPC 볼륨을 먼저 1.0으로 설정
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, 0.0f);
	}

	// 실제 BGM 시작
	FOnAkPostEventCallback DummyCallback;
	uint32 PlayingID = UAkGameplayStatics::PostEvent(MapBGMEvent, TargetActor, 0, DummyCallback);
	
	bIsMapBGMPlaying = true;
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
	if (MapBGMStopEvent)
	{
		FOnAkPostEventCallback DummyCallback;
		UAkGameplayStatics::PostEvent(MapBGMStopEvent, TargetActor, 0, DummyCallback);
		bIsMapBGMPlaying = false;
	}
	else if (!MapBGMStopEvent)
	{
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

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
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
	AActor* MapBGMTargetActor = GetTargetActorForPlayback();

	// 클라이언트 환경에서는 맵 BGM을 즉시 강제 정지
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		if (MapBGMVolumeRTPC)
		{
			SetRTPCValue(MapBGMVolumeRTPC, 0.0f, MapBGMTargetActor, 0.0f);
		}
		StopMapBGM(MapBGMTargetActor);
	}
	else // 단독 실행 및 리슨 서버 환경에서는 페이드 아웃
	{
		FadeOutAndStopMapBGM(nullptr, FadeTime);
	}

	CurrentCombatMusicStartEvent = CombatMusicStartEvent;
	CurrentCombatMusicStopEvent = CombatMusicStopEvent;

	// 클라이언트에서는 즉시, 그 외에는 페이드 시간에 맞춰 전투 BGM 시작
	float DelayTime = (GetWorld() && GetWorld()->GetNetMode() == NM_Client) ? 0.1f : FadeTime;

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

	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		CurrentCombatMusicStartEvent = nullptr;
		CurrentCombatMusicStopEvent = nullptr;
		return;
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetTargetActorForPlayback(Context);

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

void UGS_AudioManager::FadeOutAndStopMapBGM(AActor* Context, float FadeTime)
{
	if (!bIsMapBGMPlaying)
	{
		return;
	}
	
	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetTargetActorForPlayback(Context);
	
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
	
	// 기존 페이드아웃 타이머가 있다면 취소
	if (GetWorld()->GetTimerManager().IsTimerActive(MapBGMFadeOutTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(MapBGMFadeOutTimerHandle);
	}
	
	GetWorld()->GetTimerManager().SetTimer(MapBGMFadeOutTimerHandle, [this, TargetActor]()
	{
		StopMapBGM(TargetActor);
	}, FadeTime, false);
}

void UGS_AudioManager::FadeInAndStartMapBGM(AActor* Context, float FadeTime)
{
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (!IsAudioProcessingAllowed())
	{
		return;
	}

	// 게임 모드에 따른 조건부 타겟 액터 결정
	AActor* TargetActor = GetTargetActorForPlayback(Context);

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
