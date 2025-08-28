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
    
    // 모든 오디오 RTPC 초기화
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
        // 새로운 화면 투영 기반 체크 사용
        return IsSourceVisibleOnScreen(RTSController, SourceLocation);
    }
    
    // TPS 모드는 항상 true
    return true;
}


bool UGS_AudioComponentBase::IsSourceVisibleOnScreen(AGS_RTSController* RTSController, const FVector& SourceLocation) const
{
    if (!RTSController)
    {
        return true;
    }
    
    // 1. 카메라 정보 가져오기
    FVector CameraLocation;
    FRotator CameraRotation;
    if (!GetActualCameraLocation(CameraLocation)) return true;
    
    // RTSCamera에서 실제 회전값 가져오기
    if (AGS_RTSCamera* RTSCamera = CachedRTSCamera.Get())
    {
        if (USpringArmComponent* SpringArm = RTSCamera->GetSpringArmComponent())
        {
            CameraRotation = SpringArm->GetComponentRotation();
        }
        else if (UCameraComponent* CameraComp = RTSCamera->GetCameraComponent())
        {
            CameraRotation = CameraComp->GetComponentRotation();
        }
    }
    else
    {
        if (RTSController->PlayerCameraManager)
        {
            CameraRotation = RTSController->PlayerCameraManager->GetCameraRotation();
        }
    }
    
    // 2. 카메라 각도를 고려한 시야각 및 거리 체크
    FVector CameraToSource = SourceLocation - CameraLocation;
    float Distance = CameraToSource.Size();
    
    // 지면 기준 카메라 forward 벡터 계산 (pitch 영향 제거)
    FVector CameraForward = CameraRotation.Vector();
    FVector GroundForward = FVector(CameraForward.X, CameraForward.Y, 0.0f).GetSafeNormal();
    
    // 시야각 체크 - 지면 투영된 벡터로 계산
    CameraToSource.Normalize();
    FVector GroundCameraToSource = FVector(CameraToSource.X, CameraToSource.Y, 0.0f).GetSafeNormal();
    float DotProduct = FVector::DotProduct(GroundForward, GroundCameraToSource);
    
    // Pitch 각도에 따른 시야각 조정 (-60도가 -90도보다 더 넓은 시야각)
    float AdjustedFOV = 90.0f; // 기본 FOV
    float CameraPitch = FMath::Abs(CameraRotation.Pitch);
    if (CameraPitch > 45.0f)
    {
        // -60도(60도)가 -90도(90도)보다 더 넓은 시야각을 가지도록 수정
        float PitchFactor = (90.0f - CameraPitch) / 45.0f; // 90도에서 0, 45도에서 1
        AdjustedFOV = FMath::Lerp(120.0f, 140.0f, FMath::Clamp(PitchFactor, 0.0f, 1.0f));
    }
    
    float FOVCos = FMath::Cos(FMath::DegreesToRadians(AdjustedFOV * 0.5f));
    
    // 3. 수학적 시야각 체크
    bool bPassedMathCheck = (DotProduct >= FOVCos);
    if (!bPassedMathCheck)
    {
        // 시야각 밖 - 각도별 최적화된 거리 임계값 적용
        float CloseDistance = 1500.0f; // 기본값
        
        if (CameraPitch >= 85.0f) // -90도 근처
        {
            CloseDistance = 2000.0f;
        }
        else if (CameraPitch >= 55.0f && CameraPitch < 85.0f) // -60도 범위
        {
            CloseDistance = 2500.0f; // -60도는 가장 관대하게
        }
        else if (CameraPitch > 45.0f) // -45도 ~ -55도
        {
            CloseDistance = 2200.0f;
        }
        
        if (Distance > CloseDistance)
        {
            // 시야각 밖이고 멀리 있으면 화면 투영으로 재검증
            FVector2D ScreenPosition;
            bool bIsOnScreen = RTSController->ProjectWorldLocationToScreen(SourceLocation, ScreenPosition, false);
            
            if (!bIsOnScreen) return false; // 화면에도 없으면 차단
            
            // 화면 경계 확인
            FVector2D ViewportSize;
            if (GEngine && GEngine->GameViewport)
            {
                GEngine->GameViewport->GetViewportSize(ViewportSize);
                
                // -60도에서 더 관대한 화면 경계 적용
                float MarginMultiplier = (CameraPitch >= 55.0f && CameraPitch < 85.0f) ? 0.2f : 0.1f;
                float MarginX = ViewportSize.X * MarginMultiplier;
                float MarginY = ViewportSize.Y * MarginMultiplier;
                
                bool bInScreenBounds = (ScreenPosition.X >= -MarginX && 
                                      ScreenPosition.X <= ViewportSize.X + MarginX &&
                                      ScreenPosition.Y >= -MarginY && 
                                      ScreenPosition.Y <= ViewportSize.Y + MarginY);
                
                if (!bInScreenBounds) return false; // 화면 경계 밖이면 차단
            }
        }
    }
    
    // 4. 거리 기반 예외 처리
    if (Distance <= 800.0f) // 8미터 이내는 항상 들림
    {
        return true;
    }
    
    // 5. 방 모듈 체크
    if (IsInSameRoom(CameraLocation, SourceLocation))
    {
        if (Distance <= 1200.0f) // 같은 방이면 12미터까지 들림
        {
            return true;
        }
    }
    
    // 6. 최종 통과
    return true;
}

bool UGS_AudioComponentBase::GetActualCameraLocation(FVector& OutLocation) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
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
        // 1-1: 캐싱된 RTSCamera 사용
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
            FVector NewCameraLocation;
            
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
                // RTS 카메라 액터의 위치
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
        
        // 1-2: PlayerCameraManager 백업
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
    
    // 1. 카메라 정보 가져오기 (FOV, 스프링암 각도 및 길이 등)
    float CameraFOV = DefaultFOV; // 기본값
    float SpringArmPitch = 0.0f;
    float SpringArmYaw = 0.0f;
    float SpringArmLength = 2000.0f; // 기본값
    float CameraHeight = 0.0f;
    
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
    
    // 카메라 높이 계산 (SpringArm 길이와 각도를 고려)
    FVector CameraLocation;
    if (GetActualCameraLocation(CameraLocation))
    {
        CameraHeight = FMath::Abs(CameraLocation.Z);
    }
    else
    {
        // SpringArm 정보로 카메라 높이 추정
        CameraHeight = SpringArmLength * FMath::Sin(FMath::DegreesToRadians(FMath::Abs(SpringArmPitch)));
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
    
    // 3. SpringArm 정보를 활용한 정확한 FOV 계산
    float FOVRadians = FMath::DegreesToRadians(CameraFOV);
    float AspectRatio = ViewportSize.X / ViewportSize.Y;
    
    if (CameraHeight > 0.0f)
    {
        float HalfVerticalSize = CameraHeight * FMath::Tan(FOVRadians * 0.5f);
        float HalfHorizontalSize = HalfVerticalSize * AspectRatio;
        
        // SpringArm Pitch 각도 보정 (더 정확한 계산)
        if (FMath::Abs(SpringArmPitch) > 5.0f)
        {
            float PitchRad = FMath::DegreesToRadians(FMath::Abs(SpringArmPitch));
            
            // 실제 지면 투영 거리 계산
            float HorizontalDistance = SpringArmLength * FMath::Cos(PitchRad);
            float VerticalOffset = HorizontalDistance * FMath::Tan(FOVRadians * 0.5f);
            
            // 각도가 클수록 앞쪽으로 더 많이 보임
            float ForwardBias = FMath::Lerp(1.0f, 1.8f, PitchRad / (PI * 0.5f));
            float BackwardBias = FMath::Lerp(1.0f, 0.6f, PitchRad / (PI * 0.5f));
            
            HalfVerticalSize = VerticalOffset;
            HalfHorizontalSize = HalfVerticalSize * AspectRatio;
            
            // 카메라 위치 기준으로 비대칭 경계 계산
            FVector CameraPos;
            if (GetActualCameraLocation(CameraPos))
            {
                FVector ForwardDir = FVector(1, 0, 0).RotateAngleAxis(SpringArmYaw, FVector::UpVector);
                FVector2D Center2D(CameraPos.X, CameraPos.Y);
                FVector2D ForwardOffset2D(ForwardDir.X, ForwardDir.Y);
                
                // 앞쪽과 뒤쪽을 다르게 적용
                FVector2D ForwardExtent = ForwardOffset2D * (HalfVerticalSize * ForwardBias);
                FVector2D BackwardExtent = ForwardOffset2D * (HalfVerticalSize * BackwardBias);
                FVector2D SideExtent = FVector2D(-ForwardDir.Y, ForwardDir.X) * HalfHorizontalSize;
                
                Bounds = FBox2D(ForceInit);
                Bounds += Center2D + ForwardExtent + SideExtent;
                Bounds += Center2D + ForwardExtent - SideExtent;
                Bounds += Center2D - BackwardExtent + SideExtent;
                Bounds += Center2D - BackwardExtent - SideExtent;
                
                return Bounds;
            }
        }
        
        // 기본 대칭 계산
        FVector CameraPos;
        if (GetActualCameraLocation(CameraPos))
        {
            FBox2D TheoreticalBounds(
                FVector2D(CameraPos.X - HalfHorizontalSize, CameraPos.Y - HalfVerticalSize),
                FVector2D(CameraPos.X + HalfHorizontalSize, CameraPos.Y + HalfVerticalSize)
            );
            
            return TheoreticalBounds;
        }
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
        
        WarnedActors.Add(ActorName);
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

    // Occlusion 초기값 설정 (TPS 모드는 활성화: 0.0f = 0%)
    if (OcclusionDisableRTPC)
    {
        SetUnifiedRTPCValue(OcclusionDisableRTPC, 0.0f);
    }
}

// 새로운 헬퍼 함수 추가
bool UGS_AudioComponentBase::IsInSameRoom(const FVector& ListenerPos, const FVector& SourcePos) const
{
    // 방 모듈 찾기
    TArray<AActor*> OverlappingRooms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGS_RoomBase::StaticClass(), OverlappingRooms);
    
    AGS_RoomBase* ListenerRoom = nullptr;
    AGS_RoomBase* SourceRoom = nullptr;
    
    for (AActor* RoomActor : OverlappingRooms)
    {
        if (AGS_RoomBase* Room = Cast<AGS_RoomBase>(RoomActor))
        {
            FBox RoomBounds = Room->GetComponentsBoundingBox();
            
            if (RoomBounds.IsInside(ListenerPos))
                ListenerRoom = Room;
            
            if (RoomBounds.IsInside(SourcePos))
                SourceRoom = Room;
        }
    }
    
    // 같은 방이거나 연결된 방인지 체크
    return ListenerRoom == SourceRoom || AreRoomsConnected(ListenerRoom, SourceRoom);
}

bool UGS_AudioComponentBase::AreRoomsConnected(AGS_RoomBase* Room1, AGS_RoomBase* Room2) const
{
    if (!Room1 || !Room2) return false;
    
    // 인접한 방인지 체크 (거리 기반)
    FVector Room1Center = Room1->GetActorLocation();
    FVector Room2Center = Room2->GetActorLocation();
    
    float RoomDistance = FVector::Dist(Room1Center, Room2Center);
    return RoomDistance < 2000.0f; // 방 크기에 따라 조절
}

FBox2D UGS_AudioComponentBase::CalculateAngledCameraViewBounds(const FVector& CameraLocation, float Pitch) const
{
    // -60도 카메라의 실제 투영 영역 계산
    const float PitchRad = FMath::DegreesToRadians(FMath::Abs(Pitch));
    const float ViewDistance = 3000.0f; // 시야 거리
    
    // 카메라가 보는 지면 영역 계산
    float ForwardOffset = ViewDistance * FMath::Cos(PitchRad);
    
    // 실제 보이는 영역 (타원형에 가까움)
    FVector2D Center(CameraLocation.X + ForwardOffset * 0.3f, CameraLocation.Y);
    FVector2D Extents(ViewDistance * 0.8f, ViewDistance * 0.6f);
    
    return FBox2D(Center - Extents, Center + Extents);
}
