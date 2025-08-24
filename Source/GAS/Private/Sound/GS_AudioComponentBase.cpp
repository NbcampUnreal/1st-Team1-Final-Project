#include "Sound/GS_AudioComponentBase.h"
#include "AkAudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "AI/RTS/GS_RTSController.h"
#include "AI/RTS/GS_RTSCamera.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"

// RTPC 이름을 상수로 정의
const FName UGS_AudioComponentBase::DistanceToPlayerRTPCName = TEXT("Distance_to_Player");
const FName UGS_AudioComponentBase::AttenuationModeRTPCName = TEXT("Attenuation_Mode");
const FName UGS_AudioComponentBase::OcclusionDisableRTPCName = TEXT("Occlusion_Disable");

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
    
    // 초기 Distance Scaling 및 오클루전 설정 (TPS 모드 기본값)
    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        const float InitialScalingValue = TPSDistanceScaling;
        AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), InitialScalingValue, 0, GetOwner());
        
        // TPS 모드 기본값이므로 오클루전 활성화 (0.0f)
        AkAudioDevice->SetRTPCValue(*OcclusionDisableRTPCName.ToString(), 0.0f, 0, GetOwner());
    }
    
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
    
    // 2. 화면 영역 계산
    FBox2D ScreenBounds = CalculateScreenWorldBounds(RTSController);
    
    // 화면 영역 유효성 검사
    if (ScreenBounds.GetExtent().IsZero())
    {
        return true; // 유효하지 않은 화면 영역인 경우 소리 재생 허용
    }
    
    // 3. 십자 형태 맵을 위한 특별 처리
    // 소스가 화면 안에 있는지 체크
    FVector2D SourcePos2D(SourceLocation.X, SourceLocation.Y);
    bool bInScreen = ScreenBounds.IsInside(SourcePos2D);
    
    if (bInScreen)
    {
        return true; // 화면 안에 있으면 무조건 재생
    }
    
    // 4. 화면 밖이지만 가까운 경우 체크
    // 십자 형태의 통로를 고려한 거리 계산
    float DistanceToScreen = CalculateDistanceToScreenBounds(SourcePos2D, ScreenBounds);
    
    // 거리 유효성 검사
    if (DistanceToScreen < 0.0f || FMath::IsNaN(DistanceToScreen))
    {
        return true; // 유효하지 않은 거리인 경우 소리 재생 허용
    }
    
    // 5. 거리 기반 임계값 (스타크래프트 스타일)
    const float AudioExtendDistance = 800.0f; // 화면 밖 8m까지 들림
    
    // 6. 통로/복도 감지 (십자 형태 맵 특별 처리)
    bool bInCorridor = IsInCorridorRange(CameraLocation, SourceLocation);
    
    if (bInCorridor)
    {
        // 복도/통로에 있는 경우 더 멀리까지 들림
        const float CorridorAudioDistance = 1500.0f; // 15m
        return DistanceToScreen <= CorridorAudioDistance;
    }
    
    // 7. 일반적인 경우
    return DistanceToScreen <= AudioExtendDistance;
}

// 추가 헬퍼 함수: RTS 카메라의 실제 보이는 영역 계산
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
    
    // Get local player controller (캐싱하지 않음 - 자주 바뀔 수 있음)
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
            
            // RTS 카메라에서 실제 카메라 컴포넌트 찾기
            UCameraComponent* CameraComp = RTSCameraActor->FindComponentByClass<UCameraComponent>();
            if (CameraComp)
            {
                NewCameraLocation = CameraComp->GetComponentLocation();
            }
            else
            {
                // SpringArm을 통한 카메라 위치 계산
                USpringArmComponent* SpringArmComp = RTSCameraActor->FindComponentByClass<USpringArmComponent>();
                if (SpringArmComp)
                {
                    // SpringArm의 자식 카메라 찾기
                    TArray<USceneComponent*> Children;
                    SpringArmComp->GetChildrenComponents(true, Children);
                    
                    bool bFoundChildCamera = false;
                    for (USceneComponent* Child : Children)
                    {
                        if (UCameraComponent* ChildCamera = Cast<UCameraComponent>(Child))
                        {
                            NewCameraLocation = ChildCamera->GetComponentLocation();
                            bFoundChildCamera = true;
                            break;
                        }
                    }
                    
                    if (!bFoundChildCamera)
                    {
                        // SpringArm 끝점으로 카메라 위치 계산
                        NewCameraLocation = SpringArmComp->GetComponentLocation() + SpringArmComp->GetForwardVector() * SpringArmComp->TargetArmLength;
                    }
                }
                else
                {
                    // 최후에는 RTS 카메라 액터의 위치
                    NewCameraLocation = RTSCameraActor->GetActorLocation();
                }
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
                // 캐시 업데이트
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
    
    // 완료된 사운드 ID들을 제거
    for (int32 i = ActivePlayingIDs.Num() - 1; i >= 0; --i)
    {
        // Wwise에서 사운드가 여전히 재생 중인지 확인하는 로직을 추가할 수 있음
        // 현재는 잘못된 ID들만 제거
        if (ActivePlayingIDs[i] == AK_INVALID_PLAYING_ID)
        {
            ActivePlayingIDs.RemoveAt(i);
        }
    }
    
    // 배열 크기가 너무 커지지 않도록 제한
    if (ActivePlayingIDs.Num() > MaxActivePlayingIDs)
    {
        ActivePlayingIDs.RemoveAt(0, ActivePlayingIDs.Num() - MaxActivePlayingIDs);
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
        
        // 주기적으로 완료된 사운드 정리
        CleanupFinishedSounds();
    }
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
                if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
                {
                    AkAudioDevice->SetRTPCValue(*DistanceToPlayerRTPCName.ToString(), DistanceToListener, 0, GetOwner());
                    LastDistanceRTPCValue = DistanceToListener;
                    LastRTPCUpdateTime = CurrentTime;
                }
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
    if (FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get())
    {
        const float ScalingValue = GetDistanceScalingForMode(bIsRTS);
        AkAudioDevice->SetRTPCValue(*AttenuationModeRTPCName.ToString(), ScalingValue, 0, GetOwner());
        
        // RTS 모드에서는 오클루전/오브스트럭션 비활성화
        const float OcclusionValue = bIsRTS ? 1.0f : 0.0f; // 1.0f = 비활성화, 0.0f = 활성화
        AkAudioDevice->SetRTPCValue(*OcclusionDisableRTPCName.ToString(), OcclusionValue, 0, GetOwner());
    }
    else
    {
        // CRITICAL: Wwise AudioDevice가 없는 경우 경고
        UE_LOG(LogTemp, Error, TEXT("[CRITICAL] FAkAudioDevice::Get() returned NULL - Wwise audio system failure!"));
        UE_LOG(LogTemp, Error, TEXT("[CRITICAL] This may be caused by WAAPI connection failure or Wwise initialization problems"));
    }
}

FBox2D UGS_AudioComponentBase::CalculateScreenWorldBounds(AGS_RTSController* RTSController) const
{
    FBox2D Bounds(ForceInit);
    
    if (!RTSController->PlayerCameraManager)
    {
        // 기본값 반환
        return FBox2D(FVector2D(-2000, -2000), FVector2D(2000, 2000));
    }
    
    // 1. 카메라 정보 가져오기 (FOV, 스프링암 각도 등)
    float CameraFOV = 90.0f; // 기본값
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
        
        UE_LOG(LogTemp, Verbose, TEXT("[Audio] FOV Calculation - Height: %.1f, H: %.1f, V: %.1f"), 
               CameraHeight, HalfHorizontalSize, HalfVerticalSize);
        
        // 스프링암 각도 보정 적용
        if (FMath::Abs(SpringArmPitch) > 5.0f) // 카메라가 기울어져 있으면 보정
        {
            float PitchMultiplier = CalculateFOVPitchCorrection(SpringArmPitch, CameraHeight);
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
    
    // FOV 기반 계산이 실패한 경우 기존 방식 사용
    // 4. 스프링암 각도에 따른 보정 계수 계산 (기존 방식 폴백)
    float PitchCorrectionFactor = CalculatePitchCorrectionFactor(SpringArmPitch);
    float YawCorrectionFactor = CalculateYawCorrectionFactor(SpringArmYaw);
    
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
                
                // 스프링암 각도에 따른 보정 적용
                FVector2D CorrectedPos = ApplySpringArmCorrection(
                    FVector2D(GroundPos.X, GroundPos.Y), 
                    SpringArmPitch, 
                    SpringArmYaw, 
                    SpringArmLength,
                    PitchCorrectionFactor,
                    YawCorrectionFactor
                );
                
                Bounds += CorrectedPos;
            }
        }
    }
    
    // 6. 스프링암 각도에 따른 추가 보정
    Bounds = ApplyFinalSpringArmCorrection(Bounds, SpringArmPitch, SpringArmYaw, SpringArmLength);
    
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
    // 십자 형태 맵의 통로 감지
    // 카메라와 소스가 직선 통로로 연결되어 있는지 체크
    
    FVector2D CameraPos2D(CameraLocation.X, CameraLocation.Y);
    FVector2D SourcePos2D(SourceLocation.X, SourceLocation.Y);
    
    // X축 또는 Y축 정렬 체크 (통로는 보통 직선)
    // 더 관대한 임계값으로 설정하여 자연스러운 오디오 경험 제공
    const float AlignmentThreshold = 600.0f; // 6m 오차 허용 (기존 3m에서 증가)
    
    bool bXAligned = FMath::Abs(CameraPos2D.X - SourcePos2D.X) < AlignmentThreshold;
    bool bYAligned = FMath::Abs(CameraPos2D.Y - SourcePos2D.Y) < AlignmentThreshold;
    
    // 십자 형태에서는 X 또는 Y 중 하나가 정렬되어 있으면 통로
    return bXAligned || bYAligned;
}



// ===================
// 스프링암 각도 보정 함수들
// ===================

float UGS_AudioComponentBase::CalculatePitchCorrectionFactor(float PitchAngle) const
{
    // Pitch 각도는 -90도(수직 아래)에서 0도(수평)까지
    // 과도한 보정을 방지하고 안정적인 범위로 제한
    
    // 각도를 라디안으로 변환
    float PitchRad = FMath::DegreesToRadians(PitchAngle);
    
    // 코사인 값을 사용하여 보정 계수 계산 (안정적인 범위로 제한)
    // -90도일 때: cos(-90°) = 0, 보정 계수 = 0.7 (화면 영역 30% 축소)
    // -60도일 때: cos(-60°) = 0.5, 보정 계수 = 0.85
    // -50도일 때: cos(-50°) = 0.64, 보정 계수 = 0.93
    // 0도일 때: cos(0°) = 1, 보정 계수 = 1.0 (보정 없음)
    
    float CosValue = FMath::Abs(FMath::Cos(PitchRad));
    float CorrectionFactor = FMath::Lerp(0.7f, 1.0f, CosValue); // 안정적인 범위로 제한
    
    return CorrectionFactor;
}

float UGS_AudioComponentBase::CalculateFOVPitchCorrection(float PitchAngle, float CameraHeight) const
{
    // FOV 기반 Pitch 보정 - Perspective 투영에서 카메라가 기울어졌을 때의 정확한 보정
    // PitchAngle: 음수 = 아래로 기울임, 양수 = 위로 기울임
    
    float PitchRad = FMath::DegreesToRadians(PitchAngle);
    
    // 카메라가 아래로 기울어져 있으면 보이는 영역이 더 넓어짐 (원근 효과)
    // 카메라가 위로 기울어져 있으면 보이는 영역이 좁아짐
    
    if (PitchAngle < -10.0f) // 아래로 많이 기울어진 경우 (-10도 이하)
    {
        // 원근 효과로 인해 더 넓은 영역이 보임
        float PitchFactor = FMath::Abs(PitchAngle) / 90.0f; // 0.0 ~ 1.0
        return FMath::Lerp(1.0f, 2.5f, PitchFactor); // 최대 2.5배까지 확장
    }
    else if (PitchAngle > 10.0f) // 위로 많이 기울어진 경우 (10도 이상)
    {
        // 하늘을 보게 되어 지면 영역이 줄어듦
        float PitchFactor = PitchAngle / 90.0f; // 0.0 ~ 1.0
        return FMath::Lerp(1.0f, 0.5f, PitchFactor); // 최대 50%까지 축소
    }
    else
    {
        // -10도 ~ 10도 범위에서는 선형 보정
        float LinearFactor = FMath::Abs(PitchAngle) / 10.0f;
        if (PitchAngle < 0) // 아래로 기울어짐
        {
            return FMath::Lerp(1.0f, 1.3f, LinearFactor);
        }
        else // 위로 기울어짐
        {
            return FMath::Lerp(1.0f, 0.8f, LinearFactor);
        }
    }
}

float UGS_AudioComponentBase::CalculateYawCorrectionFactor(float YawAngle) const
{
    // Yaw 각도는 회전 방향에 따라 화면 영역이 달라짐
    // 일반적으로 Yaw는 큰 영향을 주지 않지만, 극단적인 각도에서는 보정 필요
    
    // 각도를 -180~180 범위로 정규화
    YawAngle = FMath::Fmod(YawAngle + 180.0f, 360.0f) - 180.0f;
    
    // Yaw 각도가 극단적일 때만 보정 (예: 90도, -90도)
    float AbsYaw = FMath::Abs(YawAngle);
    if (AbsYaw > 45.0f)
    {
        // 극단적인 Yaw 각도에서는 화면 영역이 약간 축소됨
        float YawRad = FMath::DegreesToRadians(AbsYaw);
        float SinValue = FMath::Sin(YawRad);
        return FMath::Lerp(1.0f, 0.8f, SinValue);
    }
    
    return 1.0f; // 기본값
}

FVector2D UGS_AudioComponentBase::ApplySpringArmCorrection(
    const FVector2D& OriginalPos, 
    float PitchAngle, 
    float YawAngle, 
    float ArmLength,
    float PitchCorrectionFactor,
    float YawCorrectionFactor) const
{
    // 1. Pitch 각도에 따른 거리 보정
    // Pitch가 낮을수록(수평에 가까울수록) 화면 영역이 넓어짐
    float PitchDistanceMultiplier = PitchCorrectionFactor;
    
    // 2. Yaw 각도에 따른 방향 보정
    // Yaw가 극단적일 때 화면 영역이 약간 축소됨
    float YawDistanceMultiplier = YawCorrectionFactor;
    
    // 3. 스프링암 길이에 따른 보정
    // 스프링암이 길수록 화면 영역이 넓어짐
    // 줌인할 때도 적절한 보정을 위해 동적 보정 시스템 적용
    float LengthMultiplier = CalculateZoomCorrectionFactor(ArmLength);
    
    // 4. 최종 보정 계수 계산
    float TotalCorrectionFactor = PitchDistanceMultiplier * YawDistanceMultiplier * LengthMultiplier;
    
    // 5. 위치 보정 적용
    FVector2D CorrectedPos = OriginalPos * TotalCorrectionFactor;
    
    return CorrectedPos;
}

FBox2D UGS_AudioComponentBase::ApplyFinalSpringArmCorrection(
    const FBox2D& OriginalBounds, 
    float PitchAngle, 
    float YawAngle, 
    float ArmLength) const
{
    FBox2D CorrectedBounds = OriginalBounds;
    
    // 1. Pitch 각도에 따른 최종 보정 (안정적인 범위로 제한)
    float PitchRad = FMath::DegreesToRadians(PitchAngle);
    float PitchMultiplier = FMath::Lerp(1.2f, 0.9f, FMath::Abs(FMath::Cos(PitchRad))); // 기존 1.5f~0.8f에서 1.2f~0.9f로 제한
    
    // 2. 스프링암 길이에 따른 최종 보정
    float LengthMultiplier = CalculateZoomCorrectionFactor(ArmLength);
    
    // 3. 최종 보정 계수 (과도한 보정 방지)
    float FinalMultiplier = FMath::Clamp(PitchMultiplier * LengthMultiplier, 0.7f, 1.8f);
    
    // 4. 경계 상자 확장/축소
    FVector2D Center = CorrectedBounds.GetCenter();
    FVector2D Extent = CorrectedBounds.GetExtent() * FinalMultiplier;
    
    CorrectedBounds = FBox2D(Center - Extent, Center + Extent);
    
    return CorrectedBounds;
}

float UGS_AudioComponentBase::CalculateZoomCorrectionFactor(float ArmLength) const
{
    // 줌 레벨에 따른 동적 보정 계수 계산
    // 줌인할 때와 줌아웃할 때를 구분하여 적절한 보정 적용
    
    // 1. 기본 줌 레벨 (2000 = 100%)
    const float BaseZoomLevel = 2000.0f;
    const float ZoomRatio = ArmLength / BaseZoomLevel;
    
    // 2. 줌 레벨별 보정 계수
    float CorrectionFactor = 1.0f;
    
    if (ZoomRatio < 0.5f)
    {
        // 줌인 (50% 이하): 화면 영역이 좁아지므로 보정 필요
        // 0.5배 줌일 때 0.85배 보정, 0.1배 줌일 때 0.95배 보정
        float ZoomInFactor = FMath::Lerp(0.95f, 0.85f, (0.5f - ZoomRatio) / 0.4f);
        CorrectionFactor = ZoomInFactor;
    }
    else if (ZoomRatio > 1.5f)
    {
        // 줌아웃 (150% 이상): 화면 영역이 넓어지므로 보정 필요
        // 1.5배 줌일 때 1.15배 보정, 3.0배 줌일 때 1.5배 보정
        float ZoomOutFactor = FMath::Lerp(1.15f, 1.5f, (ZoomRatio - 1.5f) / 1.5f);
        CorrectionFactor = ZoomOutFactor;
    }
    else
    {
        // 중간 줌 레벨 (50% ~ 150%): 자연스러운 보정
        // 1.0배 줌일 때 1.0배 보정, 1.5배 줌일 때 1.15배 보정
        float MidZoomFactor = FMath::Lerp(1.0f, 1.15f, (ZoomRatio - 0.5f) / 1.0f);
        CorrectionFactor = MidZoomFactor;
    }
    
    return CorrectionFactor;
}