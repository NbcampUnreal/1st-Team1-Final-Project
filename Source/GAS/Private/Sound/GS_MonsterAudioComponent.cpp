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

// 몬스터 전용 RTPC 이름을 상수로 정의
const FName UGS_MonsterAudioComponent::MonsterVariantRTPCName = TEXT("Monster_Variant");

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
    
    MonsterSoundVariant = 1;
    
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

    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        if (MonsterSoundVariant > 0) 
        {
            AkAudioDevice->SetRTPCValue(*MonsterVariantRTPCName.ToString(), MonsterSoundVariant, 0, OwnerMonster);
        }
        
        // 초기 Distance Scaling 설정 (TPS 모드 기본값)
        const float InitialScalingValue = 0.0f; // TPS 모드 기본값 (100% = 20m)
        AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), InitialScalingValue, 0, OwnerMonster);
    }
    
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

    // RTS 모드에 따른 Distance Scaling 설정
    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        const float ScalingValue = GetDistanceScalingForMode(bRTS);
        AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), ScalingValue, 0, OwnerMonster);
    }

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
    if (NewPlayingID != AK_INVALID_PLAYING_ID)
    {
        ActivePlayingIDs.Add(NewPlayingID);
        CurrentPlayingID = NewPlayingID;
        
        CleanupFinishedSounds();
    }
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
        if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
        {
            const float ScalingValue = bRTS ? 1.0f : 0.0f; // RTS: 1.0 (400% = 80m), TPS: 0.0 (100% = 20m)
            AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), ScalingValue, 0, OwnerMonster);
        }
        
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerMonster, 0, FOnAkPostEventCallback());
        if (SwingPlayingID != AK_INVALID_PLAYING_ID)
        {
            ActivePlayingIDs.Add(SwingPlayingID);
        }
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
        if (CommandPlayingID != AK_INVALID_PLAYING_ID)
        {
            ActivePlayingIDs.Add(CommandPlayingID);
        }
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


void UGS_MonsterAudioComponent::DrawDebugInfo() const
{
    if (!OwnerMonster)
        return;
    
    FVector MonsterLocation = OwnerMonster->GetActorLocation();
    float Duration = PrimaryComponentTick.TickInterval + 0.1f; 

    DrawDebugCircle(GetWorld(), MonsterLocation, AudioConfig.AlertDistance, 32, FColor::Red, false, Duration, 0, 3.0f, FVector(0, 1, 0), FVector(1, 0, 0));
    DrawDebugCircle(GetWorld(), MonsterLocation, AudioConfig.MaxAudioDistance, 32, FColor::Yellow, false, Duration, 0, 1.0f, FVector(0, 1, 0), FVector(1, 0, 0));
    
    FString StateText = FString::Printf(TEXT("Audio State: %s"), *UEnum::GetValueAsString(CurrentAudioState));
    DrawDebugString(GetWorld(), MonsterLocation + FVector(0, 0, 200), StateText, nullptr, FColor::White, Duration);
} 