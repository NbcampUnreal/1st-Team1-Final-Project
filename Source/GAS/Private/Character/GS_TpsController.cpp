// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GS_TpsController.h"
#include "Character/Component/Seeker/GS_MarkerPlacementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Controller.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Character/Interface/GS_AttackInterface.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "System/GS_PlayerState.h"
#include "UI/Character/GS_HPBoardWidget.h"
#include "System/GS_GameInstance.h"
#include "UI/Character/GS_BossHP.h"
#include "UI/Character/GS_DrakharFeverGauge.h"
#include "UI/Character/GS_FeverGaugeBoard.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Character/Component/GS_DebuffComp.h"
#include "System/GameMode/GS_InGameGM.h"
#include "EngineUtils.h"  // TActorIterator
#include "UI/Character/GS_ReviveIndicatorWidget.h"


AGS_TpsController::AGS_TpsController()
{
	InputMappingContext = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	WalkToggleAction = nullptr;

	//KimYJ
	bReplicates = true;
	bIsAutoMoving = false;
	//SetReplicates(true); 신중은
}

void AGS_TpsController::Move(const FInputActionValue& InputValue)
{
	const FVector2D InputAxisVector = InputValue.Get<FVector2D>();
	LastRotatorInMoving = GetControlRotation();
	Server_CacheMoveInputValue(InputAxisVector);
	if (AGS_Character* ControlledPawn = Cast<AGS_Character>(GetPawn()))
	{
		// 자동 이동 시 좌우 이동 (KCY)
		if (bIsAutoMoving)
		{
			if (!FMath::IsNearlyZero(InputAxisVector.Y))
			{
				float TurnSpeed = 0.7f;

				if (IsLocalController())
				{
					FRotator ControlRot = GetControlRotation();
					ControlRot.Yaw += InputAxisVector.Y * TurnSpeed;
					SetControlRotation(ControlRot);
				}
			}
			return;
		}

		// 일반 이동
		const FRotator YawRotation(0.f, LastRotatorInMoving.Yaw, 0.0f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (ControlValues.bCanMoveForward)
		{
			ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.X);
		}
		if (ControlValues.bCanMoveRight)
		{
			ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.Y);
		}
	}
}

void AGS_TpsController::Look(const FInputActionValue& InputValue)
{
	if (bIsAutoMoving)
	{
		// 자동 이동 중에는 마우스 회전 무시
		return;
	}

	const FVector2D InputAxisVector = InputValue.Get<FVector2D>();
	if (AGS_Character* ControlledPawn = Cast<AGS_Character>(GetPawn()))
	{
		if(GameInstance)
		{
			float SensitivityMultiplier = GameInstance->GetMouseSensitivity();
			if (ControlValues.bCanLookRight)
			{
				ControlledPawn->AddControllerYawInput(InputAxisVector.X * SensitivityMultiplier);
			}			
			
			if (ControlValues.bCanLookUp)
			{
				FRotator CurrentRot = GetControlRotation();

				// 위로 50, 아래로 80
				const float PitchMin = -80.f;
				const float PitchMax = 50.f;

				float NewPitch = CurrentRot.Pitch + (-InputAxisVector.Y * SensitivityMultiplier);
				NewPitch = FMath::ClampAngle(NewPitch, PitchMin, PitchMax);

				CurrentRot.Pitch = NewPitch;
				SetControlRotation(CurrentRot);
				
				//ControlledPawn->AddControllerPitchInput(InputAxisVector.Y * SensitivityMultiplier);
			}
		}
	}
}

void AGS_TpsController::WalkToggle(const FInputActionValue& InputValue)
{
	if (AGS_Seeker* ControlledPawn = Cast<AGS_Seeker>(GetPawn()))
	{
		if (ControlledPawn->CanChangeSeekerGait)
		{
			EGait CurGait = ControlledPawn->GetSeekerGait();
		
			if (CurGait == EGait::Walk)
			{
				ControlledPawn->Server_SetSeekerGait(EGait::Run);
			}
			else if (CurGait == EGait::Run)
			{
				ControlledPawn->Server_SetSeekerGait(EGait::Walk);
			}
		}
	}
}

void AGS_TpsController::PlaceMarker(const FInputActionValue& InputValue)
{
	if (AGS_Seeker* ControlledPawn = Cast<AGS_Seeker>(GetPawn()))
	{
		if (ControlledPawn->MarkerPlacementComponent)
		{
			ControlledPawn->MarkerPlacementComponent->TryPlaceMarker();
		}
	}
}

FControlValue AGS_TpsController::GetControlValue() const
{
	return ControlValues;
}

void AGS_TpsController::PageUp(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		ServerRPCSpectatePlayer();
	}
}

void AGS_TpsController::PageDown(const FInputActionValue& InputValue)
{

}

void AGS_TpsController::SetMoveControlValue(bool CanMoveRight, bool CanMoveForward)
{
	// 움직임 활성화 시 Stun 디버프 상태인지 확인 (KCY)
	if (AGS_Character* ControlledPawn = Cast<AGS_Character>(GetPawn()))
	{
		if ((CanMoveRight || CanMoveForward) && ControlledPawn->GetDebuffComp()->IsDebuffActive(EDebuffType::Stun))
		{
			// Stun 디버프 상태라면 ControlValues 값 변경하지 않음
			return;
		}
	}

	ControlValues.bCanMoveForward = CanMoveForward;
	ControlValues.bCanMoveRight = CanMoveRight;
}

FControlValue AGS_TpsController::GetMoveControlValue()
{
	return ControlValues;
}

void AGS_TpsController::SetLookControlValue(bool CanLookRight, bool CanLookUp)
{
	ControlValues.bCanLookUp = CanLookUp;
	ControlValues.bCanLookRight = CanLookRight;
}

void AGS_TpsController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGS_TpsController, ControlValues);
	DOREPLIFETIME(AGS_TpsController, MoveInputValue);
	DOREPLIFETIME(AGS_TpsController, bIsAutoMoving);
	DOREPLIFETIME(AGS_TpsController, bIsHoldingReviveKey);
}

float AGS_TpsController::GetCurrentMouseSensitivity() const
{
	if (GameInstance)
	{
		return GameInstance->GetMouseSensitivity();
	}
	return 1.0f;
}

void AGS_TpsController::InitControllerPerWorld()
{
	SetInputMode(FInputModeGameOnly());

	if (!HasAuthority() && IsLocalController())
	{
		if (!InputMappingContext)
		{
			UE_LOG(LogTemp, Error, TEXT("[Revive] InputMappingContext is NULL! Please assign IMC_Seeker in BP_PlayerController!"));
			return;
		}

		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
		if (!Subsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[Revive] Failed to get EnhancedInputLocalPlayerSubsystem"));
			return;
		}

		Subsystem->AddMappingContext(InputMappingContext, 0);

		// 오디오 리스너 설정 (약간의 지연을 두고 실행)
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			this,
			&AGS_TpsController::SetupPlayerAudioListener,
			0.1f,
			false
		);
	}
}

void AGS_TpsController::ServerRPCSpectatePlayer_Implementation()
{
	if (GetWorld()->GetGameState()->PlayerArray.IsEmpty())
	{
		return;
	}
	
	for (const auto& PS : GetWorld()->GetGameState()->PlayerArray)
	{
		AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS);
		if (IsValid(GS_PS))
		{
			if (GS_PS->bIsAlive && GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) 
			{
				AGS_TpsController* AlivePlayerController = Cast<AGS_TpsController>(GS_PS->GetPlayerController());
				
				if (IsValid(AlivePlayerController))
				{
					APlayerController* DeadPlayerPC = Cast<APlayerController>(this);
					if (DeadPlayerPC)
					{
						APawn* DeadPawn = Cast<APawn>(DeadPlayerPC->GetPawn());
						
						DeadPlayerPC->UnPossess();
						DeadPlayerPC->SetViewTargetWithBlend(AlivePlayerController);
						
						if (DeadPawn)
						{
							DeadPawn->SetLifeSpan(2.f);
						}
					}
				}
			}
		}
		//Game Over?
		//all players dead
	}
}

void AGS_TpsController::Server_CacheMoveInputValue_Implementation(FVector2D InputValue)
{
	MoveInputValue = InputValue;
}

void AGS_TpsController::TestFunction()
{
	AGS_Character* GS_Character = Cast<AGS_Character>(GetPawn());
	if (IsValid(GS_Character))
	{		
		TSubclassOf<UUserWidget> Widget = PlayerWidgetClasses[GS_Character->GetCharacterType()];
		if (IsValid(Widget))
		{
			if (PlayerWidgetInstance)
			{
				PlayerWidgetInstance->RemoveFromParent();
				PlayerWidgetInstance = nullptr;
				CrosshairWidget = nullptr;
			}
			
			PlayerWidgetInstance = CreateWidget<UUserWidget>(this, Widget);
			if (IsValid(PlayerWidgetInstance))
			{
				UGS_HPBoardWidget* HPBoardWidget = Cast<UGS_HPBoardWidget>(PlayerWidgetInstance->GetWidgetFromName(TEXT("WBP_HPBoard")));
				UGS_BossHP* BossWidget = Cast<UGS_BossHP>(PlayerWidgetInstance->GetWidgetFromName(TEXT("WBP_BossHPBoard")));
				UGS_FeverGaugeBoard* FeverWidget = Cast<UGS_FeverGaugeBoard>(PlayerWidgetInstance->GetWidgetFromName(TEXT("WBP_FeverBoard")));
				
				if (BossWidget)
				{
					//HP
					BossWidget->SetOwningActor(GS_Character);
					BossWidget->InitGuardianHPWidget();
				}

				if (FeverWidget)
				{
					FeverWidget->InitDrakharFeverWidget();
				}
				
				if (HPBoardWidget)
				{
					HPBoardWidget->InitBoardWidget();
				}

				PlayerWidgetInstance->AddToViewport(0);

				CrosshairWidget = Cast<UGS_CrossHairImage>(PlayerWidgetInstance->GetWidgetFromName(TEXT("WBP_CrossHairImage")));
				if (CrosshairWidget)
				{
					// 메르시 캐릭터에게 크로스헤어 위젯 참조 전달
					if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(GS_Character))
					{
						MerciCharacter->SetCrosshairWidget(CrosshairWidget);
					}
				}
			}
		}
	}
}

void AGS_TpsController::StartAutoMoveForward()
{
	bIsAutoMoving = true;
	Client_StartAutoMoveForward(); // 돌진 시작	
}

void AGS_TpsController::StopAutoMoveForward()
{
	//bIsAutoMoving = false;
	Client_StopAutoMoveForward();
}

void AGS_TpsController::Client_StartAutoMoveForward_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	bIsAutoMoving = true;
	SaveOriginalCameraSettings();
	ApplyChargeCameraSettings(true);
	SnapCameraToCharacterYaw();
	GetWorld()->GetTimerManager().SetTimer(AutoMoveTickHandle, this, &AGS_TpsController::AutoMoveTick, 0.01f, true);
}

void AGS_TpsController::Client_StopAutoMoveForward_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	//bIsAutoMoving = false;
	GetWorld()->GetTimerManager().ClearTimer(AutoMoveTickHandle);
	RestoreOriginalCameraSettings();
}

void AGS_TpsController::SnapCameraToCharacterYaw()
{
	if (!IsLocalController())
	{
		return;
	}

	if (APawn* MyPawn = GetPawn())
	{
		// 카메라를 캐릭터 뒤쪽으로 정렬
		FRotator CharacterRotation = MyPawn->GetActorRotation();
		SetControlRotation(CharacterRotation);
	}
}

void AGS_TpsController::SetIsAutoMoving(bool InIsAutoMoving)
{
	if(HasAuthority())
	{
		bIsAutoMoving = InIsAutoMoving;
	}
}

void AGS_TpsController::AutoMoveTick()
{
	if (!IsValid(this) || !IsLocalController())
	{
		return;
	}
	
	if (!bIsAutoMoving)
	{
		return;
	}

	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	if (!IsValid(ControlledCharacter) || !ControlledCharacter->GetCharacterMovement())
	{
		StopAutoMoveForward(); // 안전하게 정지
		return;
	}

	const FRotator YawRotation(0.f, GetPawn()->GetActorRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	if (AGS_Character* ControlledPawn = Cast<AGS_Character>(GetPawn()))
	{
		ControlledPawn->AddMovementInput(ForwardDirection, 10.0f);
	}
}

void AGS_TpsController::SaveOriginalCameraSettings()
{
	if (APawn* MyPawn = GetPawn())
	{
		bOriginalUseControllerRotationYaw = MyPawn->bUseControllerRotationYaw;

		if (AGS_Character* MyCharacter = Cast<AGS_Character>(MyPawn))
		{
			if (UCharacterMovementComponent* Movement = MyCharacter->GetCharacterMovement())
			{
				bOriginalOrientRotationToMovement = Movement->bOrientRotationToMovement;
			}

			if (USpringArmComponent* SpringArm = MyCharacter->FindComponentByClass<USpringArmComponent>())
			{
				bOriginalUsePawnControlRotation = SpringArm->bUsePawnControlRotation;
				bOriginalEnableCameraLag = SpringArm->bEnableCameraLag;
				bOriginalEnableCameraRotationLag = SpringArm->bEnableCameraRotationLag;
				bOriginalInheritYaw = SpringArm->bInheritYaw;
			}
		}
	}
}

void AGS_TpsController::RestoreOriginalCameraSettings()
{
	if (APawn* MyPawn = GetPawn())
	{
		MyPawn->bUseControllerRotationYaw = bOriginalUseControllerRotationYaw;

		if (AGS_Character* MyCharacter = Cast<AGS_Character>(MyPawn))
		{
			if (UCharacterMovementComponent* Movement = MyCharacter->GetCharacterMovement())
			{
				Movement->bOrientRotationToMovement = bOriginalOrientRotationToMovement;
			}

			if (USpringArmComponent* SpringArm = MyCharacter->FindComponentByClass<USpringArmComponent>())
			{
				SpringArm->bUsePawnControlRotation = bOriginalUsePawnControlRotation;
				SpringArm->bEnableCameraLag = bOriginalEnableCameraLag;
				SpringArm->bEnableCameraRotationLag = bOriginalEnableCameraRotationLag;
				SpringArm->bInheritYaw = bOriginalInheritYaw;
			}
		}
	}

	// 마우스 입력 복구
	SetLookControlValue(true, true);
}

void AGS_TpsController::ApplyChargeCameraSettings(bool bCharging)
{
	if (!IsLocalController())
	{
		return; // 로컬 컨트롤러에서만 실행
	}

	if (APawn* MyPawn = GetPawn())
	{
		// === 게임플레이에 영향을 주는 설정 (서버-클라 동기화 필요) ===
		// 이 부분은 서버에서도 설정되어야 함 (별도 함수로 분리 권장)
		MyPawn->bUseControllerRotationYaw = true;

		if (AGS_Character* MyCharacter = Cast<AGS_Character>(MyPawn))
		{
			if (UCharacterMovementComponent* Movement = MyCharacter->GetCharacterMovement())
			{
				Movement->bOrientRotationToMovement = false;
			}

			// === 시각적 표현만 담당하는 설정 (클라이언트 전용) ===
			if (USpringArmComponent* SpringArm = MyCharacter->FindComponentByClass<USpringArmComponent>())
			{
				SpringArm->bUsePawnControlRotation = false;
				SpringArm->bEnableCameraLag = false;
				SpringArm->bEnableCameraRotationLag = false;
				SpringArm->bInheritYaw = true;
			}
		}
	}

	// 마우스 입력 비활성화 (클라이언트 전용)
	SetLookControlValue(false, false);
}

void AGS_TpsController::Client_DrawAimAssistDebug_Implementation(const FVector& Start, const FVector& End,
	const FVector& TargetLocation, float Duration)
{
	if (UWorld* World = GetWorld())
	{
		DrawDebugLine(World, Start, End, FColor::Yellow, false, Duration, 0, 1.5f);
		DrawDebugSphere(World, TargetLocation, 24.f, 12, FColor::Red, false, Duration);
	}
}

void AGS_TpsController::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UGS_GameInstance>(GetGameInstance());

	// Input 설정 유효성 검사 : 가디언 테스트용 코드
	// if (!InputMappingContext)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("AGS_TpsController (%s): InputMappingContext is not set! Please configure it in Blueprint."), *GetNameSafe(this));
	//}

	InitControllerPerWorld();

	// 구조 표시 위젯 생성 (로컬 컨트롤러만)
	if (IsLocalController() && ReviveIndicatorWidgetClass)
	{
		ReviveIndicatorWidget = CreateWidget<UGS_ReviveIndicatorWidget>(this, ReviveIndicatorWidgetClass);
		if (ReviveIndicatorWidget)
		{
			ReviveIndicatorWidget->AddToViewport();
		}
	}
}

UUserWidget* AGS_TpsController::GetPlayerWidget()
{
	if (!IsValid(PlayerWidgetInstance))
	{
		return nullptr;
	}
	return PlayerWidgetInstance;
}

void AGS_TpsController::SetupPlayerAudioListener()
{
    if (!IsLocalController())
    {
        return;
    }

    if (AGS_Player* ControlledPlayer = Cast<AGS_Player>(GetPawn()))
    {
        ControlledPlayer->SetupLocalAudioListener();
        UE_LOG(LogAudio, Log, TEXT("Audio listener setup initiated from controller for: %s"), *ControlledPlayer->GetName());
    }
}

void AGS_TpsController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGS_TpsController::Move);
	}
	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGS_TpsController::Look);
	}
	if (PageUpAction)
	{
		EnhancedInputComponent->BindAction(PageUpAction, ETriggerEvent::Started, this, &AGS_TpsController::PageUp);
	}
	if (PageDownAction)
	{
		EnhancedInputComponent->BindAction(PageDownAction, ETriggerEvent::Started, this, &AGS_TpsController::PageDown);
	}
	if (WalkToggleAction)
	{
		EnhancedInputComponent->BindAction(WalkToggleAction, ETriggerEvent::Started, this, &AGS_TpsController::WalkToggle);
	}
	if (PlaceMarkerAction)
	{
		EnhancedInputComponent->BindAction(PlaceMarkerAction, ETriggerEvent::Started, this, &AGS_TpsController::PlaceMarker);
	}
	
	// 빈사 플레이어 구조 (E키)
	if (ReviveAction)
	{
		EnhancedInputComponent->BindAction(ReviveAction, ETriggerEvent::Started, this, &AGS_TpsController::TryStartRevive);
		EnhancedInputComponent->BindAction(ReviveAction, ETriggerEvent::Completed, this, &AGS_TpsController::StopRevive);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Revive] ReviveAction is NULL! Please assign IA_ReviveAction in BP_PlayerController!"));
	}
}

void AGS_TpsController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	InitControllerPerWorld();
}

void AGS_TpsController::BeginPlayingState()
{
	Super::BeginPlayingState();

	UE_LOG(LogTemp, Warning, TEXT("AGS_TpsController (%s) --- BeginPlayingState CALLED ---"), *GetNameSafe(this));
	if (IsLocalController())
	{
		TestFunction();
	}
}

void AGS_TpsController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoMoveTickHandle);
	}
}

void AGS_TpsController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController())
	{
		return;
	}

	// 빈사 시커 감지 및 위젯 업데이트
	UpdateReviveIndicatorVisibility();

	// 기존 구조 중 로직 (변경 없음)
	if (bIsReviving && ReviveTarget.IsValid())
	{
		AGS_Seeker* Target = ReviveTarget.Get();

		// 구조 완료: 대상이 더 이상 빈사 상태가 아님
		if (!Target->IsInDyingState())
		{
			bIsReviving = false;
			bIsHoldingReviveKey = false;
			ReviveTarget = nullptr;

			if (ReviveIndicatorWidget)
			{
				ReviveIndicatorWidget->StopRevive();
			}
			return;
		}

		// 거리 체크: 너무 멀리 떨어짐
		if (AGS_Seeker* MySeeker = Cast<AGS_Seeker>(GetPawn()))
		{
			float Distance = FVector::Dist(MySeeker->GetActorLocation(), Target->GetActorLocation());
			if (Distance > ReviveDistance)
			{
				StopRevive(FInputActionValue());
				return;
			}
		}
	}
}

// ==========================================
// 빈사 플레이어 구조 시스템 구현
// ==========================================

void AGS_TpsController::UpdateReviveIndicatorVisibility()
{
	// 자신이 시커가 아니면 무시
	AGS_Seeker* MySeeker = Cast<AGS_Seeker>(GetPawn());
	if (!IsValid(MySeeker))
	{
		return;
	}

	// 구조 중이면 위젯 상태를 건드리지 않음 (StopRevive에서 처리)
	if (bIsReviving)
	{
		return;
	}

	// 자신이 빈사 상태면 무시
	if (MySeeker->IsInDyingState())
	{
		if (bNearbyDyingSeekerDetected && ReviveIndicatorWidget)
		{
			bNearbyDyingSeekerDetected = false;
			ReviveIndicatorWidget->HideNearbyIndicator();
		}
		return;
	}

	// 근처 빈사 시커 찾기
	AGS_Seeker* NearbyDyingSeeker = FindNearbyDyingSeeker();

	if (IsValid(NearbyDyingSeeker))
	{
		// 빈사 시커 감지됨
		if (!bNearbyDyingSeekerDetected || LastDetectedDyingSeeker != NearbyDyingSeeker)
		{
			// 새로운 빈사 시커 감지 또는 대상 변경
			bNearbyDyingSeekerDetected = true;
			LastDetectedDyingSeeker = NearbyDyingSeeker;

			if (ReviveIndicatorWidget)
			{
				// E키 안내만 표시 (진행도 바는 표시 안 함)
				ReviveIndicatorWidget->ShowNearbyIndicator(NearbyDyingSeeker);
			}
		}
	}
	else
	{
		// 빈사 시커 없음
		if (bNearbyDyingSeekerDetected)
		{
			bNearbyDyingSeekerDetected = false;
			LastDetectedDyingSeeker = nullptr;

			if (ReviveIndicatorWidget)
			{
				ReviveIndicatorWidget->HideNearbyIndicator();
			}
		}
	}
}

void AGS_TpsController::TryStartRevive(const FInputActionValue& InputValue)
{
	// 로컬 컨트롤러에서만 처리
	if (!IsLocalController())
	{
		return;
	}

	// 현재 조종 중인 캐릭터가 시커인지 확인
	AGS_Seeker* MySeeker = Cast<AGS_Seeker>(GetPawn());
	if (!IsValid(MySeeker))
	{
		return;
	}

	// 자신이 빈사 상태면 구조할 수 없음
	if (MySeeker->IsInDyingState())
	{
		return;
	}

	// 근처에 빈사 상태인 시커 찾기
	AGS_Seeker* DyingSeeker = FindNearbyDyingSeeker();
	if (!IsValid(DyingSeeker))
	{
		return;
	}

	// 구조 시작
	bIsReviving = true;
	bIsHoldingReviveKey = true;  // E키 누르기 시작 (로컬)
	ReviveTarget = DyingSeeker;

	// 위젯에 구조 시작 알림
	if (ReviveIndicatorWidget)
	{
		ReviveIndicatorWidget->StartRevive(DyingSeeker);
	}

	// 서버에 홀드 상태 전달
	Server_SetHoldingReviveKey(true);

	// 서버에 구조 요청
	Server_RequestRevive(DyingSeeker);
}

void AGS_TpsController::StopRevive(const FInputActionValue& InputValue)
{
	if (!bIsReviving)
	{
		return;
	}

	bIsReviving = false;
	bIsHoldingReviveKey = false;  // E키 떼기 (로컬)

	// 위젯은 즉시 숨기지 않음 - 진행도만 숨김 (진행도 감소를 보여주기 위해)
	if (ReviveIndicatorWidget)
	{
		ReviveIndicatorWidget->StopReviveProgress();
		// CurrentTarget은 유지 (NativeTick에서 진행도 감소를 계속 표시하기 위함)
	}

	// 서버에 구조 취소 요청 (ReviveTarget을 nullptr로 설정하기 전에 호출!)
	if (ReviveTarget.IsValid())
	{
		Server_CancelRevive();
	}

	// 서버에 홀드 상태 전달
	Server_SetHoldingReviveKey(false);

	// ReviveTarget은 nullptr로 설정 (구조 중이 아니므로)
	ReviveTarget = nullptr;
}

AGS_Seeker* AGS_TpsController::FindNearbyDyingSeeker() const
{
	AGS_Seeker* MySeeker = Cast<AGS_Seeker>(GetPawn());
	if (!IsValid(MySeeker))
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FVector MyLocation = MySeeker->GetActorLocation();
	AGS_Seeker* ClosestDyingSeeker = nullptr;
	float ClosestDistance = ReviveDistance;

	// 모든 시커를 순회하여 빈사 상태인 시커 찾기
	for (TActorIterator<AGS_Seeker> It(World); It; ++It)
	{
		AGS_Seeker* OtherSeeker = *It;

		// 자기 자신 제외
		if (OtherSeeker == MySeeker)
		{
			continue;
		}

		// 빈사 상태가 아니면 스킵
		if (!OtherSeeker->IsInDyingState())
		{
			continue;
		}

		// 거리 확인
		float Distance = FVector::Dist(MyLocation, OtherSeeker->GetActorLocation());

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestDyingSeeker = OtherSeeker;
		}
	}

	return ClosestDyingSeeker;
}

void AGS_TpsController::Server_RequestRevive_Implementation(AGS_Seeker* Target)
{
	if (!IsValid(Target))
	{
		return;
	}

	AGS_Seeker* MySeeker = Cast<AGS_Seeker>(GetPawn());
	if (!IsValid(MySeeker))
	{
		return;
	}

	// 거리 재확인 (서버 검증)
	float Distance = FVector::Dist(MySeeker->GetActorLocation(), Target->GetActorLocation());
	if (Distance > ReviveDistance)
	{
		return;
	}

	// 서버에서도 ReviveTarget 설정 (Server_CancelRevive에서 사용)
	ReviveTarget = Target;

	// 서버에서 홀드 상태 설정 (타이밍 문제 해결)
	bIsHoldingReviveKey = true;

	// 대상에게 구조 시작 알림
	Target->Server_StartRevive(MySeeker);
}

void AGS_TpsController::Server_CancelRevive_Implementation()
{
	// 서버에서 홀드 상태 해제
	bIsHoldingReviveKey = false;

	if (ReviveTarget.IsValid())
	{
		ReviveTarget->Server_CancelRevive();
	}

	// 서버에서도 ReviveTarget 초기화
	ReviveTarget = nullptr;
}

void AGS_TpsController::Server_SetHoldingReviveKey_Implementation(bool bIsHolding)
{
	bIsHoldingReviveKey = bIsHolding;
}
