#include "Sound/GS_AudioComponentBase.h"
#include "AkAudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "AI/RTS/GS_RTSController.h"
#include "AI/RTS/GS_RTSCamera.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"
#include "Props/GS_RoomBase.h"
#include "AkComponent.h"
#include "Engine/OverlapResult.h"


UGS_AudioComponentBase::UGS_AudioComponentBase()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentPlayingID = AK_INVALID_PLAYING_ID;
    ActivePlayingIDs.Empty();
    LastMulticastTime = DefaultInitTime;
    LastDistanceRTPCValue = -1.0f;
    LastRTPCUpdateTime = DefaultInitTime;
    bIsRTSModeCached = false;
    bIsAudioComponentInitialized = false;

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

    // 재시도 카운터 초기화
    AudioInitRetryCount = 0;

    // Seamless Travel 대응: World가 완전히 준비될 때까지 대기 후 초기화
    // 즉시 초기화 시도
    if (!InitializeAudioSystem())
    {
        // 실패 시 다음 프레임에 재시도 (Seamless Travel 중일 가능성)
        if (UWorld* World = GetWorld())
        {
            // 멤버 변수 사용 (지역 변수 금지!)
            World->GetTimerManager().SetTimer(
                RetryInitTimerHandle,
                this,
                &UGS_AudioComponentBase::RetryAudioInitialization,
                0.1f,  // 100ms 후 재시도
                false
            );
        }
    }
}

void UGS_AudioComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 모든 타이머 안전하게 정리 (레벨 전환 대응)
    SafeClearTimer(DistanceCheckTimerHandle);
    SafeClearTimer(RetryInitTimerHandle);

    // 재시도 카운터 리셋
    AudioInitRetryCount = 0;

    // 모든 활성 사운드 중지
    StopAllActiveSounds();

    // AkComponent 정리
    if (CachedAkComponent && IsValid(CachedAkComponent))
    {
        CachedAkComponent->Stop();
        CachedAkComponent = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

// ==========================
// Transform 검증 함수들
// ==========================

bool UGS_AudioComponentBase::IsTransformValid(const FVector& Location, const FRotator& Rotation)
{
    // NaN 체크
    if (Location.ContainsNaN())
    {
        return false;
    }

    if (Rotation.ContainsNaN())
    {
        return false;
    }

    // 개별 컴포넌트 Infinity 체크
    if (!FMath::IsFinite(Location.X) || !FMath::IsFinite(Location.Y) || !FMath::IsFinite(Location.Z))
    {
        return false;
    }

    if (!FMath::IsFinite(Rotation.Pitch) || !FMath::IsFinite(Rotation.Yaw) || !FMath::IsFinite(Rotation.Roll))
    {
        return false;
    }

    return true;
}

bool UGS_AudioComponentBase::IsLocationValid(const FVector& Location)
{
    // NaN 체크
    if (Location.ContainsNaN())
    {
        return false;
    }

    // 개별 컴포넌트 Infinity 체크
    if (!FMath::IsFinite(Location.X) || !FMath::IsFinite(Location.Y) || !FMath::IsFinite(Location.Z))
    {
        return false;
    }

    return true;
}

bool UGS_AudioComponentBase::IsWorldContextValid() const
{
    UWorld* World = GetWorld();
    return World &&
           World->IsValidLowLevel() &&
           !World->bIsTearingDown &&
           IsValid(World) &&
           IsValid(this);
}

bool UGS_AudioComponentBase::SafeUpdateAkComponentTransform(UAkComponent* AkComp, const FVector& NewLocation, const FRotator& NewRotation)
{
    // 컴포넌트 유효성 체크
    if (!IsValid(AkComp))
    {
        return false;
    }

    // World 컨텍스트 체크
    if (!IsWorldContextValid())
    {
        return false;
    }

    // Transform 유효성 체크
    if (!IsTransformValid(NewLocation, NewRotation))
    {
        UE_LOG(LogTemp, Error, TEXT("[GS_AudioComponentBase] Invalid Transform detected - Location: %s, Rotation: %s (Owner: %s)"),
               *NewLocation.ToString(), *NewRotation.ToString(),
               GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
        return false;
    }

    // 안전하게 Transform 업데이트
    AkComp->SetWorldLocationAndRotation(NewLocation, NewRotation);
    return true;
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
    FRotator DummyRotation;
    return GetListenerTransform(OutLocation, DummyRotation);
}

bool UGS_AudioComponentBase::GetListenerTransform(FVector& OutLocation, FRotator& OutRotation) const
{
    OutLocation = FVector::ZeroVector;
    OutRotation = FRotator::ZeroRotator;

    if (!GetWorld())
    {
        return false;
    }

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
        FVector CameraLocation;
        FRotator CameraRotation;
        if (GetActualCameraTransform(CameraLocation, CameraRotation))
        {
            OutLocation = CameraLocation;
            OutRotation = CameraRotation;
            return true;
        }

        if (AActor* ViewTarget = RTSController->GetViewTarget())
        {
            OutLocation = ViewTarget->GetActorLocation();
            OutRotation = ViewTarget->GetActorRotation();
            return true;
        }

        if (LocalPC->PlayerCameraManager)
        {
            OutLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
            OutRotation = LocalPC->PlayerCameraManager->GetCameraRotation();
            return true;
        }
    }
    else
    {
        if (LocalPC->PlayerCameraManager)
        {
            OutLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
            OutRotation = LocalPC->PlayerCameraManager->GetCameraRotation();
            return true;
        }

        if (APawn* PlayerPawn = LocalPC->GetPawn())
        {
            OutLocation = PlayerPawn->GetActorLocation();
            OutRotation = PlayerPawn->GetActorRotation();
            return true;
        }
    }

    return false;
}

bool UGS_AudioComponentBase::IsInViewFrustum(const FVector& SourceLocation) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!LocalPC)
    {
        return true;
    }
    
    // RTS 모드 체크
    if (AGS_RTSController* RTSController = Cast<AGS_RTSController>(LocalPC))
    {
        // 새로운 화면 투영 기반 체크 사용
        return IsSourceVisibleOnScreen(RTSController, SourceLocation);
    }
    
    // TPS 모드: 카메라 앞쪽 영역에서만 들리도록 개선
    FVector CameraLocation;
    FRotator CameraRotation;
    if (GetActualCameraTransform(CameraLocation, CameraRotation))
    {
        const FVector DirectionToSource = (SourceLocation - CameraLocation).GetSafeNormal();
        const FVector CameraForwardVector = CameraRotation.Vector();
        
        // 1. 카메라 뒤에 있는 경우 필터링 (dot product < 0)
        const float DotProduct = FVector::DotProduct(DirectionToSource, CameraForwardVector);
        if (DotProduct < 0.0f)
        {
            return false; // 카메라 뒤의 소리는 들리지 않음
        }
        
        // 2. 시야각(FOV) 기반 필터링: 약 90도 이상 옆쪽의 소리는 감쇠
        // 기본 FOV 약 90도를 기준으로, cos(90도) = 0 인 지점부터 필터링 시작
        const float FOVThreshold = 0.0f; // cos(90도)
        if (DotProduct < FOVThreshold)
        {
            return false; // 90도 이상 옆쪽은 들리지 않음
        }
    }
    
    // TPS 모드: 기본 거리 체크만 수행
    return true;
}


bool UGS_AudioComponentBase::IsSourceVisibleOnScreen(AGS_RTSController* RTSController, const FVector& SourceLocation) const
{
    if (!RTSController)
    {
        return true; // RTS 모드가 아니면 항상 들리는 것으로 처리
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    if (!GetActualCameraTransform(CameraLocation, CameraRotation))
    {
        return true; // 카메라 정보를 얻을 수 없으면, 안전하게 true 반환
    }

    const FVector DirectionToSource = (SourceLocation - CameraLocation).GetSafeNormal();
    const FVector CameraForwardVector = CameraRotation.Vector();
    const float Distance = FVector::Dist(CameraLocation, SourceLocation);

    // 1. 근접 체크: 매우 가까우면 항상 들리도록 처리
    if (Distance <= 800.0f) // 8미터
    {
        return true;
    }

    // 2. 카메라 뒤에 있는 객체 필터링 (중요: 카메라 앞에 있는지 확인)
    // dot product > 0 이면 카메라 앞, < 0 이면 카메라 뒤
    const float DotProduct = FVector::DotProduct(DirectionToSource, CameraForwardVector);
    
    if (DotProduct < 0.0f)
    {
        // 카메라 뒤에 있는 경우 - 방 체크만 수행
        AActor* ViewTarget = RTSController->GetViewTarget();
        if (ViewTarget)
        {
            if (IsInSameRoom(ViewTarget->GetActorLocation(), SourceLocation))
            {
                // 같은 방 시스템 내에 있다면, 제한된 거리로 허용
                if (Distance <= 2000.0f) // 카메라 뒤쪽: 20미터
                {
                    return true;
                }
            }
        }
        return false;
    }

    // 3. 화면 내 가시성 체크: 화면에 보이면 소리가 들려야 함
    FVector2D ScreenPosition;
    // ProjectWorldLocationToScreen은 위치가 카메라 앞에 있고 화면에 투영되면 true를 반환
    if (RTSController->ProjectWorldLocationToScreen(SourceLocation, ScreenPosition, false))
    {
        FVector2D ViewportSize;
        if (GEngine && GEngine->GameViewport)
        {
            GEngine->GameViewport->GetViewportSize(ViewportSize);

            // 뷰포트 경계 내에 있는지 확인 (약간의 여유분 포함)
            // 화면 가장자리의 소리도 들을 수 있도록 마진 설정
            const float Margin = 200.0f; // 200픽셀 여유 (화면 밖의 가까운 소리도 포함)
            if (ScreenPosition.X >= -Margin && ScreenPosition.X <= ViewportSize.X + Margin &&
                ScreenPosition.Y >= -Margin && ScreenPosition.Y <= ViewportSize.Y + Margin)
            {
                // 화면에 보이므로 소리가 들려야 함
                return true;
            }
        }
        else
        {
            // 뷰포트가 없는 경우(예: 에디터), 안전하게 true 반환
            return true;
        }
    }

    // 4. 방 체크: 화면에 보이지 않지만 시야각 내에 있는 경우
    AActor* ViewTarget = RTSController->GetViewTarget();
    if (ViewTarget)
    {
        if (IsInSameRoom(ViewTarget->GetActorLocation(), SourceLocation))
        {
            // 같은 방 시스템 내에 있다면, 더 먼 거리의 소리도 허용
            if (Distance <= 3000.0f) // 같은/연결된 방일 경우 30미터
            {
                return true;
            }
        }
    }

    // 5. 위의 모든 조건에 해당하지 않으면 소리가 들리지 않음
    return false;
}

bool UGS_AudioComponentBase::GetActualCameraLocation(FVector& OutLocation) const
{
    FRotator DummyRotation;
    return GetActualCameraTransform(OutLocation, DummyRotation);
}

bool UGS_AudioComponentBase::GetActualCameraTransform(FVector& OutLocation, FRotator& OutRotation) const
{
    OutLocation = FVector::ZeroVector;
    OutRotation = FRotator::ZeroRotator;

    if (!GetWorld())
    {
        return false;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    if (!CachedCameraLocation.IsZero() &&
        (CurrentTime - LastCameraLocationUpdateTime) < CameraLocationUpdateInterval)
    {
        OutLocation = CachedCameraLocation;
        OutRotation = CachedCameraRotation;
        return true;
    }

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
        AGS_RTSCamera* RTSCameraActor = nullptr;

        if (CachedRTSCamera.IsValid())
        {
            RTSCameraActor = CachedRTSCamera.Get();
        }
        else
        {
            for (TActorIterator<AGS_RTSCamera> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
            {
                AGS_RTSCamera* FoundCamera = *ActorIterator;
                if (FoundCamera && IsValid(FoundCamera))
                {
                    CachedRTSCamera = FoundCamera;
                    RTSCameraActor = FoundCamera;
                    break;
                }
            }
        }

        if (RTSCameraActor && IsValid(RTSCameraActor))
        {
            if (IsTransformValid(RTSCameraActor->GetActorLocation(), RTSCameraActor->GetActorRotation()))
            {
                CachedCameraLocation = RTSCameraActor->GetActorLocation();
                CachedCameraRotation = RTSCameraActor->GetActorRotation();
                LastCameraLocationUpdateTime = CurrentTime;
                OutLocation = CachedCameraLocation;
                OutRotation = CachedCameraRotation;
                return true;
            }
        }

        if (LocalPC->PlayerCameraManager)
        {
            CachedCameraLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
            CachedCameraRotation = LocalPC->PlayerCameraManager->GetCameraRotation();
            LastCameraLocationUpdateTime = CurrentTime;
            OutLocation = CachedCameraLocation;
            OutRotation = CachedCameraRotation;
            return true;
        }

        if (AActor* ViewTarget = LocalPC->GetViewTarget())
        {
            CachedCameraLocation = ViewTarget->GetActorLocation();
            CachedCameraRotation = ViewTarget->GetActorRotation();
            LastCameraLocationUpdateTime = CurrentTime;
            OutLocation = CachedCameraLocation;
            OutRotation = CachedCameraRotation;
            return true;
        }
    }
    else
    {
        if (LocalPC->PlayerCameraManager)
        {
            CachedCameraLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
            CachedCameraRotation = LocalPC->PlayerCameraManager->GetCameraRotation();
            LastCameraLocationUpdateTime = CurrentTime;
            OutLocation = CachedCameraLocation;
            OutRotation = CachedCameraRotation;
            return true;
        }

        if (APawn* Pawn = LocalPC->GetPawn())
        {
            CachedCameraLocation = Pawn->GetActorLocation();
            CachedCameraRotation = Pawn->GetActorRotation();
            LastCameraLocationUpdateTime = CurrentTime;
            OutLocation = CachedCameraLocation;
            OutRotation = CachedCameraRotation;
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

bool UGS_AudioComponentBase::ValidateServerRPCCall() const
{
    // 컴포넌트 유효성 검증
    if (!IsValid(this))
    {
        return false;
    }

    // 월드 컨텍스트 유효성 검증
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
    {
        return false;
    }

    // 오너 유효성 검증
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !Owner->HasAuthority())
    {
        return false;
    }

    // RPC 호출 빈도 체크
    if (!CanSendRPC())
    {
        return false;
    }

    return true;
}

void UGS_AudioComponentBase::CleanupFinishedSounds()
{
    if (!FAkAudioDevice::Get()) return;
    
    // 잘못된 ID들 제거
    ActivePlayingIDs.RemoveAll([](AkPlayingID ID) {
        return ID == AK_INVALID_PLAYING_ID;
    });
    
    // 배열 크기 제한 - 가장 오래된 항목부터 제거
    if (ActivePlayingIDs.Num() > MaxActivePlayingIDs)
    {
        int32 ItemsToRemove = ActivePlayingIDs.Num() - MaxActivePlayingIDs;
        
        for (int32 i = 0; i < ItemsToRemove; ++i)
        {
            AkPlayingID OldID = ActivePlayingIDs[0];
            ActivePlayingIDs.RemoveAt(0);
            
            if (CurrentPlayingID == OldID)
            {
                CurrentPlayingID = ActivePlayingIDs.Num() > 0 ? ActivePlayingIDs.Last() : AK_INVALID_PLAYING_ID;
            }
            
            // 하위 클래스 정리 처리
            OnSpecificSoundFinished(OldID);
        }
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
        
        // 주기적 정리
        CleanupFinishedSounds();
    }
}

AkPlayingID UGS_AudioComponentBase::PostEventWithCallback(UAkAudioEvent* AkEvent, AActor* Actor)
{
    if (!AkEvent || !Actor) return AK_INVALID_PLAYING_ID;
    
    AkPlayingID PlayingID = UAkGameplayStatics::PostEvent(AkEvent, Actor, 0, FOnAkPostEventCallback());
    
    // ID 등록
    RegisterPlayingID(PlayingID);
    
    return PlayingID;
}

void UGS_AudioComponentBase::UpdateDistanceRTPC()
{
    // World 및 Owner 유효성 체크 (서버 안정성)
    if (!GetWorld() || !IsWorldContextValid())
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner || !IsValid(Owner))
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const bool bCurrentRTSMode = IsRTSMode();

    // 오디오 컴포넌트가 처음 초기화되거나, 게임 모드(RTS/TPS)가 변경되었을 때만 Distance Scaling을 설정합니다.
    if (!bIsAudioComponentInitialized || bCurrentRTSMode != bIsRTSModeCached)
    {
        SetDistanceScaling(bCurrentRTSMode);
        bIsRTSModeCached = bCurrentRTSMode;
        bIsAudioComponentInitialized = true;
    }

    FVector ListenerLocation;
    if (GetListenerLocation(ListenerLocation))
    {
        // Owner 위치 검증
        const FVector OwnerLocation = Owner->GetActorLocation();
        if (!IsLocationValid(OwnerLocation) || !IsLocationValid(ListenerLocation))
        {
            return;
        }

        float DistanceToListener = FVector::Dist(OwnerLocation, ListenerLocation);

        // 거리 값 검증 (NaN/Infinity 체크)
        if (!FMath::IsFinite(DistanceToListener))
        {
            return;
        }

        if (DistanceToListener <= GetMaxAudioDistance())
        {
            // RTPC 업데이트 최적화 - 거리 차이가 임계값 이상이거나 일정 시간 경과 시만 업데이트
            if (ShouldUpdateRTPC(DistanceToListener, CurrentTime))
            {
                // 거리를 0-1 범위로 정규화하여 통일된 RTPC 시스템 사용
                const float MaxDistance = GetMaxAudioDistance();
                const float NormalizedDistance = MaxDistance > 0.0f ? FMath::Clamp(DistanceToListener / MaxDistance, 0.0f, 1.0f) : 0.0f;
                
                SetUnifiedRTPCValue(DistanceToPlayerRTPC, NormalizedDistance);
                LastDistanceRTPCValue = DistanceToListener;
                LastRTPCUpdateTime = CurrentTime;
            }
        }
    }

    // 서버에서만 상태 변경 체크
    if (Owner->HasAuthority())
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
    // World 컨텍스트 검증
    if (!IsWorldContextValid())
    {
        return;
    }

    // 통일된 RTPC 시스템 사용
    // GetDistanceScalingForMode는 RTSDistanceScaling(2.0f) 또는 TPSDistanceScaling(1.0f) 값을 반환
    // SetUnifiedRTPCValue는 0-1 범위를 기대하므로, 0-2 범위를 0-1로 정규화
    const float ScalingValue = GetDistanceScalingForMode(bIsRTS);
    const float NormalizedScaling = ScalingValue / 2.0f; // 0-2 범위를 0-1로 정규화 (1.0f → 0.5f, 2.0f → 1.0f)
    SetUnifiedRTPCValue(AttenuationModeRTPC, NormalizedScaling);

    // RTS 모드에서는 오클루전/오브스트럭션 비활성화
    const float OcclusionValue = bIsRTS ? 1.0f : 0.0f; // 1.0f = 비활성화, 0.0f = 활성화
    SetUnifiedRTPCValue(OcclusionDisableRTPC, OcclusionValue); // 이미 0-1 범위

    // RTS 모드에서는 AkComponent의 내장 오클루전 기능을 직접 비활성화!
    UAkComponent* AkComp = GetOrCreateAkComponent();
    if (IsValid(AkComp))
    {
        // Owner 및 Transform 검증
        AActor* Owner = GetOwner();
        if (!Owner || !IsValid(Owner))
        {
            return;
        }

        const FVector Location = Owner->GetActorLocation();
        const FRotator Rotation = Owner->GetActorRotation();

        if (IsTransformValid(Location, Rotation))
        {
            // OcclusionRefreshInterval을 0으로 설정하면 오클루전 계산이 비활성화!
            // TPS 모드에서는 0.2초마다 계산하도록 재활성화!
            AkComp->OcclusionRefreshInterval = bIsRTS ? 0.0f : 0.2f;
        }
        else
        {
            // Invalid Transform - silently skip
        }
    }
}

UAkComponent* UGS_AudioComponentBase::GetOrCreateAkComponent()
{
    // 캐시된 컴포넌트가 유효하면 바로 반환
    if (CachedAkComponent && IsValid(CachedAkComponent))
    {
        return CachedAkComponent;
    }

    AActor* Owner = GetOwner();
    if (!IsValid(Owner))
    {
        return nullptr;
    }

    // World 검증 (Seamless Travel 대응: bIsTearingDown 체크 제거)
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel())
    {
        return nullptr;
    }

    // Transform 검증
    const FVector Location = Owner->GetActorLocation();
    const FRotator Rotation = Owner->GetActorRotation();

    if (!IsTransformValid(Location, Rotation))
    {
        UE_LOG(LogTemp, Error, TEXT("[GS_AudioComponentBase] Invalid Transform at component creation - Owner: %s"), *Owner->GetName());
        return nullptr;
    }

    // 먼저 기존 AkComponent 찾기
    CachedAkComponent = Owner->FindComponentByClass<UAkComponent>();
    if (!CachedAkComponent)
    {
        // 없으면 새로 생성
        CachedAkComponent = NewObject<UAkComponent>(Owner, NAME_None, RF_Transient);
        if (IsValid(CachedAkComponent))
        {
            // Root Component에 Attach
            if (USceneComponent* RootC = Owner->GetRootComponent())
            {
                CachedAkComponent->AttachToComponent(RootC, FAttachmentTransformRules::KeepRelativeTransform);
            }

            // Transform 재검증 후 등록
            const FVector NewLocation = Owner->GetActorLocation();
            const FRotator NewRotation = Owner->GetActorRotation();

            if (IsTransformValid(NewLocation, NewRotation))
            {
                // RegisterComponent()는 World가 완전히 준비된 후에만 호출
                if (!World->bIsTearingDown)
                {
                    CachedAkComponent->RegisterComponent();
                }
                else
                {
                    // World가 정리 중이면 등록할 수 없으므로 생성 포기하고 재시도에 맡김
                    CachedAkComponent = nullptr;
                    return nullptr;
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[GS_AudioComponentBase] Invalid Transform during component registration - Owner: %s"), *Owner->GetName());
                CachedAkComponent = nullptr;
                return nullptr;
            }
        }
    }

    return CachedAkComponent;
}

// ======================
// 통일된 RTPC 시스템 구현
// ======================
void UGS_AudioComponentBase::SetUnifiedRTPCValue(UAkRtpc* RTPC, float NormalizedValue, float InterpolationTime)
{
    if (!RTPC)
    {
        // 한 번만 경고하고 스킵 (스팸 방지)
        static TSet<FString> WarnedActors;
        FString ActorName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
        
        // 이미 경고한 액터면 스킵
        if (!WarnedActors.Contains(ActorName))
        {
            UE_LOG(LogTemp, Warning, TEXT("[AudioComponentBase] RTPC is null for actor: %s"), *ActorName);
            WarnedActors.Add(ActorName);
        }
        return;
    }

    FAkAudioDevice* AkDevice = FAkAudioDevice::Get();
    if (!AkDevice)
    {
        return;
    }

    // 0-1 정규화된 값을 0-100 Wwise 값으로 변환
    const float WwiseValue = NormalizedValue * 100.0f;
    const int32 InterpolationTimeMs = FMath::RoundToInt(InterpolationTime * 1000.0f);

    AkDevice->SetRTPCValue(RTPC, WwiseValue, InterpolationTimeMs, GetOwner());
}

// ==========
// 초기화 함수
// ==========

bool UGS_AudioComponentBase::InitializeAudioSystem()
{
    // World 유효성 체크 (Seamless Travel 대응)
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel())
    {
        return false;
    }

    // Seamless Travel 중인지 확인
    if (World->bIsTearingDown)
    {
        return false;
    }

    // Wwise 오디오 디바이스 확인
    FAkAudioDevice* AkDevice = FAkAudioDevice::Get();
    if (!AkDevice || !AkDevice->IsInitialized())
    {
        return false;
    }

    // 레벨 전환 후 AkComponent 재초기화 (중요!)
    // EndPlay에서 nullptr로 설정된 AkComponent를 다시 생성
    UAkComponent* AkComp = GetOrCreateAkComponent();
    if (!AkComp)
    {
        return false;
    }

    // 모든 오디오 RTPC 초기화
    InitializeAudioRTPCs();

    // 거리 체크 타이머 시작 - 성능 최적화된 주기
    if (!DistanceCheckTimerHandle.IsValid())
    {
        World->GetTimerManager().SetTimer(
            DistanceCheckTimerHandle,
            this,
            &UGS_AudioComponentBase::UpdateDistanceRTPC,
            DistanceCheckInterval,
            true
        );
    }

    return true;
}

void UGS_AudioComponentBase::RetryAudioInitialization()
{
    static const int32 MaxRetries = 5;

    // 멤버 변수 사용 (static TMap 제거!)
    AudioInitRetryCount++;

    if (InitializeAudioSystem())
    {
        AudioInitRetryCount = 0;  // 성공 시 리셋
        return;
    }

    // 최대 재시도 횟수 도달
    if (AudioInitRetryCount >= MaxRetries)
    {
        AudioInitRetryCount = 0;  // 실패 시 리셋
        return;
    }

    // 재시도 - 멤버 변수 사용 (지역 변수 금지!)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RetryInitTimerHandle,
            this,
            &UGS_AudioComponentBase::RetryAudioInitialization,
            0.2f,  // 200ms 후 재시도
            false
        );
    }
}

void UGS_AudioComponentBase::InitializeAudioRTPCs()
{
    if (!GetOwner())
    {
        return;
    }

    // Distance Scaling 초기값 설정 (TPS 모드 기본: 1.0f = 100%)
    SetUnifiedRTPCValue(AttenuationModeRTPC, TPSDistanceScaling);

    // 오클루전 기본값 설정 (TPS 모드에서는 활성화: 0.0f)
    SetUnifiedRTPCValue(OcclusionDisableRTPC, 0.0f);
}

void UGS_AudioComponentBase::SafeClearTimer(FTimerHandle& TimerHandle)
{
    if (TimerHandle.IsValid())
    {
        UWorld* World = GetWorld();
        if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
        {
            World->GetTimerManager().ClearTimer(TimerHandle);
        }
        TimerHandle.Invalidate();
    }
}

// ==========================
// Multicast RPC 최적화 헬퍼
// ==========================

bool UGS_AudioComponentBase::ShouldPlayMulticastSound(AActor* SourceActor, bool& OutIsRTSMode, FVector& OutListenerLocation, bool bSkipViewFrustumCheck) const
{
    // 1. 데디케이티드 서버에서는 오디오 처리 불필요
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return false;
    }

    // 2. Owner 및 World 유효성 체크 (서버 안정성 강화)
    if (!SourceActor || !IsValid(SourceActor) || !GetWorld() || !IsWorldContextValid())
    {
        return false;
    }

    // 3. 리스너 위치 가져오기
    if (!GetListenerLocation(OutListenerLocation))
    {
        return false;
    }

    // 리스너 위치 검증
    if (!IsLocationValid(OutListenerLocation))
    {
        return false;
    }

    // 4. RTS 모드 확인
    OutIsRTSMode = IsRTSMode();
    const float MaxDistance = GetMaxDistanceForMode(OutIsRTSMode);

    // 5. 소스 위치 가져오기 및 검증
    const FVector SourceLocation = SourceActor->GetActorLocation();
    if (!IsLocationValid(SourceLocation))
    {
        return false;
    }

    const float DistanceToListener = FVector::Dist(SourceLocation, OutListenerLocation);

    // 거리 값 검증 (NaN/Infinity 체크)
    if (!FMath::IsFinite(DistanceToListener))
    {
        return false;
    }

    // 6. 모드별 거리/시야각 체크
    if (OutIsRTSMode)
    {
        // RTS 모드: ViewFrustum 체크 (화면에 보이는지 확인)
        if (!bSkipViewFrustumCheck && !IsInViewFrustum(SourceLocation))
        {
            return false;
        }
    }
    else
    {
        // TPS 모드: 거리 기반 체크
        if (DistanceToListener > MaxDistance)
        {
            return false;
        }
    }

    return true;
}

bool UGS_AudioComponentBase::PrepareMulticastSound(AActor* SourceActor, bool bSkipViewFrustumCheck)
{
    // 컴포넌트 유효성 검증 (베이스 클래스에서 공통 체크)
    if (!IsValid(this))
    {
        return false;
    }

    // 월드 컨텍스트 유효성 검증
    UWorld* World = GetWorld();
    if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
    {
        return false;
    }

    bool bIsRTSMode = false;
    FVector ListenerLocation;

    // 통합 체크 수행
    if (!ShouldPlayMulticastSound(SourceActor, bIsRTSMode, ListenerLocation, bSkipViewFrustumCheck))
    {
        return false;
    }

    // Distance Scaling 자동 설정
    SetDistanceScaling(bIsRTSMode);

    return true;
}

UAkAudioEvent* UGS_AudioComponentBase::SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound, bool bUseRTSMode) const
{
    const bool bRTS = bUseRTSMode || IsRTSMode();

    if (bRTS)
    {
        // RTS 사운드가 있으면 사용, 없으면 TPS 사운드로 폴백
        return RTSSound ? RTSSound : TPSSound;
    }

    return TPSSound;
}

bool UGS_AudioComponentBase::ShouldSkipListenServerRPC() const
{
    // 리슨 서버 중복 재생 방지: 서버에서 이미 로컬 실행했으므로 RPC 수신 시 스킵
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // 리슨 서버에서 Authority를 가진 액터의 경우 RPC 스킵
    bool bIsAuthority = Owner->GetLocalRole() == ROLE_Authority;
    bool bIsListenServer = World->GetNetMode() == NM_ListenServer;
    
    return bIsAuthority && bIsListenServer;
}

// ==========================
// 오디오 시스템 검증 (통합)
// ==========================
bool UGS_AudioComponentBase::IsAudioSystemValid() const
{
	UWorld* World = GetWorld();

	// 데디케이티드 서버에서는 오디오 처리 불필요
	// 단, Listen Server는 로컬 플레이어가 있으므로 오디오 필요
	if (World && World->GetNetMode() == NM_DedicatedServer)
	{
		// Dedicated Server인 경우에만 차단
		// (Listen Server는 NM_ListenServer(2)이므로 통과)
		return false;
	}

	// Wwise 오디오 디바이스 초기화 상태 확인
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice)
	{
		return false;
	}

	if (!AudioDevice->IsInitialized())
	{
		return false;
	}

	return true;
}

// ===========
// 방 관련 함수
// ===========
bool UGS_AudioComponentBase::IsInSameRoom(const FVector& Pos1, const FVector& Pos2) const
{
    AGS_RoomBase* Room1 = FindRoomAtLocation(Pos1);
    AGS_RoomBase* Room2 = FindRoomAtLocation(Pos2);

    // 둘 다 같은 방에 있는 경우
    if (Room1 && Room2 && Room1 == Room2)
    {
        return true;
    }

    // 두 방이 연결되어 있는지 확인
    if (Room1 && Room2 && AreRoomsConnected(Room1, Room2))
    {
        return true;
    }

    // 어느 한쪽이라도 방이 아닌 야외 공간에 있다면 소리가 들리도록 함
    if (!Room1 || !Room2)
    {
        return true;
    }

    return false;
}

bool UGS_AudioComponentBase::AreRoomsConnected(AGS_RoomBase* Room1, AGS_RoomBase* Room2) const
{
    if (!Room1 || !Room2) return false;

    // 경계 상자 기반으로 인접한 방인지 체크
    const FBox Room1Bounds = Room1->GetComponentsBoundingBox(true);
    const FBox Room2Bounds = Room2->GetComponentsBoundingBox(true);
    
    // 두 방의 경계 상자가 겹치는지 확인 (인접한 방)
    return Room1Bounds.Intersect(Room2Bounds);
}

AGS_RoomBase* UGS_AudioComponentBase::FindRoomAtLocation(const FVector& Location) const
{
    if (!GetWorld())
    {
        return nullptr;
    }

    TArray<FOverlapResult> Overlaps;
    // Room 액터와 충돌할 수 있도록 설정된 오브젝트 타입(WorldStatic)을 사용
    FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_WorldStatic);

    // Location 지점에서 1.0f 반경의 구체로 오버랩되는 액터를 찾음.
    if (GetWorld()->OverlapMultiByObjectType(Overlaps, Location, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(1.0f)))
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (AGS_RoomBase* Room = Cast<AGS_RoomBase>(Overlap.GetActor()))
            {
                // 오버랩된 액터가 실제로 해당 위치를 포함하는지 경계 상자로 한 번 더 확인하여 정확도를 높임.
                if (Room->GetComponentsBoundingBox(true).IsInside(Location))
                {
                    return Room;
                }
            }
        }
    }

    return nullptr;
}
