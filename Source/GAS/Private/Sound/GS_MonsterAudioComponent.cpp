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

// 디버그 토글용 콘솔 변수 추가
static TAutoConsoleVariable<bool> CVarShowMonsterAudioDebug(
    TEXT("gs.ShowMonsterAudioDebug"),
    false,
    TEXT("몬스터 오디오 시스템 디버그 정보 표시 여부"),
    ECVF_Default
);

UGS_MonsterAudioComponent::UGS_MonsterAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 0.5초마다 틱
    
    CurrentAudioState = EMonsterAudioState::Idle;
    PreviousAudioState = EMonsterAudioState::Idle;
    LastSoundPlayTime = 0.0f;
    CurrentPlayingID = AK_INVALID_PLAYING_ID;
    
    // 기본 설정값
    AudioConfig.AlertDistance = 800.0f;
    AudioConfig.MaxAudioDistance = 1000.0f;
    IdleSoundInterval = 6.0f;
    CombatSoundInterval = 4.0f;
    
    MonsterSoundVariant = 1;
}

void UGS_MonsterAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 몬스터 참조 가져오기
    OwnerMonster = Cast<AGS_Monster>(GetOwner());
    if (!OwnerMonster)
    {
        UE_LOG(LogTemp, Warning, TEXT("GS_MonsterAudioComponent: Owner is not a Monster!"));
        return;
    }
    
    // 초기 사운드 타이머 시작
    StartSoundTimer();
}

void UGS_MonsterAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSoundTimer();
    
    // 현재 재생 중인 사운드 중지
    if (CurrentPlayingID != AK_INVALID_PLAYING_ID)
    {
        // AkAudioDevice를 통해 StopPlayingID 호출
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
        
    // 몬스터 상태 변화 확인
    CheckForStateChanges();
    
    // 가까운 시커가 있으면 거리에 따른 RTPC 업데이트
    float Distance = CalculateDistanceToNearestSeeker();
    if (Distance > 0.0f && Distance <= AudioConfig.MaxAudioDistance)
    {
        SetDistanceRTPC(Distance);
    }

#if WITH_EDITOR
    // 에디터에서 디버그 정보 표시 (CVars로 제어 가능하도록 수정)
    if (CVarShowMonsterAudioDebug.GetValueOnGameThread() && GetWorld() && GetWorld()->IsGameWorld())
    {
        DrawDebugInfo();
    }
#endif
}

void UGS_MonsterAudioComponent::SetMonsterAudioState(EMonsterAudioState NewState)
{
    if (CurrentAudioState == NewState)
        return;
        
    PreviousAudioState = CurrentAudioState;
    CurrentAudioState = NewState;
    
    UE_LOG(LogTemp, Log, TEXT("Monster Audio State Changed: %s -> %s"), 
           *UEnum::GetValueAsString(PreviousAudioState), 
           *UEnum::GetValueAsString(CurrentAudioState));
    
    // 상태 변화에 따른 타이머 업데이트
    UpdateSoundTimer();
    
    // 즉시 새 상태의 사운드 재생 (특정 조건에서)
    if (NewState == EMonsterAudioState::Combat)
    {
        PlaySound(NewState, true);
    }
}

void UGS_MonsterAudioComponent::PlaySound(EMonsterAudioState SoundType, bool bForcePlay)
{
    if (!OwnerMonster)
        return;
        
    // 너무 자주 재생되는 것을 방지
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (!bForcePlay && (CurrentTime - LastSoundPlayTime) < 1.0f)
        return;
    
    // 성능 최적화: 너무 멀면 아예 재생하지 않음
    float Distance = CalculateDistanceToNearestSeeker();
    if (Distance > AudioConfig.MaxAudioDistance)
        return;
    
    UAkAudioEvent* SoundEvent = GetSoundEvent(SoundType);
    if (!SoundEvent)
        return;
    
    // Wwise 이벤트 재생 (거리 감쇠는 Wwise Attenuation이 자동 처리)
    PostWwiseEvent(SoundEvent);
    
    LastSoundPlayTime = CurrentTime;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Monster %s played %s sound at distance %.1f (Wwise handles all distance effects)"), 
           *OwnerMonster->GetName(), 
           *UEnum::GetValueAsString(SoundType), 
           Distance);
}

void UGS_MonsterAudioComponent::PlayHurtSound()
{
    SetMonsterAudioState(EMonsterAudioState::Hurt);
    PlaySound(EMonsterAudioState::Hurt, true);
}

void UGS_MonsterAudioComponent::PlayDeathSound()
{
    SetMonsterAudioState(EMonsterAudioState::Death);
    PlaySound(EMonsterAudioState::Death, true);
}

AGS_Seeker* UGS_MonsterAudioComponent::FindNearestSeeker() const
{
    if (!GetWorld() || !OwnerMonster)
        return nullptr;
    
    AGS_Seeker* NearestSeeker = nullptr;
    float NearestDistance = FLT_MAX;
    FVector MonsterLocation = OwnerMonster->GetActorLocation();
    
    // 월드의 모든 시커를 찾아서 가장 가까운 것을 반환
    for (TActorIterator<AGS_Seeker> SeekerIterator(GetWorld()); SeekerIterator; ++SeekerIterator)
    {
        AGS_Seeker* Seeker = *SeekerIterator;
        if (!Seeker || !IsValid(Seeker))
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
    AGS_Seeker* NearestSeeker = FindNearestSeeker();
    if (!NearestSeeker || !OwnerMonster)
        return -1.0f;
    
    return FVector::Dist(OwnerMonster->GetActorLocation(), NearestSeeker->GetActorLocation());
}

void UGS_MonsterAudioComponent::SetDistanceRTPC(float Distance)
{
    if (!OwnerMonster)
        return;
    
    // FAkAudioDevice를 통해 RTPC 값 설정
    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        // "Distance_to_Player" RTPC에 거리 값 설정
        AKRESULT Result = AkAudioDevice->SetRTPCValue(
            TEXT("Distance_to_Player"), // const TCHAR* 문자열 리터럴
            Distance, 
            0, // 즉시 적용 (interpolation time = 0)
            OwnerMonster
        );
        
        if (Result != AK_Success)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("Distance RTPC 설정 실패: %d"), (int32)Result);
        }
        
        // 몬스터 타입별 사운드 변화
        if (MonsterSoundVariant > 0)
        {
            AkAudioDevice->SetRTPCValue(
                TEXT("Monster_Variant"), // const TCHAR* 문자열 리터럴
                MonsterSoundVariant, 
                0, 
                OwnerMonster
            );
        }
    }
}

void UGS_MonsterAudioComponent::PlayIdleSound()
{
    PlaySound(EMonsterAudioState::Idle);
}

void UGS_MonsterAudioComponent::PlayCombatSound()
{
    PlaySound(EMonsterAudioState::Combat);
}

void UGS_MonsterAudioComponent::StartSoundTimer()
{
    StopSoundTimer(); // 기존 타이머 정리
    
    if (!GetWorld())
        return;
    
    // 현재 상태에 따른 타이머 설정
    switch (CurrentAudioState)
    {
        case EMonsterAudioState::Idle:
            GetWorld()->GetTimerManager().SetTimer(
                IdleSoundTimer, 
                this, 
                &UGS_MonsterAudioComponent::PlayIdleSound, 
                IdleSoundInterval,
                true
            );
            break;
            
        case EMonsterAudioState::Combat:
            GetWorld()->GetTimerManager().SetTimer(
                CombatSoundTimer,
                this, 
                &UGS_MonsterAudioComponent::PlayCombatSound, 
                CombatSoundInterval,
                true
            );
            break;
            
        default:
            break;
    }
}

void UGS_MonsterAudioComponent::StopSoundTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(IdleSoundTimer);
        GetWorld()->GetTimerManager().ClearTimer(CombatSoundTimer);
    }
}

void UGS_MonsterAudioComponent::UpdateSoundTimer()
{
    StartSoundTimer(); // 새로운 상태에 맞는 타이머 시작
}

UAkAudioEvent* UGS_MonsterAudioComponent::GetSoundEvent(EMonsterAudioState SoundType) const
{
    switch (SoundType)
    {
        case EMonsterAudioState::Idle:
            return AudioConfig.IdleSound;
        case EMonsterAudioState::Combat:
            return AudioConfig.CombatSound;
        case EMonsterAudioState::Hurt:
            return AudioConfig.HurtSound;
        case EMonsterAudioState::Death:
            return AudioConfig.DeathSound;
        default:
            return nullptr;
    }
}

void UGS_MonsterAudioComponent::PostWwiseEvent(UAkAudioEvent* Event)
{
    if (!Event || !OwnerMonster)
        return;
    
    // 현재 재생 중인 같은 타입의 사운드 중지 (중복 방지)
    if (CurrentPlayingID != AK_INVALID_PLAYING_ID)
    {
        if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
        {
            AkAudioDevice->StopPlayingID(CurrentPlayingID);
        }
    }
    
    // Wwise 이벤트 재생
    CurrentPlayingID = UAkGameplayStatics::PostEvent(Event, OwnerMonster, 0, FOnAkPostEventCallback());
}

void UGS_MonsterAudioComponent::CheckForStateChanges()
{
    if (!OwnerMonster)
        return;
    
    // 몬스터가 죽었다면 Death 상태
    if (OwnerMonster->GetStatComp() && OwnerMonster->GetStatComp()->GetCurrentHealth() <= 0.0f)
    {
        if (CurrentAudioState != EMonsterAudioState::Death)
        {
            SetMonsterAudioState(EMonsterAudioState::Death);
            return;
        }
    }
    
    // Combat/Idle 상태 전환
    float DistanceToSeeker = CalculateDistanceToNearestSeeker();
    
    if (DistanceToSeeker > 0.0f)
    {
        if (DistanceToSeeker <= AudioConfig.AlertDistance && CurrentAudioState == EMonsterAudioState::Idle)
        {
            // 시커가 경계 거리 안에 들어오면 Combat
            SetMonsterAudioState(EMonsterAudioState::Combat);
        }
        else if (DistanceToSeeker > AudioConfig.AlertDistance && CurrentAudioState == EMonsterAudioState::Combat)
        {
            // 시커가 경계 거리 밖으로 나가면 Idle
            SetMonsterAudioState(EMonsterAudioState::Idle);
        }
    }
}

void UGS_MonsterAudioComponent::DrawDebugInfo() const
{
#if WITH_EDITOR
    if (!OwnerMonster || !GetWorld())
        return;
    
    FVector MonsterLocation = OwnerMonster->GetActorLocation();
    
    // 게임 로직용 경계 거리 - 지속시간을 0.6초로 설정하여 깜빡임 방지
    DrawDebugCircle(GetWorld(), MonsterLocation, AudioConfig.AlertDistance, 32, FColor::Red, false, 0.6f, 0, 3.0f, FVector(0, 1, 0), FVector(1, 0, 0));
    
    // 성능 최적화용 최대 거리 - 지속시간을 0.6초로 설정
    DrawDebugCircle(GetWorld(), MonsterLocation, AudioConfig.MaxAudioDistance, 32, FColor::Yellow, false, 0.6f, 0, 1.0f, FVector(0, 1, 0), FVector(1, 0, 0));
    
    // 현재 상태 텍스트 - 지속시간을 0.6초로 설정
    FString StateText = FString::Printf(TEXT("Audio State: %s"), *UEnum::GetValueAsString(CurrentAudioState));
    DrawDebugString(GetWorld(), MonsterLocation + FVector(0, 0, 200), StateText, nullptr, FColor::White, 0.6f);
    
    // 가장 가까운 시커와의 거리 - 지속시간을 0.6초로 설정
    // float Distance = CalculateDistanceToNearestSeeker();
    // if (Distance > 0.0f)
    // {
    //     FString DistanceText = FString::Printf(TEXT("Distance to Seeker: %.1f"), Distance);
    //     DrawDebugString(GetWorld(), MonsterLocation + FVector(0, 0, 180), DistanceText, nullptr, FColor::Cyan, 0.6f);
    //     
    //     FString InfoText = FString::Printf(TEXT("Alert: %.0f | Max: %.0f"), AudioConfig.AlertDistance, AudioConfig.MaxAudioDistance);
    //     DrawDebugString(GetWorld(), MonsterLocation + FVector(0, 0, 160), InfoText, nullptr, FColor::Green, 0.6f);
    //     
    //     // 시커와의 연결선 추가
    //     AGS_Seeker* NearestSeeker = FindNearestSeeker();
    //     if (NearestSeeker)
    //     {
    //         FColor LineColor = Distance <= AudioConfig.AlertDistance ? FColor::Orange : FColor::Green;
    //         DrawDebugLine(GetWorld(), MonsterLocation, NearestSeeker->GetActorLocation(), LineColor, false, 0.6f, 0, 2.0f);
    //     }
    // }
#endif
} 