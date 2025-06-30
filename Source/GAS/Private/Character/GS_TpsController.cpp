// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GS_TpsController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
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
#include "System/GameMode/GS_InGameGM.h"


AGS_TpsController::AGS_TpsController()
{
	InputMappingContext = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	WalkToggleAction = nullptr;

	//KimYJ
	bReplicates = true;
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
				float TurnSpeed = 0.4f;

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
	ControlValues.bCanMoveForward = CanMoveForward;
	ControlValues.bCanMoveRight = CanMoveRight;
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
		check(InputMappingContext);

		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
		check(Subsystem);
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

void AGS_TpsController::Server_NotifyPlayerIsReady_Implementation()
{
	if (AGS_BaseGM* GM = GetWorld()->GetAuthGameMode<AGS_BaseGM>())
	{
		GM->NotifyPlayerIsReady(this);
	}
}

void AGS_TpsController::Client_StartGame_Implementation()
{
	TestFunction();
	UE_LOG(LogTemp, Warning, TEXT("준비 완료. TODO: 화면 가리개 제거"));
	//TODO: 로딩스크린 제거
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
	bIsAutoMoving = false;
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

	bIsAutoMoving = false;
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

void AGS_TpsController::AutoMoveTick()
{
	if (!bIsAutoMoving)
	{
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

void AGS_TpsController::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UGS_GameInstance>(GetGameInstance());
	
	InitControllerPerWorld();
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
		Server_NotifyPlayerIsReady();
	}
}