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

// RTPC 이름을 상수로 정의
const FName UGS_MonsterAudioComponent::DistanceToPlayerRTPCName = TEXT("Distance_to_Player");
const FName UGS_MonsterAudioComponent::MonsterVariantRTPCName = TEXT("Monster_Variant");

UGS_MonsterAudioComponent::UGS_MonsterAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 틱 비활성화
    
    CurrentAudioState = EMonsterAudioState::Idle;
    PreviousAudioState = EMonsterAudioState::Idle;
    CurrentPlayingID = AK_INVALID_PLAYING_ID; // 각 클라이언트/서버 인스턴스에서 현재 재생 중인 ID
    
    // 기본 설정값
    AudioConfig.AlertDistance = 800.0f;
    AudioConfig.MaxAudioDistance = 1000.0f;
    IdleSoundInterval = 6.0f;
    CombatSoundInterval = 1.0f;
    
    MonsterSoundVariant = 1;

    SetIsReplicatedByDefault(true);
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
    }
    
    if (GetOwner()->HasAuthority())
    {
        StartSoundTimer();
    }
    PreviousAudioState = CurrentAudioState; 
    // 거리 체크 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(DistanceCheckTimerHandle, this, &UGS_MonsterAudioComponent::UpdateDistanceRTPC, 0.5f, true);
}

void UGS_MonsterAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSoundTimer(); // 서버/클라이언트 모두 타이머 중지
    GetWorld()->GetTimerManager().ClearTimer(DistanceCheckTimerHandle); // 거리 체크 타이머 중지
    
    // 로컬에서 재생 중인 사운드 중지
    if (CurrentPlayingID != AK_INVALID_PLAYING_ID)
    {
        if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
        {
            AkAudioDevice->StopPlayingID(CurrentPlayingID);
        }
        CurrentPlayingID = AK_INVALID_PLAYING_ID;
    }
    
    Super::EndPlay(EndPlayReason);
}

void UGS_MonsterAudioComponent::SetMonsterAudioState(EMonsterAudioState NewState)
{
    // 서버에서만 상태 변경 가능
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (CurrentAudioState == NewState)
        return;
        
    PreviousAudioState = CurrentAudioState; // 상태 변경 직전에 이전 상태 기록
    CurrentAudioState = NewState;
    
    // 서버에서 상태 변화에 따른 타이머 업데이트
    UpdateSoundTimer();
}

void UGS_MonsterAudioComponent::UpdateDistanceRTPC()
{
    if (!OwnerMonster) return;

    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (LocalPC && LocalPC->GetPawn())
    {
        float DistanceToLocalPlayer = FVector::Dist(OwnerMonster->GetActorLocation(), LocalPC->GetPawn()->GetActorLocation());

        if (DistanceToLocalPlayer <= AudioConfig.MaxAudioDistance)
        {
            if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
            {
                AkAudioDevice->SetRTPCValue(*DistanceToPlayerRTPCName.ToString(), DistanceToLocalPlayer, 0, OwnerMonster);
            }
        }
    }

    // 서버에서만 상태 변경 체크
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        CheckForStateChanges();
    }
}

void UGS_MonsterAudioComponent::PlaySound(EMonsterAudioState SoundType, bool bForcePlay)
{
    if (!OwnerMonster || !GetOwner()->HasAuthority())
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
    
    Multicast_TriggerSound(SoundType, bForcePlay);
}

void UGS_MonsterAudioComponent::Multicast_TriggerSound_Implementation(EMonsterAudioState SoundTypeToTrigger, bool bIsImmediate)
{
    if (!OwnerMonster || !GetWorld())
    {
        return;
    }

    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!LocalPC)
    {
        return; 
    }
    if (!LocalPC->GetPawn())
    {
        return;
    }

    float DistanceToLocalPlayer = FVector::Dist(OwnerMonster->GetActorLocation(), LocalPC->GetPawn()->GetActorLocation());

    if (DistanceToLocalPlayer > AudioConfig.MaxAudioDistance)
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

        float CurrentTime = GetWorld()->GetTimeSeconds();
        if ((CurrentTime - LocalLastSoundPlayTimes.FindOrAdd(SoundTypeToTrigger, 0.0f)) < (Interval * 0.9f)) 
        {
            return; 
        }
        LocalLastSoundPlayTimes.Emplace(SoundTypeToTrigger, CurrentTime);
    }
    
    CurrentPlayingID = UAkGameplayStatics::PostEvent(SoundEvent, OwnerMonster, 0, FOnAkPostEventCallback());
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

    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - ServerLastSwingBroadcastTime < SwingResetTime)
    {
        return;
    }
    ServerLastSwingBroadcastTime = CurrentTime;

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
        return -1.0f; // 혹은 FLT_MAX
    
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

    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!LocalPC || !LocalPC->GetPawn())
    {
        return;
    }

    const float DistanceToLocalPlayer = FVector::Dist(OwnerMonster->GetActorLocation(), LocalPC->GetPawn()->GetActorLocation());
    if (DistanceToLocalPlayer > AudioConfig.MaxAudioDistance)
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float LastTime = LocalLastSwingPlayTime;
    if (CurrentTime - LastTime < SwingResetTime * 0.9f)
    {
        return;
    }
    LocalLastSwingPlayTime = CurrentTime;

    if (SwingSound)
    {
        UAkGameplayStatics::PostEvent(SwingSound, OwnerMonster, 0, FOnAkPostEventCallback());
    }
}

void UGS_MonsterAudioComponent::PlaySelectionClickSound()
{
    if (!OwnerMonster || !SelectionClickSound) return;
    UAkGameplayStatics::PostEvent(SelectionClickSound, OwnerMonster, 0, FOnAkPostEventCallback());
}

void UGS_MonsterAudioComponent::PlayRTSMoveCommandSound()
{
    if (!OwnerMonster || !RTSMoveCommandSound) return;
    UAkGameplayStatics::PostEvent(RTSMoveCommandSound, OwnerMonster, 0, FOnAkPostEventCallback());
}

void UGS_MonsterAudioComponent::PlayRTSAttackCommandSound()
{
    if (!OwnerMonster) return;
    UAkAudioEvent* EventToPost = RTSAttackCommandSound ? RTSAttackCommandSound : RTSMoveCommandSound;
    if (EventToPost)
    {
        UAkGameplayStatics::PostEvent(EventToPost, OwnerMonster, 0, FOnAkPostEventCallback());
    }
}

void UGS_MonsterAudioComponent::PlayRTSUnitDeathSound()
{
    if (!OwnerMonster || !RTSUnitDeathSound) return;
    UAkGameplayStatics::PostEvent(RTSUnitDeathSound, OwnerMonster, 0, FOnAkPostEventCallback());
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
            break;
            
        default: // Hurt, Death 등은 타이머로 소리내지 않음
            break;
    }
}

void UGS_MonsterAudioComponent::StopSoundTimer()
{
    // 서버/클라이언트 모두 타이머 중지 가능 (OnRep 또는 EndPlay 등에서 호출)
    if (!GetWorld()) return;

    if (IdleSoundTimer.IsValid())
        GetWorld()->GetTimerManager().ClearTimer(IdleSoundTimer);
    if (CombatSoundTimer.IsValid())
        GetWorld()->GetTimerManager().ClearTimer(CombatSoundTimer);
}

void UGS_MonsterAudioComponent::UpdateSoundTimer()
{
    // 서버에서는 직접 호출, 클라이언트에서는 OnRep_CurrentAudioState를 통해 호출됨.
    StartSoundTimer();
}

UAkAudioEvent* UGS_MonsterAudioComponent::GetSoundEvent(EMonsterAudioState SoundType) const
{
    switch (SoundType)
    {
        case EMonsterAudioState::Idle:    return AudioConfig.IdleSound;
        case EMonsterAudioState::Combat:  return AudioConfig.CombatSound;
        case EMonsterAudioState::Hurt:    return AudioConfig.HurtSound;
        case EMonsterAudioState::Death:   return AudioConfig.DeathSound;
        default:
            return nullptr;
    }
}

void UGS_MonsterAudioComponent::CheckForStateChanges()
{
    if (!OwnerMonster || !GetOwner()->HasAuthority()) return;
    
    if (CurrentAudioState == EMonsterAudioState::Death)
    {
        return; 
    }
    
    if (OwnerMonster->GetStatComp() && OwnerMonster->GetStatComp()->GetCurrentHealth() <= 0.0f)
    {
        if (CurrentAudioState != EMonsterAudioState::Death)
        {
            SetMonsterAudioState(EMonsterAudioState::Death); 
        }
        return; 
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