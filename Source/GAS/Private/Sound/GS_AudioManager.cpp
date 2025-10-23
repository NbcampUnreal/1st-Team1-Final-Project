// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_AudioManager.h"
#include "Sound/GS_UIAudioSystem.h"
#include "AkAudioDevice.h"
#include "UObject/UObjectGlobals.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"

UGS_AudioManager::UGS_AudioManager()
{
	// 맵 BGM 상태 초기화
	bIsMapBGMPlaying = false;

	// 전투 BGM 상태 초기화
	bIsCombatMusicPlaying = false;

	// BGM 볼륨 초기화
	CurrentBGMVolume = 1.0f;

	// 포인터 멤버 초기화
	UIAudio = nullptr;

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

	// 네이티브 BGM 사운드 클래스 로드
	static ConstructorHelpers::FObjectFinder<USoundClass> BGMSoundClassFinder(TEXT("/Game/WwiseAudio/SC_BGM.SC_BGM"));
	if (BGMSoundClassFinder.Succeeded())
	{
		BGMSoundClass = BGMSoundClassFinder.Object;
	}
	else
	{
		BGMSoundClass = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[AudioManager] SC_BGM을 찾을 수 없습니다. 네이티브 오디오 볼륨 조절이 비활성화됩니다."));
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
		UE_LOG(LogTemp, Warning, TEXT("[AudioManager] SM_BGM을 찾을 수 없습니다. 네이티브 오디오 볼륨 조절이 비활성화됩니다."));
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

	// 오디오 에셋 유효성 검사
	if (!ValidateAudioAssets())
	{
		UE_LOG(LogTemp, Warning, TEXT("일부 오디오 에셋이 누락되었지만 시스템을 계속 진행합니다."));
	}

	// 맵 전환 시 BGM 정지를 위한 델리게이트 바인딩
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UGS_AudioManager::OnPreLoadMap);
}

void UGS_AudioManager::Deinitialize()
{
	// 델리게이트 해제
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	
	// 메모리 해제 처리
	UIAudio = nullptr;

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
	
	// 3. RTPC를 기본값(100)으로 리셋 (다음 맵에서 맵 BGM이 정상 재생되도록)
	if (MapBGMVolumeRTPC)
	{
		SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, 0.0f);
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
		UE_LOG(LogTemp, Error, TEXT("[AudioManager] SetRTPCValue: RTPC가 nullptr입니다!"));
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

// === 통합 전투 시스템 ===

void UGS_AudioManager::StopCurrentCombatMusic(AActor* Context)
{
	if (!Context)
	{
		return;
	}

	// 기존 전투 음악이 없으면 조기 종료
	if (!CurrentCombatMusicStartEvent)
	{
		return;
	}

	// StopEvent가 있으면 사용, 없으면 Actor 전체 정지
	if (CurrentCombatMusicStopEvent)
	{
		UAkGameplayStatics::PostEvent(CurrentCombatMusicStopEvent, Context, 0, FOnAkPostEventCallback());
	}
	else
	{
		UAkGameplayStatics::StopActor(Context);
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
		UE_LOG(LogTemp, Log, TEXT("[AudioManager] 전투 BGM이 이미 재생 중입니다. 중복 재생을 방지합니다. (Event: %s)"),
			*CombatMusicStartEvent->GetName());
		return;
	}

	// 다른 전투 BGM이 재생 중이면 교체 허용 (다른 몬스터 종류)
	if (bIsCombatMusicPlaying && CurrentCombatMusicStartEvent != CombatMusicStartEvent)
	{
		UE_LOG(LogTemp, Log, TEXT("[AudioManager] 전투 BGM 교체: %s → %s"),
			CurrentCombatMusicStartEvent ? *CurrentCombatMusicStartEvent->GetName() : TEXT("None"),
			*CombatMusicStartEvent->GetName());
		// 기존 BGM 정지 후 새 BGM 재생 (아래 로직 계속 진행)
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
	if (Context && CombatMusicStartEvent)
	{
		UAkGameplayStatics::PostEvent(CombatMusicStartEvent, Context, 0, FOnAkPostEventCallback());
		bIsCombatMusicPlaying = true;  // 전투 BGM 재생 상태로 설정
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

	if (StopEventToUse)
	{
		UAkGameplayStatics::PostEvent(StopEventToUse, TargetActor, 0, FOnAkPostEventCallback());
	}
	else if (CurrentCombatMusicStartEvent && TargetActor)
	{
		// StopEvent가 없으면 Actor 전체 정지
		UAkGameplayStatics::StopActor(TargetActor);
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

	// 기존 타이머 취소
	if (GetWorld()->GetTimerManager().IsTimerActive(MapBGMFadeOutTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(MapBGMFadeOutTimerHandle);
	}

	// FadeTime 후 정지
	GetWorld()->GetTimerManager().SetTimer(MapBGMFadeOutTimerHandle,
		[this, TargetActor]()
		{
			StopMapBGM(TargetActor);
		},
		FadeTime, false);
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

	// 기존 타이머 취소
	if (GetWorld()->GetTimerManager().IsTimerActive(MapBGMFadeInTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(MapBGMFadeInTimerHandle);
	}

	// 볼륨 0으로 설정 후 페이드인
	SetRTPCValue(MapBGMVolumeRTPC, 0.0f, TargetActor, 0.0f);

	GetWorld()->GetTimerManager().SetTimer(MapBGMFadeInTimerHandle,
		[this, TargetActor, FadeTime]()
		{
			if (MapBGMVolumeRTPC && bIsMapBGMPlaying)
			{
				SetRTPCValue(MapBGMVolumeRTPC, 1.0f, TargetActor, FadeTime * 1000.0f);
			}
		},
		0.1f, false);
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
