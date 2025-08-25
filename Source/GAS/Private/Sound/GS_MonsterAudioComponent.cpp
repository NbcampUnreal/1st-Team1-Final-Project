#include "Sound/GS_MonsterAudioComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Component/GS_StatComp.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "AI/RTS/GS_RTSController.h"

UGS_MonsterAudioComponent::UGS_MonsterAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentAudioState = EMonsterAudioState::Idle;
    PreviousAudioState = EMonsterAudioState::Idle;
    
    // 기본 설정값
    AudioConfig.AlertDistance = 800.0f;
    AudioConfig.MaxAudioDistance = 2000.0f; // TPS 모드 기준 (20미터)
    IdleSoundInterval = 6.0f;
    CombatSoundInterval = 1.0f;
}

void UGS_MonsterAudioComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGS_MonsterAudioComponent, CurrentAudioState);
}

void UGS_MonsterAudioComponent::OnRep_CurrentAudioState()
{
    if (OwnerMonster && GetWorld() && GetWorld()->IsNetMode(NM_Client))
    {
        UpdateSoundTimer();
    }
    PreviousAudioState = CurrentAudioState;
}

void UGS_MonsterAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerMonster = Cast<AGS_Monster>(GetOwner());
    if (!OwnerMonster)
    {
        return;
    }

    // 통일된 RTPC 시스템으로 초기화
    InitializeAudioRTPCs();
    
    if (GetOwner()->HasAuthority())
    {
        StartSoundTimer();
    }
    PreviousAudioState = CurrentAudioState; 
    
}

void UGS_MonsterAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSoundTimer();
    
    
    Super::EndPlay(EndPlayReason);
}

void UGS_MonsterAudioComponent::InitializeAudioRTPCs()
{
    // 기본 RTPC 초기화
    Super::InitializeAudioRTPCs();
    
    // 몬스터의 Distance Scaling 초기값을 1.0f (TPS 100%)로 수정
    SetUnifiedRTPCValue(AttenuationModeRTPC, 1.0f); // 1.0f = TPS 100% (20m)
}

void UGS_MonsterAudioComponent::SetMonsterAudioState(EMonsterAudioState NewState)
{
    
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (CurrentAudioState == NewState)
        return;
        
    PreviousAudioState = CurrentAudioState;
    CurrentAudioState = NewState;
    
    UpdateSoundTimer();
}

void UGS_MonsterAudioComponent::PlaySound(EMonsterAudioState SoundType, bool bForcePlay)
{
    if (!OwnerMonster || !GetOwner()->HasAuthority())
        return;

    if (!CanSendRPC())
        return;

    if (!bForcePlay)
    {
        float Interval;
        if (SoundType == EMonsterAudioState::Idle) Interval = IdleSoundInterval;
        else if (SoundType == EMonsterAudioState::Combat) Interval = CombatSoundInterval;
        else Interval = 1.0f;

        float CurrentTime = GetWorld()->GetTimeSeconds();
        if ((CurrentTime - ServerLastBroadcastTime.FindOrAdd(SoundType, 0.0f)) < Interval)
        {
            return;
        }
        ServerLastBroadcastTime.Emplace(SoundType, CurrentTime);
    }
    
    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_TriggerSound(SoundType, bForcePlay);
}

void UGS_MonsterAudioComponent::Multicast_TriggerSound_Implementation(EMonsterAudioState SoundTypeToTrigger, bool bIsImmediate)
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
    if (!OwnerMonster || !GetWorld())
    {
        return;
    }

    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    // RTS 모드와 TPS 모드에 따른 거리 체크
    const bool bRTS = IsRTSMode();
    const float MaxDistance = GetMaxDistanceForMode(bRTS);
    
    float DistanceToListener = FVector::Dist(OwnerMonster->GetActorLocation(), ListenerLocation);
    
    // RTS 모드에서는 화면 시야각 기반 체크, TPS 모드에서는 거리 기반 체크
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!IsInViewFrustum(OwnerMonster->GetActorLocation()))
        {
            return;
        }
    }
    else
    {
        // TPS 모드: 기존 거리 기반 체크
        if (DistanceToListener > MaxDistance)
        {
            return;
        }
    }
    
    UAkAudioEvent* SoundEvent = GetSoundEvent(SoundTypeToTrigger);
    if (!SoundEvent)
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정 (통일된 시스템 사용)
    SetDistanceScaling(bRTS);

    if (!bIsImmediate)
    {
        float Interval;
        if (SoundTypeToTrigger == EMonsterAudioState::Idle) Interval = IdleSoundInterval;
        else if (SoundTypeToTrigger == EMonsterAudioState::Combat) Interval = CombatSoundInterval;
        else Interval = 1.0f;

        float CurrentTime = GetWorld()->GetTimeSeconds();
        if ((CurrentTime - LocalLastSoundPlayTimes.FindOrAdd(SoundTypeToTrigger, 0.0f)) < (Interval * LocalSoundCooldownMultiplier)) 
        {
            return; 
        }
        LocalLastSoundPlayTimes.Emplace(SoundTypeToTrigger, CurrentTime);
    }
    
    AkPlayingID NewPlayingID = UAkGameplayStatics::PostEvent(SoundEvent, OwnerMonster, 0, FOnAkPostEventCallback());
    RegisterPlayingID(NewPlayingID);
}

void UGS_MonsterAudioComponent::PlayHurtSound()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        SetMonsterAudioState(EMonsterAudioState::Hurt);
        PlaySound(EMonsterAudioState::Hurt, true);
    }
}

void UGS_MonsterAudioComponent::PlayDeathSound()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        SetMonsterAudioState(EMonsterAudioState::Death);
        PlaySound(EMonsterAudioState::Death, true);
    }
}

void UGS_MonsterAudioComponent::PlaySwingSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    if (!CanSendRPC())
        return;

    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - ServerLastSwingBroadcastTime < SwingResetTime)
    {
        return;
    }
    ServerLastSwingBroadcastTime = CurrentTime;
    LastMulticastTime = CurrentTime;

    Multicast_PlaySwingSound();
}

AGS_Seeker* UGS_MonsterAudioComponent::FindNearestSeeker() const
{
    if (!GetWorld() || !OwnerMonster) return nullptr;

    AGS_Seeker* NearestSeeker = nullptr;
    float MinDistanceSq = FLT_MAX;
    FVector MonsterLocation = OwnerMonster->GetActorLocation();

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AudioConfig.AlertDistance);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerMonster);

    bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        MonsterLocation,
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        QueryParams
    );

    if (bHasOverlap)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            AGS_Seeker* Seeker = Cast<AGS_Seeker>(Result.GetActor());
            if (Seeker)
            {
                float DistanceSq = FVector::DistSquared(MonsterLocation, Seeker->GetActorLocation());
                if (DistanceSq < MinDistanceSq)
                {
                    MinDistanceSq = DistanceSq;
                    NearestSeeker = Seeker;
                }
            }
        }
    }

    return NearestSeeker;
}

float UGS_MonsterAudioComponent::CalculateDistanceToNearestSeeker() const
{
    AGS_Seeker* NearestSeeker = FindNearestSeeker();
    if (!NearestSeeker || !OwnerMonster)
        return -1.0f;
    
    return FVector::Dist(OwnerMonster->GetActorLocation(), NearestSeeker->GetActorLocation());
}

void UGS_MonsterAudioComponent::PlayIdleSound()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        PlaySound(EMonsterAudioState::Idle, false);
    }
}

void UGS_MonsterAudioComponent::PlayCombatSound()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        PlaySound(EMonsterAudioState::Combat, false);
    }
}

void UGS_MonsterAudioComponent::Multicast_PlaySwingSound_Implementation()
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
    if (!OwnerMonster || !GetWorld())
    {
        return;
    }

    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    const bool bRTS = IsRTSMode();
    const float MaxDistance = GetMaxDistanceForMode(bRTS);
    
    const float DistanceToListener = FVector::Dist(OwnerMonster->GetActorLocation(), ListenerLocation);
    
    // RTS 모드에서는 화면 시야각 기반 체크, TPS 모드에서는 거리 기반 체크
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!IsInViewFrustum(OwnerMonster->GetActorLocation()))
        {
            return;
        }
    }
    else
    {
        // TPS 모드: 기존 거리 기반 체크
        if (DistanceToListener > MaxDistance)
        {
            return;
        }
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float LastTime = LocalLastSwingPlayTime;
    if (CurrentTime - LastTime < SwingResetTime * LocalSoundCooldownMultiplier)
    {
        return;
    }
    LocalLastSwingPlayTime = CurrentTime;

    UAkAudioEvent* SoundToPlay = bRTS ? RTS_SwingSound : SwingSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정 (통일된 시스템 사용)
        SetDistanceScaling(bRTS);
        
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerMonster, 0, FOnAkPostEventCallback());
        RegisterPlayingID(SwingPlayingID);
    }
}

void UGS_MonsterAudioComponent::PlayRTSCommandSound(ERTSCommandSoundType CommandType)
{
    UAkAudioEvent* SoundToPlay = nullptr;
    switch(CommandType)
    {
    case ERTSCommandSoundType::Selection:
        SoundToPlay = SelectionClickSound;
        break;
    case ERTSCommandSoundType::Move:
        SoundToPlay = RTSMoveCommandSound;
        break;
    case ERTSCommandSoundType::Attack:
        SoundToPlay = RTSAttackCommandSound ? RTSAttackCommandSound : RTSMoveCommandSound;
        break;
    case ERTSCommandSoundType::Death:
        // Death 커맨드는 RTS_DeathSound 사용 (AudioConfig에서)
        SoundToPlay = AudioConfig.RTS_DeathSound;
        break;
    }

    if (SoundToPlay)
    {
        AkPlayingID CommandPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(CommandPlayingID);
    }
}

void UGS_MonsterAudioComponent::StartSoundTimer()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld())
        return;

    StopSoundTimer();
    
    float Interval;
    FTimerDelegate TimerDelegate;

    switch (CurrentAudioState)
    {
        case EMonsterAudioState::Idle:
            Interval = IdleSoundInterval;
            TimerDelegate.BindUObject(this, &UGS_MonsterAudioComponent::PlayIdleSound);
            GetWorld()->GetTimerManager().SetTimer(IdleSoundTimer, TimerDelegate, Interval, true);
            break;
            
        case EMonsterAudioState::Combat:
            Interval = CombatSoundInterval;
            TimerDelegate.BindUObject(this, &UGS_MonsterAudioComponent::PlayCombatSound);
            GetWorld()->GetTimerManager().SetTimer(CombatSoundTimer, TimerDelegate, Interval, true);
            break;
            
        default: // Hurt, Death 등은 타이머로 소리내지 않음
            break;
    }
}

void UGS_MonsterAudioComponent::StopSoundTimer()
{
    if (!GetWorld()) return;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    if (IdleSoundTimer.IsValid())
        TimerManager.ClearTimer(IdleSoundTimer);
    if (CombatSoundTimer.IsValid())
        TimerManager.ClearTimer(CombatSoundTimer);
}

void UGS_MonsterAudioComponent::UpdateSoundTimer()
{
    StartSoundTimer();
}

UAkAudioEvent* UGS_MonsterAudioComponent::GetSoundEvent(EMonsterAudioState SoundType) const
{
    const bool bRTS = IsRTSMode();

    switch (SoundType)
    {
        case EMonsterAudioState::Idle:    return bRTS ? nullptr : AudioConfig.IdleSound;
        case EMonsterAudioState::Combat:  return bRTS && AudioConfig.RTS_CombatSound ? AudioConfig.RTS_CombatSound : AudioConfig.CombatSound;
        case EMonsterAudioState::Hurt:    return bRTS && AudioConfig.RTS_HurtSound ? AudioConfig.RTS_HurtSound : AudioConfig.HurtSound;
        case EMonsterAudioState::Death:   return bRTS && AudioConfig.RTS_DeathSound ? AudioConfig.RTS_DeathSound : AudioConfig.DeathSound;
        default:
            return nullptr;
    }
}

float UGS_MonsterAudioComponent::GetMaxAudioDistance() const
{
    return AudioConfig.MaxAudioDistance;
}

void UGS_MonsterAudioComponent::CheckForStateChanges()
{
    if (!OwnerMonster || !GetOwner() || !GetOwner()->HasAuthority() || !GetWorld()) 
        return;
    
    if (CurrentAudioState == EMonsterAudioState::Death)
    {
        return; 
    }
    
    if (OwnerMonster->GetStatComp() && IsValid(OwnerMonster->GetStatComp()))
    {
        if (OwnerMonster->GetStatComp()->GetCurrentHealth() <= 0.0f)
        {
            if (CurrentAudioState != EMonsterAudioState::Death)
            {
                SetMonsterAudioState(EMonsterAudioState::Death); 
            }
            return; 
        }
    }
    
    if (CurrentAudioState == EMonsterAudioState::Hurt) return;
    
    float DistanceToSeeker = CalculateDistanceToNearestSeeker();
    
    if (DistanceToSeeker >= 0.0f) 
    {
        if (DistanceToSeeker <= AudioConfig.AlertDistance && CurrentAudioState == EMonsterAudioState::Idle)
        {
            SetMonsterAudioState(EMonsterAudioState::Combat);
        }
        else if (DistanceToSeeker > AudioConfig.AlertDistance && CurrentAudioState == EMonsterAudioState::Combat)
        {
            SetMonsterAudioState(EMonsterAudioState::Idle);
        }
    }
    else if (CurrentAudioState == EMonsterAudioState::Combat)
    {
        SetMonsterAudioState(EMonsterAudioState::Idle);
    }
} 