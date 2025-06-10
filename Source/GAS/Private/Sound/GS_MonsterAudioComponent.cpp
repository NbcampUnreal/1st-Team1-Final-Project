#include "Sound/GS_MonsterAudioComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Component/GS_StatComp.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "Net/UnrealNetwork.h" // For DOREPLIFETIME
#include "Kismet/GameplayStatics.h" // For GetPlayerController

// RTPC 이름을 상수로 정의 (클래스 외부)
const FName UGS_MonsterAudioComponent::DistanceToPlayerRTPCName = TEXT("Distance_to_Player");
const FName UGS_MonsterAudioComponent::MonsterVariantRTPCName = TEXT("Monster_Variant");

UGS_MonsterAudioComponent::UGS_MonsterAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 0.5초마다 틱
    
    CurrentAudioState = EMonsterAudioState::Idle;
    PreviousAudioState = EMonsterAudioState::Idle;
    CurrentPlayingID = AK_INVALID_PLAYING_ID; // 각 클라이언트/서버 인스턴스에서 현재 재생 중인 ID
    
    // 기본 설정값
    AudioConfig.AlertDistance = 800.0f;
    AudioConfig.MaxAudioDistance = 1000.0f;
    IdleSoundInterval = 6.0f;
    CombatSoundInterval = 4.0f;
    
    MonsterSoundVariant = 1;

    SetIsReplicatedByDefault(true); // 컴포넌트 복제 활성화
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
        UE_LOG(LogTemp, Warning, TEXT("UGS_MonsterAudioComponent: Owner is not a Monster!"));
        return;
    }

    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        if (MonsterSoundVariant > 0) 
        {
            AkAudioDevice->SetRTPCValue(*MonsterVariantRTPCName.ToString(), MonsterSoundVariant, 0, OwnerMonster);
        }
    }
    
    if (GetOwner()->HasAuthority()) // HasAuthority()가 더 명확하고 일반적인 서버 체크 방식입니다.
    {
        StartSoundTimer();
    }
    PreviousAudioState = CurrentAudioState; 
}

void UGS_MonsterAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSoundTimer(); // 서버/클라이언트 모두 타이머 중지
    
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

void UGS_MonsterAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerMonster)
        return;
        
    if (GetOwner()->HasAuthority())
    {
        CheckForStateChanges();
    }

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
    // CurrentAudioState가 복제 변수이므로, 서버에서 변경되면 OnRep_CurrentAudioState가 클라이언트에서 자동 호출됨
    
    // 서버에서 상태 변화에 따른 타이머 업데이트
    UpdateSoundTimer();
    
    // Combat 상태로 변경 시 즉시 사운드 재생
    if (NewState == EMonsterAudioState::Combat && CurrentAudioState != PreviousAudioState) // 중복 방지
    {
        PlaySound(NewState, true);
    }
}

// Server-authoritative function to decide to broadcast a sound trigger
void UGS_MonsterAudioComponent::PlaySound(EMonsterAudioState SoundType, bool bForcePlay)
{
    if (!OwnerMonster || !GetOwner()->HasAuthority())
        return;

    if (!bForcePlay)
    {
        float Interval = 0.0f;
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
        float Interval = 0.0f;
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

AGS_Seeker* UGS_MonsterAudioComponent::FindNearestSeeker() const
{
    if (!GetWorld() || !OwnerMonster) return nullptr;
    
    AGS_Seeker* NearestSeeker = nullptr;
    float NearestDistance = FLT_MAX;
    FVector MonsterLocation = OwnerMonster->GetActorLocation();
    
    for (TActorIterator<AGS_Seeker> SeekerIterator(GetWorld()); SeekerIterator; ++SeekerIterator)
    {
        AGS_Seeker* Seeker = *SeekerIterator;
        if (!Seeker || !Seeker->IsValidLowLevel() || !IsValid(Seeker)) 
            continue;
            
        float Distance = FVector::Dist(MonsterLocation, Seeker->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestSeeker = Seeker;
        }
    }
    return NearestSeeker;
}

float UGS_MonsterAudioComponent::CalculateDistanceToNearestSeeker() const
{
    // 서버에서 주로 사용. 클라이언트에서는 로컬 플레이어와의 거리를 계산하는 별도 로직이 Tick에 있음.
    AGS_Seeker* NearestSeeker = FindNearestSeeker();
    if (!NearestSeeker || !OwnerMonster)
        return -1.0f; // 혹은 FLT_MAX
    
    return FVector::Dist(OwnerMonster->GetActorLocation(), NearestSeeker->GetActorLocation());
}

void UGS_MonsterAudioComponent::PlayIdleSound()
{
    // 서버에서만 타이머에 의해 호출됨
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        PlaySound(EMonsterAudioState::Idle, false);
    }
}

void UGS_MonsterAudioComponent::PlayCombatSound()
{
    // 서버에서만 타이머에 의해 호출됨
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        PlaySound(EMonsterAudioState::Combat, false);
    }
}

void UGS_MonsterAudioComponent::StartSoundTimer()
{
    // 서버에서만 타이머 시작/제어
    if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld())
        return;

    StopSoundTimer();
    
    float Interval = 0.0f;
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
            ensureMsgf(false, TEXT("GetSoundEvent: Unknown SoundType %s requested for monster %s"), *UEnum::GetValueAsString(SoundType), OwnerMonster ? *OwnerMonster->GetName() : TEXT("UnknownMonster"));
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

    // 기본 거리 원형 표시
    DrawDebugCircle(GetWorld(), MonsterLocation, AudioConfig.AlertDistance, 32, FColor::Red, false, Duration, 0, 3.0f, FVector(0, 1, 0), FVector(1, 0, 0));
    DrawDebugCircle(GetWorld(), MonsterLocation, AudioConfig.MaxAudioDistance, 32, FColor::Yellow, false, Duration, 0, 1.0f, FVector(0, 1, 0), FVector(1, 0, 0));
    
    // 상태 정보 표시
    FString StateText = FString::Printf(TEXT("Audio State: %s"), *UEnum::GetValueAsString(CurrentAudioState));
    DrawDebugString(GetWorld(), MonsterLocation + FVector(0, 0, 200), StateText, nullptr, FColor::White, Duration);
} 