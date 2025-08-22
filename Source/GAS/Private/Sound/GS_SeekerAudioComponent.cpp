#include "Sound/GS_SeekerAudioComponent.h"
#include "Character/Player/Seeker/GS_Seeker.h"
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
#include "Character/Skill/GS_SkillComp.h"
#include "UObject/UObjectGlobals.h"
#include "Character/Skill/GS_SkillSet.h"
#include "Character/GS_Character.h"
#include "Character/Player/GS_Player.h"
#include "Character/Component/GS_StatComp.h"


UGS_SeekerAudioComponent::UGS_SeekerAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentAudioState = ESeekerAudioState::Idle;
    PreviousAudioState = ESeekerAudioState::Idle;
    
    AudioConfig.MaxAudioDistance = 2000.0f;
    CombatSoundInterval = 5.0f;
    
    SkillEventID = 0;
    LastHitSoundTime = 0.0f;
}

void UGS_SeekerAudioComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGS_SeekerAudioComponent, CurrentAudioState);
}

void UGS_SeekerAudioComponent::OnRep_CurrentAudioState()
{
    if (OwnerSeeker && GetWorld() && GetWorld()->IsNetMode(NM_Client))
    {
        UpdateSoundTimer();
    }
    PreviousAudioState = CurrentAudioState;
}

void UGS_SeekerAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerSeeker = Cast<AGS_Seeker>(GetOwner());
    if (!OwnerSeeker)
    {
        return;
    }

    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        // 초기 Distance Scaling 설정 (TPS 모드 기본값)
        const float InitialScalingValue = TPSDistanceScaling;
        AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), InitialScalingValue, 0, OwnerSeeker);
    }
    
    if (GetOwner()->HasAuthority())
    {
        StartSoundTimer();
    }
    PreviousAudioState = CurrentAudioState; 
}

void UGS_SeekerAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSoundTimer();
    
    StopAllActiveSounds();
    
    Super::EndPlay(EndPlayReason);
}

float UGS_SeekerAudioComponent::GetMaxAudioDistance() const
{
    return AudioConfig.MaxAudioDistance;
}

void UGS_SeekerAudioComponent::SetSeekerAudioState(ESeekerAudioState NewState)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (CurrentAudioState == NewState)
        return;
        
    PreviousAudioState = CurrentAudioState;
    CurrentAudioState = NewState;
    
    UpdateSoundTimer();
}

void UGS_SeekerAudioComponent::PlaySound(ESeekerAudioState SoundType, bool bForcePlay)
{
    if (!OwnerSeeker || !GetOwner()->HasAuthority())
        return;

    if (!CanSendRPC())
        return;

    if (!bForcePlay)
    {
        float Interval;
        if (SoundType == ESeekerAudioState::Combat) Interval = CombatSoundInterval;
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

void UGS_SeekerAudioComponent::Multicast_TriggerSound_Implementation(ESeekerAudioState SoundTypeToTrigger, bool bIsImmediate)
{
    if (!OwnerSeeker || !GetWorld())
    {
        return;
    }

    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("SeekerAudio: Failed to get listener location"));
        return;
    }

    // RTS 모드와 TPS 모드에 따른 거리 체크
    const bool bRTS = IsRTSMode();
    const float MaxDistance = GetMaxDistanceForMode(bRTS);
    
    float DistanceToListener = FVector::Dist(OwnerSeeker->GetActorLocation(), ListenerLocation);
        
    // RTS 모드에서는 거리 체크를 더 관대하게 (거리 제한을 50% 더 넓게)
    float ActualMaxDistance = bRTS ? MaxDistance * 1.5f : MaxDistance;
    
    if (DistanceToListener > ActualMaxDistance)
    {
        return;
    }
    
    UAkAudioEvent* SoundEvent = GetSoundEvent(SoundTypeToTrigger);
    if (!SoundEvent)
    {
        // RTS 모드에서 사운드 이벤트가 없는 경우 상세 로그 출력
        if (IsRTSMode())
        {
            UE_LOG(LogTemp, Warning, TEXT("SeekerAudio: RTS Mode - No sound event found for state %s"), 
                   *UEnum::GetValueAsString(SoundTypeToTrigger));
            
            // 각 상태별로 어떤 사운드 이벤트가 설정되어 있는지 로그
            switch (SoundTypeToTrigger)
            {
                case ESeekerAudioState::Combat:
                    UE_LOG(LogTemp, Warning, TEXT("SeekerAudio: RTS_CombatSound=%s, TPS_CombatSound=%s"), 
                           AudioConfig.RTS_CombatSound ? *AudioConfig.RTS_CombatSound->GetName() : TEXT("NULL"),
                           AudioConfig.CombatSound ? *AudioConfig.CombatSound->GetName() : TEXT("NULL"));
                    break;
                case ESeekerAudioState::Hurt:
                    UE_LOG(LogTemp, Warning, TEXT("SeekerAudio: RTS_HurtSound=%s, TPS_HurtSound=%s"), 
                           AudioConfig.RTS_HurtSound ? *AudioConfig.RTS_HurtSound->GetName() : TEXT("NULL"),
                           AudioConfig.HurtSound ? *AudioConfig.HurtSound->GetName() : TEXT("NULL"));
                    break;
                case ESeekerAudioState::Death:
                    UE_LOG(LogTemp, Warning, TEXT("SeekerAudio: RTS_DeathSound=%s, TPS_DeathSound=%s"), 
                           AudioConfig.RTS_DeathSound ? *AudioConfig.RTS_DeathSound->GetName() : TEXT("NULL"),
                           AudioConfig.DeathSound ? *AudioConfig.DeathSound->GetName() : TEXT("NULL"));
                    break;
                default:
                    break;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SeekerAudio: TPS Mode - No sound event found for state %s"), 
                   *UEnum::GetValueAsString(SoundTypeToTrigger));
        }
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(bRTS);

    if (!bIsImmediate)
    {
        float Interval;
        // Idle과 Aiming은 시커에서 사용하지 않음
        if (SoundTypeToTrigger == ESeekerAudioState::Combat) Interval = CombatSoundInterval;
        else Interval = 1.0f;

        float CurrentTime = GetWorld()->GetTimeSeconds();
        if ((CurrentTime - LocalLastSoundPlayTimes.FindOrAdd(SoundTypeToTrigger, 0.0f)) < (Interval * LocalSoundCooldownMultiplier)) 
        {
            return; 
        }
        LocalLastSoundPlayTimes.Emplace(SoundTypeToTrigger, CurrentTime);
    }
    
    AkPlayingID NewPlayingID = UAkGameplayStatics::PostEvent(SoundEvent, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(NewPlayingID);
}

void UGS_SeekerAudioComponent::PlayHurtSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
    {
        return;
    }

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayHurtSound();
}

void UGS_SeekerAudioComponent::PlayDeathSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
    {
        return;
    }

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayDeathSound();
}

void UGS_SeekerAudioComponent::PlayBowDrawSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
        return;

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayBowDrawSound();
}

void UGS_SeekerAudioComponent::Multicast_PlayBowDrawSound_Implementation()
{
    if (!OwnerSeeker || !GetWorld())
    {
        return;
    }

    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("SeekerAudio BowDraw: Failed to get listener location"));
        return;
    }

    // RTS 모드와 TPS 모드에 따른 거리 체크
    const bool bRTS = IsRTSMode();
    const float MaxDistance = GetMaxDistanceForMode(bRTS);
    
    const float DistanceToListener = FVector::Dist(OwnerSeeker->GetActorLocation(), ListenerLocation);
    
    // RTS 모드에서는 거리 체크를 더 관대하게 (거리 제한을 50% 더 넓게)
    float ActualMaxDistance = bRTS ? MaxDistance * 1.5f : MaxDistance;
    
    if (DistanceToListener > ActualMaxDistance)
    {
        // 거리가 너무 멀면 사운드 재생하지 않음
        return;
    }

    UAkAudioEvent* SoundToPlay = bRTS ? RTS_BowDrawSound : BowDrawSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        AkPlayingID BowPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(BowPlayingID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SeekerAudio BowDraw: No sound event found for %s mode"), 
               bRTS ? TEXT("RTS") : TEXT("TPS"));
    }
}

void UGS_SeekerAudioComponent::PlayBowReleaseSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
        return;

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayBowReleaseSound();
}

void UGS_SeekerAudioComponent::Multicast_PlayBowReleaseSound_Implementation()
{
    if (!OwnerSeeker || !GetWorld())
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
    
    const float DistanceToListener = FVector::Dist(OwnerSeeker->GetActorLocation(), ListenerLocation);
    if (DistanceToListener > MaxDistance)
    {
        return;
    }

    UAkAudioEvent* SoundToPlay = bRTS ? RTS_BowReleaseSound : BowReleaseSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        AkPlayingID ReleasePlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ReleasePlayingID);
    }
}

// RTS 커맨드 사운드는 시커에서 사용하지 않음 (플레이어 직접 조종)

void UGS_SeekerAudioComponent::StartSoundTimer()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld())
        return;

    StopSoundTimer();
    
    float Interval;
    FTimerDelegate TimerDelegate;

    switch (CurrentAudioState)
    {
        case ESeekerAudioState::Idle:
            break;
            
        case ESeekerAudioState::Combat:
            Interval = CombatSoundInterval; 
            TimerDelegate.BindUObject(this, &UGS_SeekerAudioComponent::PlayCombatSound);
            GetWorld()->GetTimerManager().SetTimer(CombatSoundTimer, TimerDelegate, Interval, true);
            break;
            
        case ESeekerAudioState::Aiming:
            break;
            
        default:
            break;
    }
}

void UGS_SeekerAudioComponent::StopSoundTimer()
{
    if (!GetWorld()) return;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    if (IdleSoundTimer.IsValid())
        TimerManager.ClearTimer(IdleSoundTimer);
    if (CombatSoundTimer.IsValid())
        TimerManager.ClearTimer(CombatSoundTimer);
}

void UGS_SeekerAudioComponent::UpdateSoundTimer()
{
    StartSoundTimer();
}

UAkAudioEvent* UGS_SeekerAudioComponent::GetSoundEvent(ESeekerAudioState SoundType) const
{
    const bool bRTS = IsRTSMode();

    switch (SoundType)
    {
        case ESeekerAudioState::Idle:    
            return nullptr;
        case ESeekerAudioState::Combat:  
            if (bRTS)
            {
                return AudioConfig.RTS_CombatSound ? AudioConfig.RTS_CombatSound : AudioConfig.CombatSound;
            }
            return AudioConfig.CombatSound;
        case ESeekerAudioState::Aiming:  
            return nullptr;
        case ESeekerAudioState::Hurt:    
            if (bRTS)
            {
                return AudioConfig.RTS_HurtSound ? AudioConfig.RTS_HurtSound : AudioConfig.HurtSound;
            }
            return AudioConfig.HurtSound;
        case ESeekerAudioState::Death:   
            if (bRTS)
            {
                return AudioConfig.RTS_DeathSound ? AudioConfig.RTS_DeathSound : AudioConfig.DeathSound;
            }
            return AudioConfig.DeathSound;
        default:
            return nullptr;
    }
}

void UGS_SeekerAudioComponent::CheckForStateChanges()
{
    if (!OwnerSeeker || !GetOwner() || !GetOwner()->HasAuthority() || !GetWorld()) 
        return;
    
    if (CurrentAudioState == ESeekerAudioState::Death)
    {
        return; 
    }
    
    if (OwnerSeeker->GetStatComp() && IsValid(OwnerSeeker->GetStatComp()))
    {
        if (OwnerSeeker->GetStatComp()->GetCurrentHealth() <= 0.0f)
        {
            if (CurrentAudioState != ESeekerAudioState::Death)
            {
                SetSeekerAudioState(ESeekerAudioState::Death); 
            }
            return; 
        }
    }
    
    if (CurrentAudioState == ESeekerAudioState::Hurt) return;
    
    // 시커의 상태에 따른 오디오 상태 변경
    if (OwnerSeeker->GetAimState())
    {
        if (CurrentAudioState != ESeekerAudioState::Aiming)
        {
            SetSeekerAudioState(ESeekerAudioState::Aiming);
        }
        return;
    }
    
}

void UGS_SeekerAudioComponent::PlayIdleSound()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        PlaySound(ESeekerAudioState::Idle, false);
    }
}

void UGS_SeekerAudioComponent::PlayCombatSound()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        PlaySound(ESeekerAudioState::Combat, false);
    }
}

// =============================================
// 기존 CharacterAudioComponent 기능들 (스킬 관련)
// =============================================

void UGS_SeekerAudioComponent::PlaySkill()
{
    FOnAkPostEventCallback DummyCallback;

    SkillEventID = UAkGameplayStatics::PostEvent(SkillEvent,
        GetOwner(), // Post the event to the owner of this component 
        0, // No callback mask
        DummyCallback, // No callback
        false // bStopWhenAttachedToDestroyed
    );
}

void UGS_SeekerAudioComponent::StopSkill()
{
    if (GetOwner())
    {
        UAkGameplayStatics::StopActor(GetOwner());
    }
}

UAkComponent* UGS_SeekerAudioComponent::GetOrCreateAkComponent()
{
    if (CachedAkComponent && IsValid(CachedAkComponent))
    {
        return CachedAkComponent;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    CachedAkComponent = Owner->FindComponentByClass<UAkComponent>();
    if (!CachedAkComponent)
    {
        CachedAkComponent = NewObject<UAkComponent>(Owner);
        CachedAkComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        CachedAkComponent->RegisterComponent();
    }

    return CachedAkComponent;
}

void UGS_SeekerAudioComponent::PlaySoundAtLocation(UAkAudioEvent* SoundEvent, const FVector& Location)
{
    // 데디케이티드 서버에서는 사운드 재생하지 않음
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
    {
        return;
    }

    if (!SoundEvent)
    {
        return;
    }

    if (!FAkAudioDevice::Get())
    {
        return;
    }

    if (Location != FVector::ZeroVector)
    {
        UAkGameplayStatics::PostEventAtLocation(SoundEvent, Location, FRotator::ZeroRotator, GetWorld());
    }
    else
    {
        // 위치가 Zero Vector면 Owner 위치에서 재생
        UAkComponent* AkComp = GetOrCreateAkComponent();
        if (AkComp)
        {
            AkPlayingID LocationPlayingID = UAkGameplayStatics::PostEvent(SoundEvent, GetOwner(), 0, FOnAkPostEventCallback());
            RegisterPlayingID(LocationPlayingID);
        }
    }
}

const struct FSkillInfo* UGS_SeekerAudioComponent::GetSkillInfoFromDataTable(ESkillSlot SkillSlot) const
{
    // Owner가 AGS_Player인지 확인
    AGS_Player* OwnerPlayer = Cast<AGS_Player>(GetOwner());
    if (!OwnerPlayer)
    {
        return nullptr;
    }

    // SkillComp 가져오기
    UGS_SkillComp* SkillComp = OwnerPlayer->GetSkillComp();
    if (!SkillComp)
    {
        return nullptr;
    }

    // 데이터 테이블 가져오기
    UDataTable* SkillDataTable = SkillComp->GetSkillDataTable();
    if (!SkillDataTable)
    {
        return nullptr;
    }

    // 캐릭터 타입을 기반으로 RowName 구하기
    FString CharTypeString = UEnum::GetValueAsString(OwnerPlayer->GetCharacterType());
    int32 SeparatorIndex;
    if (CharTypeString.FindChar(TEXT(':'), SeparatorIndex))
    {
        CharTypeString = CharTypeString.RightChop(SeparatorIndex + 2);
    }
    FName RowName = FName(*CharTypeString);

    // 스킬셋 찾기
    FString Context;
    FGS_SkillSet* SkillSet = SkillDataTable->FindRow<FGS_SkillSet>(RowName, Context);
    if (!SkillSet)
    {
        return nullptr;
    }

    // 스킬 슬롯에 따라 적절한 스킬 정보 반환
    switch (SkillSlot)
    {
        case ESkillSlot::Ready:    return &SkillSet->ReadySkill;
        case ESkillSlot::Aiming:   return &SkillSet->AimingSkill;
        case ESkillSlot::Moving:   return &SkillSet->MovingSkill;
        case ESkillSlot::Ultimate: return &SkillSet->UltimateSkill;
        case ESkillSlot::Rolling:  return &SkillSet->RollingSkill;
        default:                   return nullptr;
    }
}

void UGS_SeekerAudioComponent::PlaySkillSoundFromDataTable(ESkillSlot SkillSlot, bool bIsSkillStart)
{
    const FSkillInfo* SkillInfo = GetSkillInfoFromDataTable(SkillSlot);
    if (!SkillInfo)
    {
        return;
    }

    // 스킬 시작/종료 사운드 재생
    PlaySkillSoundFromSkillInfo(bIsSkillStart, SkillInfo->SkillStartSound, SkillInfo->SkillEndSound);
}

void UGS_SeekerAudioComponent::PlaySkillLoopSoundFromDataTable(ESkillSlot SkillSlot)
{
    const FSkillInfo* SkillInfo = GetSkillInfoFromDataTable(SkillSlot);
    if (!SkillInfo || !SkillInfo->SkillLoopSound)
    {
        return;
    }

    // 루프 사운드 재생
    AkPlayingID LoopPlayingID = UAkGameplayStatics::PostEvent(SkillInfo->SkillLoopSound, GetOwner(), 0, FOnAkPostEventCallback());
    RegisterPlayingID(LoopPlayingID);
}

void UGS_SeekerAudioComponent::StopSkillLoopSoundFromDataTable(ESkillSlot SkillSlot)
{
    const FSkillInfo* SkillInfo = GetSkillInfoFromDataTable(SkillSlot);
    if (!SkillInfo || !SkillInfo->SkillLoopStopSound)
    {
        return;
    }

    // 루프 사운드 정지
    AkPlayingID StopPlayingID = UAkGameplayStatics::PostEvent(SkillInfo->SkillLoopStopSound, GetOwner(), 0, FOnAkPostEventCallback());
    RegisterPlayingID(StopPlayingID);
}

void UGS_SeekerAudioComponent::PlaySkillCollisionSoundFromDataTable(ESkillSlot SkillSlot, uint8 CollisionType)
{
    const FSkillInfo* SkillInfo = GetSkillInfoFromDataTable(SkillSlot);
    if (!SkillInfo)
    {
        return;
    }

    // 충돌 타입에 따른 사운드 선택
    UAkAudioEvent* CollisionSound = nullptr;
    switch (CollisionType)
    {
        case 0: // 벽 충돌
            CollisionSound = SkillInfo->WallCollisionSound;
            break;
        case 1: // 몬스터 충돌
            CollisionSound = SkillInfo->MonsterCollisionSound;
            break;
        case 2: // 가디언 충돌
            CollisionSound = SkillInfo->GuardianCollisionSound;
            break;
        default:
            return;
    }

    if (CollisionSound)
    {
        AkPlayingID CollisionPlayingID = UAkGameplayStatics::PostEvent(CollisionSound, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(CollisionPlayingID);
    }
}

void UGS_SeekerAudioComponent::RequestSkillAudio(ESkillSlot SkillSlot, int32 AudioEventType, FVector Location)
{
    // Event-Driven 방식으로 적절한 함수 호출
    switch (AudioEventType)
    {
        case 0: // 스킬 시작
            PlaySkillSoundFromDataTable(SkillSlot, true);
            break;
        case 1: // 스킬 종료
            PlaySkillSoundFromDataTable(SkillSlot, false);
            break;
        case 2: // 루프 시작 (궁극기)
            PlaySkillLoopSoundFromDataTable(SkillSlot);
            break;
        case 3: // 루프 정지 (궁극기)
            StopSkillLoopSoundFromDataTable(SkillSlot);
            break;
        default:
            // 충돌 사운드 (4=벽, 5=몬스터, 6=가디언)
            if (AudioEventType >= 4 && AudioEventType <= 6)
            {
                PlaySkillCollisionSoundFromDataTable(SkillSlot, AudioEventType - 4);
            }
            break;
    }
}

void UGS_SeekerAudioComponent::PlaySkillSoundFromSkillInfo(bool bIsSkillStart, UAkAudioEvent* SkillStartSound, UAkAudioEvent* SkillEndSound)
{
    UAkAudioEvent* SoundToPlay = bIsSkillStart ? SkillStartSound : SkillEndSound;
    if (SoundToPlay)
    {
        AkPlayingID SkillPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(SkillPlayingID);
    }
}

void UGS_SeekerAudioComponent::PlayComboAttackSound(UAkAudioEvent* SwingSound, UAkAudioEvent* VoiceSound, UAkAudioEvent* StopEvent, float ResetTime)
{
    // 콤보 공격 사운드 구현
    if (SwingSound)
    {
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(SwingSound, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(SwingPlayingID);
    }
    
    if (VoiceSound)
    {
        AkPlayingID VoicePlayingID = UAkGameplayStatics::PostEvent(VoiceSound, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(VoicePlayingID);
    }

    CurrentStopEvent = StopEvent;
    if (ResetTime > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(AttackSoundResetTimerHandle, this, &UGS_SeekerAudioComponent::ResetAttackSoundSequence, ResetTime, false);
    }
}

void UGS_SeekerAudioComponent::PlayComboAttackSoundByIndex(int32 ComboIndex, const TArray<UAkAudioEvent*>& SwingSounds, const TArray<UAkAudioEvent*>& VoiceSounds, UAkAudioEvent* StopEvent, float ResetTime)
{
    if (SwingSounds.IsValidIndex(ComboIndex))
    {
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(SwingSounds[ComboIndex], GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(SwingPlayingID);
    }
    
    if (VoiceSounds.IsValidIndex(ComboIndex))
    {
        AkPlayingID VoicePlayingID = UAkGameplayStatics::PostEvent(VoiceSounds[ComboIndex], GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(VoicePlayingID);
    }

    CurrentStopEvent = StopEvent;
    if (ResetTime > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(AttackSoundResetTimerHandle, this, &UGS_SeekerAudioComponent::ResetAttackSoundSequence, ResetTime, false);
    }
}

void UGS_SeekerAudioComponent::PlayComboAttackSoundByIndexWithExtra(int32 ComboIndex, const TArray<UAkAudioEvent*>& SwingSounds, const TArray<UAkAudioEvent*>& VoiceSounds, const TArray<UAkAudioEvent*>& ExtraSounds, UAkAudioEvent* StopEvent, float ResetTime)
{
    PlayComboAttackSoundByIndex(ComboIndex, SwingSounds, VoiceSounds, StopEvent, ResetTime);
    
    if (ExtraSounds.IsValidIndex(ComboIndex))
    {
        AkPlayingID ExtraPlayingID = UAkGameplayStatics::PostEvent(ExtraSounds[ComboIndex], GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(ExtraPlayingID);
    }
}

void UGS_SeekerAudioComponent::PlayFinalAttackSound(UAkAudioEvent* ExtraSound)
{
    if (ExtraSound)
    {
        AkPlayingID FinalPlayingID = UAkGameplayStatics::PostEvent(ExtraSound, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(FinalPlayingID);
    }
}

void UGS_SeekerAudioComponent::PlayGenericSound(UAkAudioEvent* SoundToPlay, bool bPlayOnLocalOnly)
{
    if (!SoundToPlay) return;

    if (bPlayOnLocalOnly || GetWorld()->GetNetMode() == NM_Standalone)
    {
        AkPlayingID GenericPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(GenericPlayingID);
    }
    else
    {
        // 멀티플레이어에서는 RPC를 통해 동기화 필요
        // 필요한 경우 별도의 RPC 함수 추가
    }
}

void UGS_SeekerAudioComponent::PlayHitSound()
{
    // PlayHitSound는 PlayHurtSound로 대체됨 - RTS/TPS 모드 지원을 위해
    PlayHurtSound();
}

void UGS_SeekerAudioComponent::ResetAttackSoundSequence()
{
    if (CurrentStopEvent)
    {
        AkPlayingID StopPlayingID = UAkGameplayStatics::PostEvent(CurrentStopEvent, GetOwner(), 0, FOnAkPostEventCallback());
        RegisterPlayingID(StopPlayingID);
        CurrentStopEvent = nullptr;
    }
}

// ===================
// 화살 관련 함수 구현
// ===================

void UGS_SeekerAudioComponent::PlayArrowShotSound()
{
    // This function is called from Merci's multicast RPC, so play directly
    Multicast_PlayArrowShotSound_Implementation();
}

void UGS_SeekerAudioComponent::PlayArrowTypeChangeSound()
{
    if (!CanSendRPC()) return;
    
    Multicast_PlayArrowTypeChangeSound();
}

void UGS_SeekerAudioComponent::PlayArrowEmptySound()
{
    if (!CanSendRPC()) return;
    
    Multicast_PlayArrowEmptySound();
}

void UGS_SeekerAudioComponent::PlayHitFeedbackSound()
{
    if (!CanSendRPC()) return;
    
    Multicast_PlayHitFeedbackSound();
}

// ===================
// 화살 관련 멀티캐스트 RPC 구현
// ===================

void UGS_SeekerAudioComponent::Multicast_PlayArrowShotSound_Implementation()
{
    if (!OwnerSeeker || !GetWorld())
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
    
    const float DistanceToListener = FVector::Dist(OwnerSeeker->GetActorLocation(), ListenerLocation);
    if (DistanceToListener > MaxDistance)
    {
        return;
    }

    UAkAudioEvent* SoundToPlay = bRTS ? RTS_ArrowShotSound : ArrowShotSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        AkPlayingID ArrowShotPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ArrowShotPlayingID);
    }
}

void UGS_SeekerAudioComponent::Multicast_PlayArrowTypeChangeSound_Implementation()
{
    if (!OwnerSeeker || !ArrowTypeChangeSound)
    {
        return;
    }

    AkPlayingID ChangePlayingID = UAkGameplayStatics::PostEvent(ArrowTypeChangeSound, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(ChangePlayingID);
}

void UGS_SeekerAudioComponent::Multicast_PlayArrowEmptySound_Implementation()
{
    if (!OwnerSeeker || !ArrowEmptySound)
    {
        return;
    }

    AkPlayingID EmptyPlayingID = UAkGameplayStatics::PostEvent(ArrowEmptySound, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(EmptyPlayingID);
}

void UGS_SeekerAudioComponent::Multicast_PlayHitFeedbackSound_Implementation()
{
    if (!OwnerSeeker || !HitFeedbackSound_Archery)
    {
        return;
    }

    // 히트 피드백 사운드는 UI 피드백이므로 거리 제한 없이 재생
    AkPlayingID FeedbackPlayingID = UAkGameplayStatics::PostEvent(HitFeedbackSound_Archery, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(FeedbackPlayingID);
}

void UGS_SeekerAudioComponent::Multicast_PlayHurtSound_Implementation()
{
    if (!OwnerSeeker || !GetWorld())
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
    
    const float DistanceToListener = FVector::Dist(OwnerSeeker->GetActorLocation(), ListenerLocation);
    
    // RTS 모드에서는 거리 체크를 더 관대하게 (50% 더 넓게)
    float ActualMaxDistance = bRTS ? MaxDistance * 1.5f : MaxDistance;
    
    // EXPERIMENTAL: In RTS mode with multiplayer, try bypassing distance check for test
    bool bSkipDistanceCheck = false;
    if (bRTS && GetWorld()->GetNetMode() != NM_Standalone)
    {
        bSkipDistanceCheck = true;
    }
    
    if (!bSkipDistanceCheck && DistanceToListener > ActualMaxDistance)
    {
        return;
    }

    UAkAudioEvent* SoundToPlay = bRTS ? AudioConfig.RTS_HurtSound : AudioConfig.HurtSound;
    
    // 폴백 시스템: RTS 사운드가 없으면 TPS 사운드 사용
    if (bRTS && !SoundToPlay && AudioConfig.HurtSound)
    {
        SoundToPlay = AudioConfig.HurtSound;
    }
    
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        // Enhanced Wwise debugging
        if (FAkAudioDevice* AkDevice = FAkAudioDevice::Get())
        {
            // Set RTPC values explicitly for debugging
            AkDevice->SetRTPCValue(*DistanceToPlayerRTPCName.ToString(), DistanceToListener, 0, OwnerSeeker);
            AkDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), GetDistanceScalingForMode(bRTS), 0, OwnerSeeker);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No Wwise AudioDevice found!"));
        }
        
        AkPlayingID HurtPlayingID;
        
        // EXPERIMENTAL: In RTS multiplayer mode, try posting with different approach
        if (bRTS && GetWorld()->GetNetMode() != NM_Standalone)
        {
            // Try posting with OwnerSeeker but with different RTPC settings
            HurtPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        }
        else
        {
            HurtPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        }
        
        RegisterPlayingID(HurtPlayingID);
        
        // Check if the sound is considered playing
        if (HurtPlayingID == AK_INVALID_PLAYING_ID)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Playing ID - sound may not play"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayHurtSound - No sound event available!"));
    }
}

void UGS_SeekerAudioComponent::Multicast_PlayDeathSound_Implementation()
{
    if (!OwnerSeeker || !GetWorld())
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
    
    const float DistanceToListener = FVector::Dist(OwnerSeeker->GetActorLocation(), ListenerLocation);
    
    // RTS 모드에서는 거리 체크를 더 관대하게 (50% 더 너비게)
    float ActualMaxDistance = bRTS ? MaxDistance * 1.5f : MaxDistance;
    
    // EXPERIMENTAL: In RTS mode with multiplayer, try bypassing distance check for test
    bool bSkipDistanceCheck = false;
    if (bRTS && GetWorld()->GetNetMode() != NM_Standalone)
    {
        bSkipDistanceCheck = true;
    }
    
    if (!bSkipDistanceCheck && DistanceToListener > ActualMaxDistance)
    {
        return;
    }

    UAkAudioEvent* SoundToPlay = bRTS ? AudioConfig.RTS_DeathSound : AudioConfig.DeathSound;
    
    // 폴백 시스템: RTS 사운드가 없으면 TPS 사운드 사용
    if (bRTS && !SoundToPlay && AudioConfig.DeathSound)
    {
        SoundToPlay = AudioConfig.DeathSound;
    }
    
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        // Enhanced Wwise debugging for Death sound
        if (FAkAudioDevice* AkDevice = FAkAudioDevice::Get())
        {
            AkDevice->SetRTPCValue(*DistanceToPlayerRTPCName.ToString(), DistanceToListener, 0, OwnerSeeker);
            AkDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), GetDistanceScalingForMode(bRTS), 0, OwnerSeeker);
        }
        
        AkPlayingID DeathPlayingID;
        
        // EXPERIMENTAL: In RTS multiplayer mode, try posting with different approach
        if (bRTS && GetWorld()->GetNetMode() != NM_Standalone)
        {
            // Try posting with OwnerSeeker but with different RTPC settings
            DeathPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        }
        else
        {
            DeathPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        }
        
        RegisterPlayingID(DeathPlayingID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayDeathSound - No sound event available!"));
    }
}