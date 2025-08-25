#include "Sound/GS_AudioComponentBase.h"
#include "AkAudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "AI/RTS/GS_RTSController.h"
#include "AI/RTS/GS_RTSCamera.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"


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
    
    // 모든 오디오 RTPC 초기화 (통일된 시스템 사용)
    InitializeAudioRTPCs();
    
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
            // RTS 모드에서는 실제 카메라 위치를 리스너 위치로 사용
            FVector ActualCameraLocation;
            if (GetActualCameraLocation(ActualCameraLocation))
            {
                OutLocation = ActualCameraLocation;
                return true;
            }
            
            // 폴백: ViewTarget 위치
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
        return CheckRTSAudioVisibility(RTSController, SourceLocation);
    }
    
    // TPS 모드는 항상 true
    return true;
}

bool UGS_AudioComponentBase::CheckRTSAudioVisibility(AGS_RTSController* RTSController, const FVector& SourceLocation) const
{
    // TPS 모드에서는 프러스텀 계산 스킵 (성능 최적화)
    if (!IsRTSMode())
    {
        return true;
    }
    
    // 입력 유효성 검사
    if (!RTSController || !IsValid(RTSController))
    {
        return true;
    }
    
    // 1. 카메라 정보 가져오기
    FVector CameraLocation;
    if (!GetActualCameraLocation(CameraLocation))
    {
        if (AActor* ViewTarget = RTSController->GetViewTarget())
        {
            CameraLocation = ViewTarget->GetActorLocation();
        }
        else
        {
            // 카메라 위치를 가져올 수 없는 경우 안전한 폴백
            // RTS 모드에서는 기본적으로 소리 재생 허용 (플레이어 경험 보장)
            return true;
        }
    }
    
    // 카메라 위치 유효성 검사
    if (CameraLocation.IsZero())
    {
        return true; // 유효하지 않은 위치인 경우 소리 재생 허용
    }
    
    // 2. RTS 카메라의 실제 뷰포트 경계 계산 (캐싱 활용)
    FBox2D ScreenBounds(ForceInit);
    
    // 캐싱된 RTS 카메라 사용 (성능 최적화)
    AGS_RTSCamera* RTSCameraActor = CachedRTSCamera.Get();
    
    if (!RTSCameraActor || !IsValid(RTSCameraActor))
    {
        // 캐시가 유효하지 않으면 새로 찾기
        for (TActorIterator<AGS_RTSCamera> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
        {
            AGS_RTSCamera* FoundCamera = *ActorIterator;
            if (FoundCamera && IsValid(FoundCamera))
            {
                RTSCameraActor = FoundCamera;
                CachedRTSCamera = FoundCamera; // 캐시 업데이트
                break;
            }
        }
    }
    
    if (RTSCameraActor)
    {
        // RTS 카메라의 컴포넌트들이 유효한지 확인
        UCameraComponent* CameraComp = RTSCameraActor->GetCameraComponent();
        USpringArmComponent* SpringArmComp = RTSCameraActor->GetSpringArmComponent();
        
        if (CameraComp && SpringArmComp)
        {
            // 간단한 RTS 카메라 뷰포트 계산 사용
            ScreenBounds = RTSCameraActor->GetSimpleViewBounds();
        }
        else
        {
            // 컴포넌트가 없으면 폴백: 기존 방식 사용
            ScreenBounds = CalculateScreenWorldBounds(RTSController);
        }
    }
    else
    {
        // 폴백: 기존 방식 사용
        ScreenBounds = CalculateScreenWorldBounds(RTSController);
    }
    
    // 화면 영역 유효성 검사
    if (ScreenBounds.GetExtent().IsZero())
    {
        return true; // 유효하지 않은 화면 영역인 경우 소리 재생 허용
    }
    
    // 3. RTS 카메라의 뷰포트 내 위치 확인
    FVector2D SourcePos2D(SourceLocation.X, SourceLocation.Y);
    bool bInScreen = ScreenBounds.IsInside(SourcePos2D);
    
    if (bInScreen)
    {
        return true; // 화면 안에 있으면 무조건 재생
    }
    
    // 4. 화면 밖이지만 가까운 경우 체크
    // RTS 카메라의 정확한 거리 계산 사용
    float DistanceToScreen = CalculateDistanceToScreenBounds(SourcePos2D, ScreenBounds);
    
    // 거리 유효성 검사
    if (DistanceToScreen < 0.0f || FMath::IsNaN(DistanceToScreen))
    {
        return true; // 유효하지 않은 거리인 경우 소리 재생 허용
    }
    
    // 5. 통로/복도 감지 (십자 형태 맵 특별 처리)
    bool bInCorridor = IsInCorridorRange(CameraLocation, SourceLocation);
    
    if (bInCorridor)
    {
        // 복도/통로에 있는 경우 더 멀리까지 들림
        return DistanceToScreen <= CorridorAudioDistance;
    }
    
    // 6. 일반적인 경우
    return DistanceToScreen <= AudioExtendDistance;
}

// RTS 카메라의 실제 보이는 영역 계산
FBox2D UGS_AudioComponentBase::GetRTSCameraViewBounds() const
{
    FBox2D ViewBounds(ForceInit);
    
    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!LocalPC || !LocalPC->PlayerCameraManager)
    {
        return ViewBounds;
    }
    
    // 화면의 네 모서리를 월드 좌표로 변환
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    else
    {
        ViewportSize = FVector2D(1920, 1080); // 기본값
    }
    
    // 화면 모서리 좌표
    TArray<FVector2D> ScreenCorners = {
        FVector2D(0, 0),                                    // 좌상단
        FVector2D(ViewportSize.X, 0),                       // 우상단
        FVector2D(0, ViewportSize.Y),                       // 좌하단
        FVector2D(ViewportSize.X, ViewportSize.Y)           // 우하단
    };
    
    // 각 모서리를 월드 좌표로 변환
    for (const FVector2D& ScreenPos : ScreenCorners)
    {
        FVector WorldPos, WorldDir;
        if (LocalPC->DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldPos, WorldDir))
        {
            // 지면과의 교점 계산 (Z = 0 평면)
            float T = -WorldPos.Z / WorldDir.Z;
            FVector GroundPos = WorldPos + WorldDir * T;
            
            ViewBounds += FVector2D(GroundPos.X, GroundPos.Y);
        }
    }
    
    // 여유 공간 추가 (화면 크기의 50%)
    FVector2D Center = ViewBounds.GetCenter();
    FVector2D Extent = ViewBounds.GetExtent() * 1.5f; // 150% 크기
    
    return FBox2D(Center - Extent, Center + Extent);
}

bool UGS_AudioComponentBase::GetActualCameraLocation(FVector& OutLocation) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // 캐싱된 카메라 위치가 유효하고 업데이트 주기 내라면 캐시 사용
    if (!CachedCameraLocation.IsZero() && 
        (CurrentTime - LastCameraLocationUpdateTime) < CameraLocationUpdateInterval)
    {
        OutLocation = CachedCameraLocation;
        return true;
    }
    
    // Get local player controller
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
    
    // RTS Controller 처리
    if (AGS_RTSController* RTSController = Cast<AGS_RTSController>(LocalPC))
    {
        // 1-1: 캐싱된 RTSCamera 사용 (성능 최적화)
        AGS_RTSCamera* RTSCameraActor = nullptr;
        
        if (CachedRTSCamera.IsValid())
        {
            RTSCameraActor = CachedRTSCamera.Get();
        }
        else
        {
            // 월드에서 RTSCamera 찾기 (한 번만 실행)
            for (TActorIterator<AGS_RTSCamera> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
            {
                AGS_RTSCamera* FoundCamera = *ActorIterator;
                if (FoundCamera && IsValid(FoundCamera))
                {
                    RTSCameraActor = FoundCamera;
                    CachedRTSCamera = FoundCamera; // 캐시에 저장
                    break;
                }
            }
        }
        
        if (RTSCameraActor)
        {
            FVector NewCameraLocation = FVector::ZeroVector;
            
            UCameraComponent* CameraComp = RTSCameraActor->GetCameraComponent();
            USpringArmComponent* SpringArmComp = RTSCameraActor->GetSpringArmComponent();
            
            if (CameraComp)
            {
                // 카메라 컴포넌트의 실제 위치 사용
                NewCameraLocation = CameraComp->GetComponentLocation();
            }
            else if (SpringArmComp)
            {
                // SpringArm을 통한 카메라 위치 계산
                NewCameraLocation = SpringArmComp->GetComponentLocation() + SpringArmComp->GetForwardVector() * SpringArmComp->TargetArmLength;
            }
            else
            {
                // 최후에는 RTS 카메라 액터의 위치
                NewCameraLocation = RTSCameraActor->GetActorLocation();
            }
            
            if (!NewCameraLocation.IsZero())
            {
                // 캐시 업데이트 (mutable 변수들이므로 const 함수에서도 수정 가능)
                CachedCameraLocation = NewCameraLocation;
                LastCameraLocationUpdateTime = CurrentTime;
                
                OutLocation = NewCameraLocation;
                return true;
            }
        }
        
        // 1-2: PlayerCameraManager 백업 (캐싱 없음 - 불안정함)
        if (RTSController->PlayerCameraManager)
        {
            FVector CameraManagerLocation = RTSController->PlayerCameraManager->GetCameraLocation();
            
            if (!CameraManagerLocation.IsZero())
            {
                CachedCameraLocation = CameraManagerLocation;
                LastCameraLocationUpdateTime = CurrentTime;
                
                OutLocation = CameraManagerLocation;
                return true;
            }
        }
    }
    
    // 백업: TPS 모드이거나 다른 경우
    if (LocalPC->PlayerCameraManager)
    {
        FVector CameraManagerLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
        if (!CameraManagerLocation.IsZero())
        {
            OutLocation = CameraManagerLocation;
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
    if (!GetOwner() || !GetWorld()) return;

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const bool bCurrentRTSMode = IsRTSMode();
    
    FVector ListenerLocation;
    if (GetListenerLocation(ListenerLocation))
    {
        float DistanceToListener = FVector::Dist(GetOwner()->GetActorLocation(), ListenerLocation);

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
        
        // Distance Scaling을 매 업데이트마다 현재 모드에 맞게 설정
        SetDistanceScaling(bCurrentRTSMode);
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
    // 통일된 RTPC 시스템 사용
    const float ScalingValue = GetDistanceScalingForMode(bIsRTS);
    SetUnifiedRTPCValue(AttenuationModeRTPC, ScalingValue / 100.0f); // 0-100 → 0-1 정규화
    
    // RTS 모드에서는 오클루전/오브스트럭션 비활성화
    const float OcclusionValue = bIsRTS ? 1.0f : 0.0f; // 1.0f = 비활성화, 0.0f = 활성화
    SetUnifiedRTPCValue(OcclusionDisableRTPC, OcclusionValue); // 이미 0-1 범위
}

FBox2D UGS_AudioComponentBase::CalculateScreenWorldBounds(AGS_RTSController* RTSController) const
{
    // 단순화된 계산 방식 우선 시도
    FBox2D SimplifiedBounds = CalculateSimplifiedScreenBounds(RTSController);
    if (!SimplifiedBounds.GetExtent().IsZero())
    {
        return SimplifiedBounds;
    }
    
    // 폴백: 기존 복잡한 방식
    FBox2D Bounds(ForceInit);
    
    if (!RTSController->PlayerCameraManager)
    {
        // 기본값 반환
        return FBox2D(FVector2D(-2000, -2000), FVector2D(2000, 2000));
    }
    
    // 1. 카메라 정보 가져오기 (FOV, 스프링암 각도 등)
    float CameraFOV = DefaultFOV; // 기본값
    float SpringArmPitch = 0.0f;
    float SpringArmYaw = 0.0f;
    float SpringArmLength = 2000.0f;
    
    // FOV 가져오기
    if (RTSController->PlayerCameraManager)
    {
        CameraFOV = RTSController->PlayerCameraManager->GetFOVAngle();
    }
    
    if (AActor* ViewTarget = RTSController->GetViewTarget())
    {
        // 카메라 컴포넌트에서 FOV 확인
        if (UCameraComponent* CameraComp = ViewTarget->FindComponentByClass<UCameraComponent>())
        {
            CameraFOV = CameraComp->FieldOfView;
        }
        
        if (USpringArmComponent* SpringArm = ViewTarget->FindComponentByClass<USpringArmComponent>())
        {
            SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
            SpringArmYaw = SpringArm->GetRelativeRotation().Yaw;
            SpringArmLength = SpringArm->TargetArmLength;
        }
    }
    
    // 2. 뷰포트 크기
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    else
    {
        ViewportSize = FVector2D(1920, 1080);
    }
    
    // 3. FOV 기반 계산 (Perspective 투영 고려)
    float FOVRadians = FMath::DegreesToRadians(CameraFOV);
    float AspectRatio = ViewportSize.X / ViewportSize.Y;
    
    // FOV와 카메라 높이를 이용한 이론적 화면 영역 계산
    FVector CameraLocation;
    if (GetActualCameraLocation(CameraLocation))
    {
        float CameraHeight = FMath::Abs(CameraLocation.Z);
        
        // Perspective 투영에서 지면에 투영되는 영역 계산
        float HalfVerticalSize = CameraHeight * FMath::Tan(FOVRadians * 0.5f);
        float HalfHorizontalSize = HalfVerticalSize * AspectRatio;
        
        // FOV 기반 이론적 경계 (스프링암 각도 무시)
        FBox2D TheoreticalBounds(
            FVector2D(CameraLocation.X - HalfHorizontalSize, CameraLocation.Y - HalfVerticalSize),
            FVector2D(CameraLocation.X + HalfHorizontalSize, CameraLocation.Y + HalfVerticalSize)
        );
        
        //  Pitch 각도 보정
        if (FMath::Abs(SpringArmPitch) > 5.0f) // 카메라가 기울어져 있으면 보정
        {
            // 선형 보정: -90도에서 1.3배, 0도에서 1.0배
            float PitchRad = FMath::DegreesToRadians(FMath::Abs(SpringArmPitch));
            float PitchMultiplier = FMath::Lerp(1.0f, 1.3f, PitchRad / (PI * 0.5f));
            
            HalfVerticalSize *= PitchMultiplier;
            HalfHorizontalSize *= PitchMultiplier;
            
            TheoreticalBounds = FBox2D(
                FVector2D(CameraLocation.X - HalfHorizontalSize, CameraLocation.Y - HalfVerticalSize),
                FVector2D(CameraLocation.X + HalfHorizontalSize, CameraLocation.Y + HalfVerticalSize)
            );
        }
        
        Bounds = TheoreticalBounds;
        
        return Bounds; // FOV 기반 정확한 계산 결과 반환
    }
    
    // 4. 스프링암 각도 보정 계산
    float PitchCorrectionFactor = 1.0f;
    float YawCorrectionFactor = 1.0f;
    
    // Pitch 보정
    if (FMath::Abs(SpringArmPitch) > 5.0f)
    {
        float PitchRad = FMath::DegreesToRadians(FMath::Abs(SpringArmPitch));
        PitchCorrectionFactor = FMath::Lerp(1.0f, 0.85f, PitchRad / (PI * 0.5f));
    }
    
    // Yaw 보정
    if (FMath::Abs(SpringArmYaw) > 45.0f)
    {
        float YawRad = FMath::DegreesToRadians(FMath::Abs(SpringArmYaw));
        YawCorrectionFactor = FMath::Lerp(1.0f, 0.9f, YawRad / PI);
    }
    
    // 5. 화면 가장자리 샘플링 (성능과 정확도 균형) - 폴백 방식
    const int32 SampleCount = 8; // 각 변마다 8개 포인트 (성능 최적화)
    TArray<FVector2D> ScreenPoints;
    
    // 상단 변
    for (int32 i = 0; i <= SampleCount; i++)
    {
        float X = (ViewportSize.X / SampleCount) * i;
        ScreenPoints.Add(FVector2D(X, 0));
    }
    
    // 우측 변
    for (int32 i = 1; i <= SampleCount; i++)
    {
        float Y = (ViewportSize.Y / SampleCount) * i;
        ScreenPoints.Add(FVector2D(ViewportSize.X, Y));
    }
    
    // 하단 변
    for (int32 i = SampleCount - 1; i >= 0; i--)
    {
        float X = (ViewportSize.X / SampleCount) * i;
        ScreenPoints.Add(FVector2D(X, ViewportSize.Y));
    }
    
    // 좌측 변
    for (int32 i = SampleCount - 1; i > 0; i--)
    {
        float Y = (ViewportSize.Y / SampleCount) * i;
        ScreenPoints.Add(FVector2D(0, Y));
    }
    
    // 5. 월드 좌표로 변환 (스프링암 각도 보정 적용)
    for (const FVector2D& ScreenPos : ScreenPoints)
    {
        FVector WorldPos, WorldDir;
        if (RTSController->DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldPos, WorldDir))
        {
            // 지면과의 교점 계산 (Z = 0 평면)
            if (FMath::Abs(WorldDir.Z) > 0.001f)
            {
                float T = -WorldPos.Z / WorldDir.Z;
                FVector GroundPos = WorldPos + WorldDir * T;
                
                // 단순한 스프링암 보정 적용
                FVector2D OriginalPos(GroundPos.X, GroundPos.Y);
                float TotalCorrectionFactor = PitchCorrectionFactor * YawCorrectionFactor;
                FVector2D CorrectedPos = OriginalPos * TotalCorrectionFactor;
                
                Bounds += CorrectedPos;
            }
        }
    }
    
    // 6. 최종 경계 보정
    FVector2D Center = Bounds.GetCenter();
    FVector2D CurrentExtent = Bounds.GetExtent();
    
    float FinalMultiplier = FMath::Clamp(PitchCorrectionFactor * YawCorrectionFactor, 0.7f, 1.5f);
    FVector2D NewExtent = CurrentExtent * FinalMultiplier;
    
    Bounds = FBox2D(Center - NewExtent, Center + NewExtent);
    
    return Bounds;
}


float UGS_AudioComponentBase::CalculateDistanceToScreenBounds(const FVector2D& Point, const FBox2D& Bounds) const
{
    // 점과 사각형 경계까지의 최단 거리
    if (Bounds.IsInside(Point))
    {
        return 0.0f;
    }
    
    FVector2D ClosestPoint;
    ClosestPoint.X = FMath::Clamp(Point.X, Bounds.Min.X, Bounds.Max.X);
    ClosestPoint.Y = FMath::Clamp(Point.Y, Bounds.Min.Y, Bounds.Max.Y);
    
    return FVector2D::Distance(Point, ClosestPoint);
}

bool UGS_AudioComponentBase::IsInCorridorRange(const FVector& CameraLocation, const FVector& SourceLocation) const
{
    // 십자 형태 맵의 통로 감지 (카메라와 소스가 직선 통로로 연결되어 있는지 체크)
    FVector2D CameraPos2D(CameraLocation.X, CameraLocation.Y);
    FVector2D SourcePos2D(SourceLocation.X, SourceLocation.Y);
    
    // X축 또는 Y축 정렬 체크 (통로는 보통 직선)
    bool bXAligned = FMath::Abs(CameraPos2D.X - SourcePos2D.X) < CorridorAlignmentThreshold;
    bool bYAligned = FMath::Abs(CameraPos2D.Y - SourcePos2D.Y) < CorridorAlignmentThreshold;
    
    // 십자 형태에서는 X 또는 Y 중 하나가 정렬되어 있으면 통로
    return bXAligned || bYAligned;
}


// ======================
// 화면 경계 계산 함수 구현
// ======================

FBox2D UGS_AudioComponentBase::CalculateSimplifiedScreenBounds(AGS_RTSController* RTSController) const
{
    if (!RTSController || !RTSController->PlayerCameraManager)
    {
        return FBox2D(ForceInit);
    }
    
    FVector CameraLocation;
    if (!GetActualCameraLocation(CameraLocation))
    {
        return FBox2D(ForceInit);
    }
    
    // 카메라 설정 가져오기
    float CameraFOV = RTSController->PlayerCameraManager->GetFOVAngle();
    float CameraHeight = FMath::Abs(CameraLocation.Z);
    
    // 기본 화면 영역 계산
    return CalculateBasicViewBounds(CameraLocation, CameraFOV, CameraHeight);
}

FBox2D UGS_AudioComponentBase::CalculateBasicViewBounds(const FVector& CameraLocation, float FOV, float CameraHeight, float AspectRatio) const
{
    // FOV를 라디안으로 변환
    const float FOVRadians = FMath::DegreesToRadians(FOV);
    
    // 기본 투영 계산
    const float HalfVerticalSize = CameraHeight * FMath::Tan(FOVRadians * 0.5f);
    const float HalfHorizontalSize = HalfVerticalSize * AspectRatio;
    
    // 경계 상자 생성 (여유 공간 추가)
    const FVector2D HalfExtent(HalfHorizontalSize * ViewBoundsMargin, HalfVerticalSize * ViewBoundsMargin);
    const FVector2D Center(CameraLocation.X, CameraLocation.Y);
    
    return FBox2D(Center - HalfExtent, Center + HalfExtent);
}

// 통일된 RTPC 시스템 구현
void UGS_AudioComponentBase::SetUnifiedRTPCValue(UAkRtpc* RTPC, float NormalizedValue, float InterpolationTime)
{
    if (!RTPC)
    {
        // 한 번만 경고하고 스킵 (스팸 방지)
        static TSet<FString> WarnedActors;
        FString ActorName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
        
        if (!WarnedActors.Contains(ActorName))
        {
            UE_LOG(LogTemp, Warning, TEXT("[AudioBase] RTPC not assigned in %s - check Blueprint configuration"), *ActorName);
            WarnedActors.Add(ActorName);
        }
        return;
    }

    FAkAudioDevice* AkDevice = FAkAudioDevice::Get();
    if (!AkDevice)
    {
        UE_LOG(LogTemp, Error, TEXT("[AudioBase] SetUnifiedRTPCValue: Wwise AudioDevice not found"));
        return;
    }

    // 0-1 정규화된 값을 0-100 Wwise 값으로 변환
    const float WwiseValue = NormalizedValue * 100.0f;
    const int32 InterpolationTimeMs = FMath::RoundToInt(InterpolationTime * 1000.0f);

    AKRESULT Result = AkDevice->SetRTPCValue(RTPC, WwiseValue, InterpolationTimeMs, GetOwner());
    
    if (Result != AK_Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AudioBase] SetUnifiedRTPCValue failed for RTPC: %s, Value: %.2f"), 
               RTPC ? *RTPC->GetName() : TEXT("Unknown"), WwiseValue);
    }
}

void UGS_AudioComponentBase::InitializeAudioRTPCs()
{
    if (!GetOwner())
    {
        return;
    }

    // Distance Scaling 초기값 설정 (TPS 모드 기본: 1.0f = 100%)
    if (AttenuationModeRTPC)
    {
        SetUnifiedRTPCValue(AttenuationModeRTPC, TPSDistanceScaling);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AudioBase] AttenuationModeRTPC not assigned in %s - audio distance scaling disabled"), *GetOwner()->GetName());
    }

    // Occlusion 초기값 설정 (TPS 모드는 활성화: 0.0f = 0%)
    if (OcclusionDisableRTPC)
    {
        SetUnifiedRTPCValue(OcclusionDisableRTPC, 0.0f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AudioBase] OcclusionDisableRTPC not assigned in %s - audio occlusion disabled"), *GetOwner()->GetName());
    }
}