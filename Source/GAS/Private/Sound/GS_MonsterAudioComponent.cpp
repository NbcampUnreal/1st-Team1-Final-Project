#include "Sound/GS_MonsterAudioComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Component/GS_StatComp.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
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
    // 타이머 안전 정리 (SafeClearTimer 패턴)
    if (IdleSoundTimer.IsValid())
    {
        UWorld* World = GetWorld();
        if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
        {
            World->GetTimerManager().ClearTimer(IdleSoundTimer);
        }
        IdleSoundTimer.Invalidate();
    }

    if (CombatSoundTimer.IsValid())
    {
        UWorld* World = GetWorld();
        if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
        {
            World->GetTimerManager().ClearTimer(CombatSoundTimer);
        }
        CombatSoundTimer.Invalidate();
    }

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
    // 컴포넌트 유효성 검증
    if (!IsValid(this))
    {
        return;
    }

    // 월드 컨텍스트 유효성 검증
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
    {
        return;
    }

    // 오너 유효성 검증
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->HasAuthority())
    {
        return;
    }

    if (!OwnerMonster)
    {
        return;
    }

    if (!CanSendRPC())
    {
        return;
    }

    if (!bForcePlay)
    {
        float Interval;
        if (SoundType == EMonsterAudioState::Idle) Interval = IdleSoundInterval;
        else if (SoundType == EMonsterAudioState::Combat) Interval = CombatSoundInterval;
        else Interval = 1.0f;

        float CurrentTime = World->GetTimeSeconds();
        if ((CurrentTime - ServerLastBroadcastTime.FindOrAdd(SoundType, 0.0f)) < Interval)
        {
            return;
        }
        ServerLastBroadcastTime.Emplace(SoundType, CurrentTime);
    }

    LastMulticastTime = World->GetTimeSeconds();
    Multicast_TriggerSound(SoundType, bForcePlay);
}

void UGS_MonsterAudioComponent::Multicast_TriggerSound_Implementation(EMonsterAudioState SoundTypeToTrigger, bool bIsImmediate)
{
    if (!OwnerMonster)
    {
        return;
    }

    // 죽음 사운드는 거리/시야각 체크 없이 항상 재생 (bSkipViewFrustumCheck = true)
    // 다른 사운드는 정상 체크 수행 (bSkipViewFrustumCheck = false)
    const bool bSkipCheck = (SoundTypeToTrigger == EMonsterAudioState::Death);

    // 통합 체크 및 Distance Scaling 설정 (중복 로직 제거)
    if (!PrepareMulticastSound(OwnerMonster, bSkipCheck))
    {
        return;
    }

    UAkAudioEvent* SoundEvent = GetSoundEvent(SoundTypeToTrigger);
    if (!SoundEvent)
    {
        return;
    }

    if (!bIsImmediate)
    {
        float Interval;
        if (SoundTypeToTrigger == EMonsterAudioState::Idle) Interval = IdleSoundInterval;
        else if (SoundTypeToTrigger == EMonsterAudioState::Combat) Interval = CombatSoundInterval;
        else Interval = 1.0f;

        UWorld* World = GetWorld();
        float CurrentTime = World->GetTimeSeconds();
        if ((CurrentTime - LocalLastSoundPlayTimes.FindOrAdd(SoundTypeToTrigger, 0.0f)) < (Interval * LocalSoundCooldownMultiplier))
        {
            return;
        }
        LocalLastSoundPlayTimes.Emplace(SoundTypeToTrigger, CurrentTime);
    }

    AkPlayingID NewPlayingID = UAkGameplayStatics::PostEvent(SoundEvent, OwnerMonster, 0, FOnAkPostEventCallback());
    RegisterPlayingID(NewPlayingID);

    // Combat 사운드인 경우 PlayingID 추적
    /*if (SoundTypeToTrigger == EMonsterAudioState::Combat)
    {
        CurrentCombatPlayingID = NewPlayingID;
    }*/
}

void UGS_MonsterAudioComponent::PlayHurtSound()
{
    // 컴포넌트 유효성 검증
    if (!IsValid(this))
    {
        return;
    }

    // 월드 컨텍스트 유효성 검증
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
    {
        return;
    }

    // 오너 유효성 검증
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->HasAuthority())
    {
        return;
    }

    SetMonsterAudioState(EMonsterAudioState::Hurt);
    PlaySound(EMonsterAudioState::Hurt, true);
}

void UGS_MonsterAudioComponent::PlayDeathSound()
{
    // 컴포넌트 유효성 검증
    if (!IsValid(this))
    {
        return;
    }

    // 월드 컨텍스트 유효성 검증
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
    {
        return;
    }

    // 오너 유효성 검증
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->HasAuthority())
    {
        return;
    }

    // 죽음 사운드 재생 전에 현재 재생 중인 모든 사운드 중단
    StopAllActiveSounds();

    SetMonsterAudioState(EMonsterAudioState::Death);
    PlaySound(EMonsterAudioState::Death, true);
}

void UGS_MonsterAudioComponent::PlaySwingSound()
{
    // 컴포넌트 유효성 검증
    if (!IsValid(this))
    {
        return;
    }

    // 월드 컨텍스트 유효성 검증
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
    {
        return;
    }

    // 오너 유효성 검증
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->HasAuthority())
    {
        return;
    }

    if (!CanSendRPC())
    {
        return;
    }

    const float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - ServerLastSwingBroadcastTime < SwingResetTime)
    {
        return;
    }
    ServerLastSwingBroadcastTime = CurrentTime;
    LastMulticastTime = CurrentTime;

    Multicast_PlaySwingSound();
}

/*void UGS_MonsterAudioComponent::StopSwingSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    Multicast_StopSwingSound();
}*/

/*void UGS_MonsterAudioComponent::StopCombatSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    Multicast_StopCombatSound();
}*/

AGS_Seeker* UGS_MonsterAudioComponent::FindNearestSeeker() const
{
    if (!GetWorld() || !OwnerMonster)
    {
        return nullptr;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    // 1. 캐시 유효성 체크
    if ((CurrentTime - LastSeekerSearchTime) < SeekerCacheValidDuration)
    {
        // 캐시된 Seeker가 여전히 유효한지 확인
        if (CachedNearestSeeker.IsValid())
        {
            // 캐시된 Seeker가 여전히 AlertDistance 안에 있는지 확인
            const float DistanceSq = FVector::DistSquared(
                OwnerMonster->GetActorLocation(),
                CachedNearestSeeker->GetActorLocation()
            );
            
            if (DistanceSq <= FMath::Square(AudioConfig.AlertDistance))
            {
                return CachedNearestSeeker.Get();
            }
        }
    }

    // 2. 캐시가 만료되었거나 유효하지 않음 - 새로 검색
    AGS_Seeker* NearestSeeker = nullptr;
    float MinDistanceSq = FLT_MAX;
    const FVector MonsterLocation = OwnerMonster->GetActorLocation();

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AudioConfig.AlertDistance);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerMonster);

    const bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
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
            // [주석처리] 죽은 시커 제외 로직
            if (Seeker /*&& !Seeker->IsDead()*/)  // 죽은 시커는 제외
            {
                const float DistanceSq = FVector::DistSquared(MonsterLocation, Seeker->GetActorLocation());
                if (DistanceSq < MinDistanceSq)
                {
                    MinDistanceSq = DistanceSq;
                    NearestSeeker = Seeker;
                }
            }
        }
    }

    // 3. 캐시 업데이트
    CachedNearestSeeker = NearestSeeker;
    LastSeekerSearchTime = CurrentTime;

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
    // 통합 체크 및 Distance Scaling 설정
    if (!PrepareMulticastSound(OwnerMonster, false))
    {
        return;
    }

    // 로컬 쿨다운 체크
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LocalLastSwingPlayTime < SwingResetTime * LocalSoundCooldownMultiplier)
    {
        return;
    }
    LocalLastSwingPlayTime = CurrentTime;

    // 모드별 사운드 선택 및 재생
    UAkAudioEvent* SoundToPlay = SelectSoundEventByMode(SwingSound, RTS_SwingSound);
    if (SoundToPlay)
    {
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerMonster, 0, FOnAkPostEventCallback());
        RegisterPlayingID(SwingPlayingID);
    }
}

/*void UGS_MonsterAudioComponent::Multicast_StopSwingSound_Implementation()
{
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }

    if (!OwnerMonster || !GetWorld())
    {
        return;
    }

    // 현재 재생 중인 스윙 사운드만 선별적으로 중단
    if (CurrentSwingPlayingID != 0 && FAkAudioDevice::Get())
    {
        FAkAudioDevice::Get()->StopPlayingID(CurrentSwingPlayingID);
        CurrentSwingPlayingID = 0;
    }
}*/

/*void UGS_MonsterAudioComponent::Multicast_StopCombatSound_Implementation()
{
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }

    if (!OwnerMonster || !GetWorld())
    {
        return;
    }

    // Combat 사운드 타이머 먼저 중단하여 새로운 Combat 사운드 재생 방지
    StopSoundTimer();

    // 현재 재생 중인 모든 Combat 사운드를 중단 (반복 재생으로 여러 개가 있을 수 있음)
    if (OwnerMonster)
    {
        // Combat 사운드 이벤트를 사용해서 Stop 이벤트 호출 (만약 Stop 이벤트가 있다면)
        UAkAudioEvent* CombatSoundToStop = GetSoundEvent(EMonsterAudioState::Combat);
        if (CombatSoundToStop)
        {
            StopAllActiveSounds();
        }
    }

    // PlayingID 초기화
    CurrentCombatPlayingID = 0;
}*/

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