#include "Sound/GS_AudioComponentBase.h"
#include "AkAudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "AI/RTS/GS_RTSController.h"
#include "Net/UnrealNetwork.h"

// RTPC 이름을 상수로 정의
const FName UGS_AudioComponentBase::DistanceToPlayerRTPCName = TEXT("Distance_to_Player");
const FName UGS_AudioComponentBase::AttenuationModeRTPCName = TEXT("Attenuation_Mode");

UGS_AudioComponentBase::UGS_AudioComponentBase()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentPlayingID = AK_INVALID_PLAYING_ID;
    ActivePlayingIDs.Empty();
    LastMulticastTime = DefaultInitTime;
    LastDistanceRTPCValue = -1.0f;
    LastRTPCUpdateTime = DefaultInitTime;

    SetIsReplicatedByDefault(true);
}

void UGS_AudioComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // 필요한 경우 하위 클래스에서 추가 리플리케이션 프로퍼티 설정
}

void UGS_AudioComponentBase::BeginPlay()
{
    Super::BeginPlay();
    
    // 초기 Distance Scaling 설정 (TPS 모드 기본값)
    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        const float InitialScalingValue = TPSDistanceScaling;
        AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), InitialScalingValue, 0, GetOwner());
    }
    
    // 거리 체크 타이머 시작 - 성능 최적화된 주기
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(DistanceCheckTimerHandle, this, &UGS_AudioComponentBase::UpdateDistanceRTPC, DistanceCheckInterval, true);
    }
}

void UGS_AudioComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 타이머 중지
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DistanceCheckTimerHandle);
    }
    
    // 모든 활성 사운드 중지
    StopAllActiveSounds();
    
    Super::EndPlay(EndPlayReason);
}

bool UGS_AudioComponentBase::IsRTSMode() const
{
    if (!GetWorld()) 
    {
        return false;
    }
    
    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!LocalPC)
    {
        return false;
    }
    
    bool bIsRTS = Cast<AGS_RTSController>(LocalPC) != nullptr;    
    return bIsRTS;
}

bool UGS_AudioComponentBase::GetListenerLocation(FVector& OutLocation) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    // First try to find a local player controller (for client-side audio)
    APlayerController* LocalPC = nullptr;
    UWorld* World = GetWorld();
    
    for (auto It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->IsLocalController())
        {
            LocalPC = PC;
            break;
        }
    }
    
    // Fallback to PlayerController index 0 if no local controller found
    if (!LocalPC)
    {
        LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    }
    
    if (!LocalPC)
    {
        return false;
    }
    
    if (AGS_RTSController* RTSController = Cast<AGS_RTSController>(LocalPC))
    {
        AActor* ViewTarget = RTSController->GetViewTarget();
        if (ViewTarget && IsValid(ViewTarget))
        {
            OutLocation = ViewTarget->GetActorLocation();
            return true;
        }
    }
    else 
    {
        APawn* PlayerPawn = LocalPC->GetPawn();
        if (PlayerPawn && IsValid(PlayerPawn))
        {
            OutLocation = PlayerPawn->GetActorLocation();
            return true;
        }
    }

    return false;
}

float UGS_AudioComponentBase::GetMaxDistanceForMode(bool bIsRTS) const
{
    return bIsRTS ? RTSMaxDistance : GetMaxAudioDistance();
}

float UGS_AudioComponentBase::GetDistanceScalingForMode(bool bIsRTS) const
{
    return bIsRTS ? RTSDistanceScaling : TPSDistanceScaling;
}

bool UGS_AudioComponentBase::CanSendRPC() const
{
    if (!GetWorld()) return false;
    
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastMulticastTime) >= MinRPCInterval;
}

void UGS_AudioComponentBase::CleanupFinishedSounds()
{
    if (!FAkAudioDevice::Get()) return;
    
    // 완료된 사운드 ID들을 제거
    for (int32 i = ActivePlayingIDs.Num() - 1; i >= 0; --i)
    {
        // Wwise에서 사운드가 여전히 재생 중인지 확인하는 로직을 추가할 수 있음
        // 현재는 잘못된 ID들만 제거
        if (ActivePlayingIDs[i] == AK_INVALID_PLAYING_ID)
        {
            ActivePlayingIDs.RemoveAt(i);
        }
    }
    
    // 배열 크기가 너무 커지지 않도록 제한
    if (ActivePlayingIDs.Num() > MaxActivePlayingIDs)
    {
        ActivePlayingIDs.RemoveAt(0, ActivePlayingIDs.Num() - MaxActivePlayingIDs);
    }
}

void UGS_AudioComponentBase::StopAllActiveSounds()
{
    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        for (AkPlayingID PlayingID : ActivePlayingIDs)
        {
            if (PlayingID != AK_INVALID_PLAYING_ID)
            {
                AkAudioDevice->StopPlayingID(PlayingID);
            }
        }
    }
    
    ActivePlayingIDs.Empty();
    CurrentPlayingID = AK_INVALID_PLAYING_ID;
}

void UGS_AudioComponentBase::RegisterPlayingID(AkPlayingID NewPlayingID)
{
    if (NewPlayingID != AK_INVALID_PLAYING_ID)
    {
        ActivePlayingIDs.Add(NewPlayingID);
        CurrentPlayingID = NewPlayingID;
        
        // 주기적으로 완료된 사운드 정리
        CleanupFinishedSounds();
    }
}

void UGS_AudioComponentBase::UpdateDistanceRTPC()
{
    if (!GetOwner() || !GetWorld()) return;

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    FVector ListenerLocation;
    if (GetListenerLocation(ListenerLocation))
    {
        float DistanceToListener = FVector::Dist(GetOwner()->GetActorLocation(), ListenerLocation);

        if (DistanceToListener <= GetMaxAudioDistance())
        {
            // RTPC 업데이트 최적화 - 거리 차이가 임계값 이상이거나 일정 시간 경과 시만 업데이트
            if (ShouldUpdateRTPC(DistanceToListener, CurrentTime))
            {
                if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
                {
                    AkAudioDevice->SetRTPCValue(*DistanceToPlayerRTPCName.ToString(), DistanceToListener, 0, GetOwner());
                    LastDistanceRTPCValue = DistanceToListener;
                    LastRTPCUpdateTime = CurrentTime;
                }
            }
        }
    }

    // 서버에서만 상태 변경 체크
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        CheckForStateChanges();
    }
}

bool UGS_AudioComponentBase::ShouldUpdateRTPC(float NewDistance, float CurrentTime) const
{
    if (LastDistanceRTPCValue < 0.0f) // 초기화
    {
        return true;
    }
    else if (FMath::Abs(NewDistance - LastDistanceRTPCValue) >= RTPCDistanceThreshold)
    {
        return true; // 거리 변화가 큼
    }
    else if (CurrentTime - LastRTPCUpdateTime >= MinRTPCUpdateInterval)
    {
        return true; // 시간 경과
    }
    
    return false;
}

void UGS_AudioComponentBase::SetDistanceScaling(bool bIsRTS)
{
    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        const float ScalingValue = GetDistanceScalingForMode(bIsRTS);
        AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), ScalingValue, 0, GetOwner());
    }
}