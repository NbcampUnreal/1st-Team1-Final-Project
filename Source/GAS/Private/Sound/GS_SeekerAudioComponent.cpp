#include "Sound/GS_SeekerAudioComponent.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/World.h"
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
    
    SkillEventID = AK_INVALID_PLAYING_ID;
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
	
	// Owner가 GS_Character인지 확인
	OwnerCharacter = Cast<AGS_Character>(GetOwner());
	if (OwnerCharacter)
	{
		// Owner의 CharacterType을 자동으로 설정
		ECharacterType DetectedType = OwnerCharacter->GetCharacterType();
		CharacterType = DetectedType;
		
		// 타입별 로그 출력 (디버깅용)
		FString TypeName;
		switch(DetectedType)
		{
			case ECharacterType::Ares:
				TypeName = TEXT("Ares (Melee - Sword)");
				break;
			case ECharacterType::Chan:
				TypeName = TEXT("Chan (Melee - Axe/Shield)");
				break;
			case ECharacterType::Merci:
				TypeName = TEXT("Merci (Ranged - Bow)");
				break;
			default:
				TypeName = TEXT("Unknown");
				break;
		}
	    
		// Owner가 GS_Seeker인지도 확인
		OwnerSeeker = Cast<AGS_Seeker>(GetOwner());
	}
    
}

void UGS_SeekerAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSoundTimer();
    
    StopAllActiveSounds();
    
    Super::EndPlay(EndPlayReason);
}

void UGS_SeekerAudioComponent::OnSpecificSoundFinished(AkPlayingID FinishedID)
{
    // 스킬 이벤트가 완료된 경우 SkillEventID 초기화
    if (SkillEventID == FinishedID)
    {
        SkillEventID = AK_INVALID_PLAYING_ID;
    }
    
    // 부모 클래스 처리
    Super::OnSpecificSoundFinished(FinishedID);
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
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if ((CurrentTime - ServerLastBroadcastTime.FindOrAdd(SoundType, 0.0f)) < 1.0f)
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
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
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
    
    float DistanceToListener = FVector::Dist(OwnerSeeker->GetActorLocation(), ListenerLocation);
        
    // RTS 모드에서는 화면 시야각 기반 체크, TPS 모드에서는 거리 기반 체크
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
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
    SetDistanceScaling(bRTS);

    if (!bIsImmediate)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if ((CurrentTime - LocalLastSoundPlayTimes.FindOrAdd(SoundTypeToTrigger, 0.0f)) < (1.0f * LocalSoundCooldownMultiplier)) 
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

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayHurtSound();
}

void UGS_SeekerAudioComponent::PlayDeathSound()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // 죽음 소리는 중요하므로 RPC 빈도 체크 우회 (항상 재생)
    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayDeathSound();
}

void UGS_SeekerAudioComponent::PlayBowDrawSound()
{
    // 메르시만 활 사운드 재생 가능
    if (CharacterType != ECharacterType::Merci)
    {
        return;
    }
    
    if (!OwnerSeeker || !OwnerSeeker->IsMerci())
    {
        return;
    }

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
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
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
    
    // RTS 모드에서는 화면 시야각 기반 체크, TPS 모드에서는 거리 기반 체크
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
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

    UAkAudioEvent* SoundToPlay = bRTS ? RTSMerciBowDrawSound : BowDrawSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        AkPlayingID BowPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(BowPlayingID);
    }
    else
    {
    }
}

void UGS_SeekerAudioComponent::PlayBowReleaseSound()
{
    // 메르시만 활 사운드 재생 가능
    if (CharacterType != ECharacterType::Merci)
    {
        return;
    }
    
    if (!OwnerSeeker || !OwnerSeeker->IsMerci())
    {
        return;
    }

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
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
        
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
    
    // RTS 모드에서는 화면 시야각 기반 체크, TPS 모드에서는 거리 기반 체크
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
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

    UAkAudioEvent* SoundToPlay = bRTS ? RTSMerciBowReleaseSound : BowReleaseSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        AkPlayingID ReleasePlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ReleasePlayingID);
    }
}

void UGS_SeekerAudioComponent::StartSoundTimer()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld())
        return;

    StopSoundTimer();
    
    FTimerDelegate TimerDelegate;

    switch (CurrentAudioState)
    {
        case ESeekerAudioState::Idle:
            break;
            
        case ESeekerAudioState::Combat:
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
    if (CombatSoundTimerHandle.IsValid())
    {
        TimerManager.ClearTimer(CombatSoundTimerHandle);
    }
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
            return nullptr;
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

// =============================================
// 스킬 관련 기능들
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
    
    // 스킬 이벤트도 ActivePlayingIDs에서 관리 (선택적 정지를 위해)
    RegisterPlayingID(SkillEventID);
}

void UGS_SeekerAudioComponent::StopSkill()
{
    // 선택적 정지: 스킬 사운드만 정지
    if (SkillEventID != AK_INVALID_PLAYING_ID)
    {
        if (FAkAudioDevice* AkDevice = FAkAudioDevice::Get())
        {
            AkDevice->StopPlayingID(SkillEventID);
        }
        SkillEventID = AK_INVALID_PLAYING_ID;
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
    // 메르시만 화살 사운드 재생 가능
    if (!OwnerSeeker || !OwnerSeeker->IsMerci())
    {
        return;
    }

    // 이 함수는 메르시의 멀티캐스트 RPC에서 호출되므로 직접 재생
    // 근접 캐릭터(아레스, 찬)는 이 함수를 사용하지 않음
    Multicast_PlayArrowShotSound_Implementation();
}

// ===================
// 찬 전용 방패 슬램 사운드 함수 구현
// ===================

void UGS_SeekerAudioComponent::PlayShieldSlamStartSound()
{
    // 찬만 방패 슬램 사운드 재생 가능
    if (CharacterType != ECharacterType::Chan)
    {
        return;
    }
    
    if (!OwnerSeeker || !OwnerSeeker->IsChan())
    {
        return;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
        return;

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayShieldSlamStartSound();
}

void UGS_SeekerAudioComponent::PlayShieldSlamImpactSound()
{
    // 찬만 방패 슬램 사운드 재생 가능
    if (!OwnerSeeker || !OwnerSeeker->IsChan())
    {
        return;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
        return;

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayShieldSlamImpactSound();
}

// ===================
// 찬 전용 콤보 공격 사운드 함수 구현
// ===================

void UGS_SeekerAudioComponent::PlayChanComboAttackSound(int32 ComboIndex)
{
    // 찬만 찬 콤보 공격 사운드 재생 가능
    if (CharacterType != ECharacterType::Chan)
    {
        return;
    }
    
    if (!OwnerSeeker || !OwnerSeeker->IsChan())
    {
        return;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
        return;

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayChanComboAttackSound(ComboIndex);
}

void UGS_SeekerAudioComponent::PlayChanFinalAttackSound()
{
    // 찬만 찬 최종 공격 사운드 재생 가능
    if (!OwnerSeeker || !OwnerSeeker->IsChan())
    {
        return;
    }

    if (!OwnerSeeker || !ChanFinalAttackExtraSound)
    {
        return;
    }

    // 찬 전용 최종 공격 추가 사운드 재생
    AkPlayingID FinalPlayingID = UAkGameplayStatics::PostEvent(ChanFinalAttackExtraSound, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(FinalPlayingID);
}

// ===================
// 아레스 전용 콤보 공격 사운드 함수 구현
// ===================

void UGS_SeekerAudioComponent::PlayAresComboAttackSound(int32 ComboIndex)
{
    // 아레스만 아레스 콤보 공격 사운드 재생 가능
    if (CharacterType != ECharacterType::Ares)
    {
        return;
    }
    
    if (!OwnerSeeker || !OwnerSeeker->IsAres())
    {
        return;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
        return;

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayAresComboAttackSound(ComboIndex);
}

void UGS_SeekerAudioComponent::PlayAresComboAttackSoundWithExtra(int32 ComboIndex)
{
    // 기본 콤보 공격 사운드 재생
    PlayAresComboAttackSound(ComboIndex);

    // 추가 사운드 재생
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
        return;

    LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Multicast_PlayAresComboAttackSoundWithExtra(ComboIndex);
}

// ===================
// TPS 콤보 사운드 멀티캐스트 RPC 구현
// ===================

void UGS_SeekerAudioComponent::Multicast_PlayChanComboAttackSound_Implementation(int32 ComboIndex)
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
    if (!OwnerSeeker || !GetWorld())
    {
        return;
    }

    // 찬 전용 TPS 콤보 공격 사운드 재생
    if (ChanAxeSwingSound)
    {
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(ChanAxeSwingSound, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(SwingPlayingID);
    }

    if (ChanAttackVoiceSound)
    {
        AkPlayingID VoicePlayingID = UAkGameplayStatics::PostEvent(ChanAttackVoiceSound, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(VoicePlayingID);
    }

    // 콤보 정지 사운드 설정
    if (ChanAxeSwingStopEvent)
    {
        CurrentStopEvent = ChanAxeSwingStopEvent;
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(AttackSoundResetTimerHandle, this, &UGS_SeekerAudioComponent::ResetAttackSoundSequence, 1.0f, false);
        }
    }
}

void UGS_SeekerAudioComponent::Multicast_PlayAresComboAttackSound_Implementation(int32 ComboIndex)
{
    // 공통 체크 로직 사용 (콤보 사운드는 거리 체크 없이 재생)
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
    if (!OwnerSeeker || !GetWorld())
    {
        return;
    }

    // 콤보 인덱스 검증 및 변환
    int32 ArrayIndex = ValidateAndConvertComboIndex(ComboIndex, AresComboSwingSounds.Num());
    if (ArrayIndex == INDEX_NONE)
    {
        return;
    }
    
    // 아레스 전용 TPS 콤보 공격 사운드 재생
    if (AresComboSwingSounds.IsValidIndex(ArrayIndex) && AresComboSwingSounds[ArrayIndex])
    {
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(AresComboSwingSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(SwingPlayingID);
    }

    if (AresComboVoiceSounds.IsValidIndex(ArrayIndex) && AresComboVoiceSounds[ArrayIndex])
    {
        AkPlayingID VoicePlayingID = UAkGameplayStatics::PostEvent(AresComboVoiceSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(VoicePlayingID);
    }

    // 콤보 정지 사운드 설정
    if (AresSwordSwingStopEvent)
    {
        CurrentStopEvent = AresSwordSwingStopEvent;
        if (GetWorld())
        {
            static constexpr float ComboResetDelay = 1.0f; // 하드코딩 제거
            GetWorld()->GetTimerManager().SetTimer(AttackSoundResetTimerHandle, this, &UGS_SeekerAudioComponent::ResetAttackSoundSequence, ComboResetDelay, false);
        }
    }
}

void UGS_SeekerAudioComponent::Multicast_PlayAresComboAttackSoundWithExtra_Implementation(int32 ComboIndex)
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
    if (!OwnerSeeker || !GetWorld())
    {
        return;
    }

    // 추가 사운드 재생
    int32 ArrayIndex = ComboIndex - 1; // 1-based에서 0-based로 변환
    if (AresComboExtraSounds.IsValidIndex(ArrayIndex) && AresComboExtraSounds[ArrayIndex])
    {
        AkPlayingID ExtraPlayingID = UAkGameplayStatics::PostEvent(AresComboExtraSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ExtraPlayingID);
    }
}

void UGS_SeekerAudioComponent::PlayArrowTypeChangeSound()
{
    // 메르시만 화살 사운드 재생 가능
    if (!OwnerSeeker || !OwnerSeeker->IsMerci())
    {
        return;
    }

    if (!CanSendRPC()) return;
    
    Multicast_PlayArrowTypeChangeSound();
}

void UGS_SeekerAudioComponent::PlayArrowEmptySound()
{
    // 메르시만 화살 사운드 재생 가능
    if (!OwnerSeeker || !OwnerSeeker->IsMerci())
    {
        return;
    }

    if (!CanSendRPC()) return;
    
    Multicast_PlayArrowEmptySound();
}

void UGS_SeekerAudioComponent::PlayHitFeedbackSound()
{
    // 메르시만 타격 피드백 사운드 재생 가능
    if (!OwnerSeeker || !OwnerSeeker->IsMerci())
    {
        return;
    }

    if (!CanSendRPC()) return;
    
    Multicast_PlayHitFeedbackSound();
}

// ===================
// 화살 관련 멀티캐스트 RPC 구현
// ===================

void UGS_SeekerAudioComponent::Multicast_PlayArrowShotSound_Implementation()
{
    // 공통 체크 로직 사용
    if (!ShouldPlaySoundAtLocation(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // 모드별 사운드 이벤트 선택
    UAkAudioEvent* SoundToPlay = SelectSoundEventByMode(ArrowShotSound, RTSMerciArrowShotSound);
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(IsRTSMode());
        
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
    if (!OwnerSeeker || !HitFeedbackSound)
    {
        return;
    }

    // 히트 피드백 사운드는 UI 피드백이므로 거리 제한 없이 재생
    AkPlayingID FeedbackPlayingID = UAkGameplayStatics::PostEvent(HitFeedbackSound, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(FeedbackPlayingID);
}

// ===================
// 방패 슬램 사운드 멀티캐스트 RPC 구현
// ===================

void UGS_SeekerAudioComponent::Multicast_PlayShieldSlamStartSound_Implementation()
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
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
    
    // RTS 모드에서는 화면 시야각 기반 체크, TPS 모드에서는 거리 기반 체크
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
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

    UAkAudioEvent* SoundToPlay = bRTS ? RTSShieldSlamStartSound : ShieldSlamStartSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        AkPlayingID ShieldSlamPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ShieldSlamPlayingID);
    }
}

void UGS_SeekerAudioComponent::Multicast_PlayShieldSlamImpactSound_Implementation()
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
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
    
    // RTS 모드에서는 화면 시야각 기반 체크, TPS 모드에서는 거리 기반 체크
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
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

    UAkAudioEvent* SoundToPlay = bRTS ? RTSShieldSlamImpactSound : ShieldSlamImpactSound;
    if (SoundToPlay)
    {
        // RTS 모드에 따른 Distance Scaling 설정
        SetDistanceScaling(bRTS);
        
        AkPlayingID ImpactPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ImpactPlayingID);
    }
}

void UGS_SeekerAudioComponent::Multicast_PlayHurtSound_Implementation()
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
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
    
    // 피격/죽음 사운드는 중요한 피드백이므로 ViewFrustum 체크 제외
    // 화면 밖에 있는 팀원의 피격 상황도 들을 수 있어야 함
    // 거리 체크만 수행 (RTS/TPS 공통)
    if (DistanceToListener > MaxDistance)
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
        
        // 거리 기반 RTPC 설정
        const float RTPCMaxDistance = GetMaxAudioDistance();
        // RTS 모드에서는 더 넓은 거리 범위 사용
        const float EffectiveMaxDistance = bRTS ? FMath::Max(RTPCMaxDistance, 10000.0f) : RTPCMaxDistance;
        // 거리가 멀수록 볼륨이 작아지도록 역수 관계 적용
        const float DistanceRatio = FMath::Clamp(DistanceToListener / EffectiveMaxDistance, 0.0f, 1.0f);
        const float NormalizedDistance = 1.0f - DistanceRatio; // 거리가 멀수록 0에 가까워짐
        SetUnifiedRTPCValue(DistanceToPlayerRTPC, NormalizedDistance);
        
        // 사운드 재생
        AkPlayingID HurtPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        
        if (HurtPlayingID != AK_INVALID_PLAYING_ID)
        {
            RegisterPlayingID(HurtPlayingID);
        }
    }
}

void UGS_SeekerAudioComponent::Multicast_PlayDeathSound_Implementation()
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
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
    
    // 죽음 사운드는 중요한 피드백이므로 ViewFrustum 체크 제외
    // 화면 밖에 있는 팀원의 죽음도 들을 수 있어야 함
    // 거리 체크만 수행 (RTS/TPS 공통)
    if (DistanceToListener > MaxDistance)
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
        
        // 거리 기반 RTPC 설정 (통일된 시스템 사용)
        const float RTPCMaxDistance = GetMaxAudioDistance();
        const float NormalizedDistance = RTPCMaxDistance > 0.0f ? FMath::Clamp(DistanceToListener / RTPCMaxDistance, 0.0f, 1.0f) : 0.0f;
        SetUnifiedRTPCValue(DistanceToPlayerRTPC, NormalizedDistance);
        
        // 사운드 재생
        AkPlayingID DeathPlayingID = UAkGameplayStatics::PostEvent(SoundToPlay, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(DeathPlayingID);
    }
}

// ===================
// RTS 사운드 재생 함수들 구현
// ===================

void UGS_SeekerAudioComponent::PlayRTSAresSwordSwingSound(int32 ComboIndex)
{
    if (!OwnerSeeker || !OwnerSeeker->IsAres())
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 거리 및 시야각 체크
    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // 배열 인덱스 체크 (0-based)
    int32 ArrayIndex = ComboIndex - 1; // 1-based에서 0-based로 변환
    if (!RTSAresSwordSwingSounds.IsValidIndex(ArrayIndex) || !RTSAresSwordSwingSounds[ArrayIndex])
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(true);
    
    AkPlayingID RTSSwordPlayingID = UAkGameplayStatics::PostEvent(RTSAresSwordSwingSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(RTSSwordPlayingID);
}

void UGS_SeekerAudioComponent::PlayRTSAresComboVoiceSound(int32 ComboIndex)
{
    if (!OwnerSeeker || !OwnerSeeker->IsAres())
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 거리 및 시야각 체크
    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // 배열 인덱스 체크 (0-based)
    int32 ArrayIndex = ComboIndex - 1; // 1-based에서 0-based로 변환
    if (!RTSAresComboVoiceSounds.IsValidIndex(ArrayIndex) || !RTSAresComboVoiceSounds[ArrayIndex])
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(true);
    
    AkPlayingID RTSVoicePlayingID = UAkGameplayStatics::PostEvent(RTSAresComboVoiceSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(RTSVoicePlayingID);
}

void UGS_SeekerAudioComponent::PlayRTSAresComboExtraSound(int32 ComboIndex)
{
    if (!OwnerSeeker || !OwnerSeeker->IsAres())
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 거리 및 시야각 체크
    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // 배열 인덱스 체크 (0-based)
    int32 ArrayIndex = ComboIndex - 1; // 1-based에서 0-based로 변환
    if (!RTSAresComboExtraSounds.IsValidIndex(ArrayIndex) || !RTSAresComboExtraSounds[ArrayIndex])
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(true);
    
    AkPlayingID RTSExtraPlayingID = UAkGameplayStatics::PostEvent(RTSAresComboExtraSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(RTSExtraPlayingID);
}

// ===================
// RTS 콤보 사운드 통합 함수들 구현
// ===================



void UGS_SeekerAudioComponent::PlayRTSAresComboAttackSound(int32 ComboIndex)
{
    // 아레스만 아레스 RTS 콤보 공격 사운드 재생 가능
    if (CharacterType != ECharacterType::Ares)
    {
        return;
    }
    
    if (!OwnerSeeker || !OwnerSeeker->IsAres())
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }
    
    if (!OwnerSeeker || !GetWorld())
    {
        return;
    }

    // 아레스 전용 RTS 콤보 공격 사운드 재생
    int32 ArrayIndex = ComboIndex - 1; // 1-based에서 0-based로 변환
    
    if (RTSAresSwordSwingSounds.IsValidIndex(ArrayIndex) && RTSAresSwordSwingSounds[ArrayIndex])
    {
        AkPlayingID SwingPlayingID = UAkGameplayStatics::PostEvent(RTSAresSwordSwingSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(SwingPlayingID);
    }

    if (RTSAresComboVoiceSounds.IsValidIndex(ArrayIndex) && RTSAresComboVoiceSounds[ArrayIndex])
    {
        AkPlayingID VoicePlayingID = UAkGameplayStatics::PostEvent(RTSAresComboVoiceSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(VoicePlayingID);
    }
}

void UGS_SeekerAudioComponent::PlayRTSAresComboAttackSoundWithExtra(int32 ComboIndex)
{
    // 기본 RTS 콤보 공격 사운드 재생
    PlayRTSAresComboAttackSound(ComboIndex);

    // 추가 사운드 재생
    int32 ArrayIndex = ComboIndex - 1; // 1-based에서 0-based로 변환
    if (RTSAresComboExtraSounds.IsValidIndex(ArrayIndex) && RTSAresComboExtraSounds[ArrayIndex])
    {
        AkPlayingID ExtraPlayingID = UAkGameplayStatics::PostEvent(RTSAresComboExtraSounds[ArrayIndex], OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ExtraPlayingID);
    }
}

// ===================
// 찬 전용 RTS 공격 사운드 함수 구현
// ===================

void UGS_SeekerAudioComponent::PlayRTSChanAttackSound()
{
    if (!OwnerSeeker || !OwnerSeeker->IsChan() || !RTSChanAxeSwingSound)
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 거리 및 시야각 체크
    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(true);
    
    // 도끼 휘두르기 사운드 재생
    AkPlayingID AxePlayingID = UAkGameplayStatics::PostEvent(RTSChanAxeSwingSound, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(AxePlayingID);

    // 공격 음성 사운드 재생
    if (RTSChanAttackVoiceSound)
    {
        AkPlayingID VoicePlayingID = UAkGameplayStatics::PostEvent(RTSChanAttackVoiceSound, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(VoicePlayingID);
    }
}

void UGS_SeekerAudioComponent::PlayRTSChanShieldSlamSound()
{
    if (!OwnerSeeker || !OwnerSeeker->IsChan())
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 거리 및 시야각 체크
    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(true);
    
    // 방패 슬램 시작 사운드 재생
    if (RTSShieldSlamStartSound)
    {
        AkPlayingID StartPlayingID = UAkGameplayStatics::PostEvent(RTSShieldSlamStartSound, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(StartPlayingID);
    }

    // 방패 슬램 충돌 사운드 재생
    if (RTSShieldSlamImpactSound)
    {
        AkPlayingID ImpactPlayingID = UAkGameplayStatics::PostEvent(RTSShieldSlamImpactSound, OwnerSeeker, 0, FOnAkPostEventCallback());
        RegisterPlayingID(ImpactPlayingID);
    }
}

void UGS_SeekerAudioComponent::PlayRTSMerciBowDrawSound()
{
    if (!OwnerSeeker || !OwnerSeeker->IsMerci() || !RTSMerciBowDrawSound)
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 거리 및 시야각 체크
    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(true);
    
    AkPlayingID RTSBowPlayingID = UAkGameplayStatics::PostEvent(RTSMerciBowDrawSound, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(RTSBowPlayingID);
}

void UGS_SeekerAudioComponent::PlayRTSMerciArrowShotSound()
{
    if (!OwnerSeeker || !OwnerSeeker->IsMerci() || !RTSMerciArrowShotSound)
    {
        return;
    }

    // RTS 모드에서만 재생
    if (!IsRTSMode())
    {
        return;
    }

    // 거리 및 시야각 체크
    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return;
    }

    if (!IsInViewFrustum(OwnerSeeker->GetActorLocation()))
    {
        return;
    }

    // RTS 모드에 따른 Distance Scaling 설정
    SetDistanceScaling(true);
    
    AkPlayingID RTSArrowPlayingID = UAkGameplayStatics::PostEvent(RTSMerciArrowShotSound, OwnerSeeker, 0, FOnAkPostEventCallback());
    RegisterPlayingID(RTSArrowPlayingID);
}

// ===================
// 공통 헬퍼 함수들 구현
// ===================

bool UGS_SeekerAudioComponent::ShouldPlaySoundAtLocation(const FVector& SourceLocation, bool bSkipViewFrustumCheck) const
{
    // 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return false;
    }
    
    if (!OwnerSeeker || !GetWorld())
    {
        return false;
    }

    FVector ListenerLocation;
    if (!GetListenerLocation(ListenerLocation))
    {
        return false;
    }

    // RTS 모드와 TPS 모드에 따른 거리 체크
    const bool bRTS = IsRTSMode();
    const float MaxDistance = GetMaxDistanceForMode(bRTS);
    
    const float DistanceToListener = FVector::Dist(SourceLocation, ListenerLocation);
    
    // 모드별 체크 로직
    if (bRTS)
    {
        // RTS 모드: View Frustum 체크 (화면에 보이는지 확인)
        if (!bSkipViewFrustumCheck && !IsInViewFrustum(SourceLocation))
        {
            return false;
        }
    }
    else
    {
        // TPS 모드: 기존 거리 기반 체크
        if (DistanceToListener > MaxDistance)
        {
            return false;
        }
    }
    
    return true;
}

int32 UGS_SeekerAudioComponent::ValidateAndConvertComboIndex(int32 ComboIndex, int32 ArraySize) const
{
    // 1-based에서 0-based로 변환
    int32 ArrayIndex = ComboIndex - ComboIndexOffset;
    
    // 유효성 검사
    if (ArrayIndex < 0 || ArrayIndex >= ArraySize)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SeekerAudio] Invalid combo index: %d (Array size: %d)"), ComboIndex, ArraySize);
        return INDEX_NONE;
    }
    
    return ArrayIndex;
}

UAkAudioEvent* UGS_SeekerAudioComponent::SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound, bool bUseRTSMode) const
{
    const bool bRTS = bUseRTSMode || IsRTSMode();
    
    if (bRTS)
    {
        // RTS 사운드가 있으면 사용, 없으면 TPS 사운드로 폴백
        return RTSSound ? RTSSound : TPSSound;
    }
    
    return TPSSound;
}