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
		UE_LOG(LogTemp, Warning, TEXT("PlayEvent 호출 실패: Invalid Event or Context."));
		return;
	}
	FOnAkPostEventCallback DummyCallback;
	UAkGameplayStatics::PostEvent(Event, Context, 0, DummyCallback);
}

// === 맵 BGM 관리 시스템 ===

void UGS_AudioManager::StartMapBGM(AActor* Context)
{
	if (!MapBGMEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("MapBGMEvent가 null입니다!"));
		return;
	}

	if (bIsMapBGMPlaying)
	{
		return; // 중복 재생 방지
	}

	// 적절한 Context 찾기 (플레이어 컨트롤러 Pawn으로 설정)
	AActor* TargetActor = nullptr;
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		TargetActor = GetWorld()->GetFirstPlayerController()->GetPawn();
	}
	
	if(!TargetActor && Context)
	{
	    TargetActor = Context;
	}

	if (!TargetActor)
	{
	    UE_LOG(LogTemp, Error, TEXT("StartMapBGM: TargetActor를 찾을 수 없습니다. BGM 재생 불가."));
	    return;
	}

	// 클라이언트별 안전 장치: Wwise에서 이미 재생중인지 확인
	if (FAkAudioDevice::Get())
	{
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
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Wwise AudioDevice를 찾을 수 없습니다!"));
	}
}

void UGS_AudioManager::StopMapBGM(AActor* Context)
{
	if (!bIsMapBGMPlaying)
	{
		return;
	}

	// 적절한 Context 찾기
	AActor* TargetActor = nullptr;
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		TargetActor = GetWorld()->GetFirstPlayerController()->GetPawn();
	}
	if (!TargetActor && Context)
	{
	    TargetActor = Context;
	}

	// Wwise Stop 이벤트를 사용한 부드러운 정지
	if (MapBGMStopEvent && TargetActor)
	{
		FOnAkPostEventCallback DummyCallback;
		UAkGameplayStatics::PostEvent(MapBGMStopEvent, TargetActor, 0, DummyCallback);
	}
	else if (!MapBGMStopEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("MapBGMStopEvent가 null입니다! 강제 정지로 대체합니다."));
		
		// Stop 이벤트가 없다면 기존 방식으로 강제 정지
		if (MapBGMVolumeRTPC && TargetActor)
		{
			SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);
		}
		if (TargetActor)
		{
			UAkGameplayStatics::StopActor(TargetActor);
		}
	}
	else if (!TargetActor)
	{
		// TargetActor가 없는 경우 전역 정지 시도
		if (MapBGMStopEvent)
		{
			FOnAkPostEventCallback DummyCallback;
			UAkGameplayStatics::PostEvent(MapBGMStopEvent, nullptr, 0, DummyCallback);
		}
	}
	
	bIsMapBGMPlaying = false;
}

void UGS_AudioManager::FadeMapBGMForCombat(AActor* Context, float FadeTime)
{
	if (!Context || !bIsMapBGMPlaying)
	{
		return;
	}

	// 맵 BGM 볼륨을 30%로 감소
	float CombatVolume = 0.3f;
	SetRTPCValue(MapBGMVolumeRTPC, CombatVolume, Context, FadeTime * 1000.0f);
}

void UGS_AudioManager::RestoreMapBGMFromCombat(AActor* Context, float FadeTime)
{
	if (!Context || !bIsMapBGMPlaying)
	{
		return;
	}

	// 맵 BGM 볼륨을 100%로 복원
	SetRTPCValue(MapBGMVolumeRTPC, 1.0f, Context, FadeTime * 1000.0f);
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

	// 기존에 재생 중인 전투 음악이 있다면 정지
	if (CurrentCombatMusicStartEvent && CurrentCombatMusicStopEvent)
	{
		UAkGameplayStatics::PostEvent(CurrentCombatMusicStopEvent, Context, 0, FOnAkPostEventCallback());
	}
	else if (CurrentCombatMusicStartEvent)
	{
		UAkGameplayStatics::StopActor(Context);
	}

	// 멀티플레이어 환경에서는 맵 BGM을 즉시 강제 정지
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone)
	{
		if (MapBGMVolumeRTPC)
		{
			SetRTPCValue(MapBGMVolumeRTPC, 0.0f, Context, 0.0f);
		}
		StopMapBGM(nullptr);
	}
	else
	{
		FadeOutAndStopMapBGM(Context, FadeTime);
	}

	CurrentCombatMusicStartEvent = CombatMusicStartEvent;
	CurrentCombatMusicStopEvent = CombatMusicStopEvent;

	float DelayTime = (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone) ? 0.1f : FadeTime;

	FTimerHandle CombatBGMHandle;
	GetWorld()->GetTimerManager().SetTimer(CombatBGMHandle, [this, Context, CombatMusicStartEvent]()
	{
		if (Context && CombatMusicStartEvent)
		{
			UAkGameplayStatics::PostEvent(CombatMusicStartEvent, Context, 0, FOnAkPostEventCallback());
		}
	}, DelayTime, false);
}

void UGS_AudioManager::EndCombatSequence(AActor* Context, float FadeTime)
{
	if (!Context)
	{
		return;
	}

	// 현재 재생 중인 전투 BGM 정지
	if (CurrentCombatMusicStartEvent && CurrentCombatMusicStopEvent)
	{
		UAkGameplayStatics::PostEvent(CurrentCombatMusicStopEvent, Context, 0, FOnAkPostEventCallback());
	}
	else if (CurrentCombatMusicStartEvent)
	{
		UAkGameplayStatics::StopActor(Context);
	}

	CurrentCombatMusicStartEvent = nullptr;
	CurrentCombatMusicStopEvent = nullptr;

	// 맵 BGM 페이드인 후 재생
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
	
	UE_LOG(LogTemp, Warning, TEXT("=== BGM 상태 정보 ==="));
	UE_LOG(LogTemp, Warning, TEXT("네트워크 모드: %s"), *NetModeString);
	UE_LOG(LogTemp, Warning, TEXT("맵 BGM 재생 중: %s"), bIsMapBGMPlaying ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Warning, TEXT("MapBGMEvent: %s"), MapBGMEvent ? TEXT("로드됨") : TEXT("null"));
	UE_LOG(LogTemp, Warning, TEXT("MapBGMStopEvent: %s"), MapBGMStopEvent ? TEXT("로드됨") : TEXT("null"));
	UE_LOG(LogTemp, Warning, TEXT("MapBGMVolumeRTPC: %s"), MapBGMVolumeRTPC ? TEXT("로드됨") : TEXT("null"));
	UE_LOG(LogTemp, Warning, TEXT("Wwise AudioDevice: %s"), FAkAudioDevice::Get() ? TEXT("활성") : TEXT("비활성"));
	UE_LOG(LogTemp, Warning, TEXT("=================="));
}

void UGS_AudioManager::FadeOutAndStopMapBGM(AActor* Context, float FadeTime)
{
	if (!bIsMapBGMPlaying)
	{
		return;
	}
	
	if (!MapBGMVolumeRTPC)
	{
		StopMapBGM(Context);
		return;
	}
	
	SetRTPCValue(MapBGMVolumeRTPC, 0.0f, Context, FadeTime * 1000.0f);
	
	if (FadeTime <= 0.0f)
	{
		StopMapBGM(Context);
		return;
	}
	
	FTimerHandle FadeOutHandle;
	GetWorld()->GetTimerManager().SetTimer(FadeOutHandle, [this, Context]()
	{
		StopMapBGM(Context);
	}, FadeTime, false);
}

void UGS_AudioManager::FadeInAndStartMapBGM(AActor* Context, float FadeTime)
{
	// 적절한 Context 찾기
	AActor* TargetActor = Context;
	if (!TargetActor && GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		TargetActor = GetWorld()->GetFirstPlayerController()->GetPawn();
	}
	
	// BGM 재생 시작
	StartMapBGM(TargetActor);
	
	// 볼륨 페이드인
	if (MapBGMVolumeRTPC && bIsMapBGMPlaying)
	{
		SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);
		
		FTimerHandle VolumeHandle;
		GetWorld()->GetTimerManager().SetTimer(VolumeHandle, [this, TargetActor, FadeTime]()
		{
			if (MapBGMVolumeRTPC && bIsMapBGMPlaying)
			{
				SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, FadeTime * 1000.0f);
			}
		}, 0.1f, false);
	}
}

// === 콘솔 명령어 구현 ===

void UGS_AudioManager::DebugMapBGM()
{
	UE_LOG(LogTemp, Warning, TEXT("콘솔 명령어: DebugMapBGM 실행"));
	LogCurrentBGMStatus();
}

void UGS_AudioManager::RestartMapBGM()
{
	UE_LOG(LogTemp, Warning, TEXT("콘솔 명령어: RestartMapBGM 실행"));
	
	// 기존 BGM 정지
	StopMapBGM(nullptr);
	
	// 짧은 지연 후 재시작
	FTimerHandle RestartHandle;
	GetWorld()->GetTimerManager().SetTimer(RestartHandle, [this]()
	{
		StartMapBGM(nullptr);
	}, 0.5f, false);
}

void UGS_AudioManager::StopMapBGMDebug()
{
	UE_LOG(LogTemp, Warning, TEXT("콘솔 명령어: StopMapBGMDebug 실행"));
	StopMapBGM(nullptr);
}
