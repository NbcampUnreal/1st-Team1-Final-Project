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


AGS_TpsController::AGS_TpsController()
{
	InputMappingContext = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	WalkToggleAction = nullptr;

	//KimYJ
	bReplicates = true;
	SetReplicates(true);
}

void AGS_TpsController::Move(const FInputActionValue& InputValue)
{
	const FVector2D InputAxisVector = InputValue.Get<FVector2D>();
	LastRotatorInMoving = GetControlRotation();
	const FRotator YawRotation(0.f, LastRotatorInMoving.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    if (AGS_Character* ControlledPawn = Cast<AGS_Character>(GetPawn()))
	{
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
				ControlledPawn->AddControllerPitchInput(InputAxisVector.Y * SensitivityMultiplier);
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

void AGS_TpsController::ServerRPCSpectatePlayer_Implementation()
{
	if (GetWorld()->GetGameState()->PlayerArray.IsEmpty())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Player Array EMPTY"));
		return;
	}
	
	for (const auto& PS : GetWorld()->GetGameState()->PlayerArray)
	{
		//UE_LOG(LogTemp, Warning, TEXT("---valid ps"));

		AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS);
		if (IsValid(GS_PS))
		{
			//UE_LOG(LogTemp, Warning, TEXT("---valid GS_PS"));
			if (GS_PS->bIsAlive && GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) 
			{
				AGS_TpsController* AlivePlayerController = Cast<AGS_TpsController>(GS_PS->GetPlayerController());
				
				if (IsValid(AlivePlayerController))
				{
					//UE_LOG(LogTemp, Warning, TEXT("---valid Alive PlayerPC"));
					
					APlayerController* DeadPlayerPC = Cast<APlayerController>(this);
					if (DeadPlayerPC)
					{
						//UE_LOG(LogTemp, Warning, TEXT("---valid dead player PC"));

						APawn* DeadPawn = Cast<APawn>(DeadPlayerPC->GetPawn());
						
						DeadPlayerPC->UnPossess();
						DeadPlayerPC->SetViewTargetWithBlend(AlivePlayerController);
						
						if (DeadPawn)
						{
							//UE_LOG(LogTemp, Warning, TEXT("-------%s"), *DeadPawn->GetName());
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
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("---- Remove ")),true, true, FLinearColor::Blue,5.f);
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
					//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Boss HP Init ")),true, true, FLinearColor::Red,5.f);
					BossWidget->SetOwningActor(GS_Character);
					BossWidget->InitGuardianHPWidget();
				}

				if (FeverWidget)
				{
					//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Fever Gauge Init ")),true, true, FLinearColor::Red,5.f);
					FeverWidget->InitDrakharFeverWidget();
				}
				
				if (HPBoardWidget)
				{
					//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("---- Valid HP Board Widget ")),true, true, FLinearColor::Red,5.f);
					HPBoardWidget->InitBoardWidget();
				}

				CrosshairWidget = Cast<UGS_CrossHairImage>(PlayerWidgetInstance->GetWidgetFromName(TEXT("WBP_CrossHairImage")));
				if (CrosshairWidget)
				{
					// 메르시 캐릭터에게 크로스헤어 위젯 참조 전달
					if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(GS_Character))
					{
						MerciCharacter->SetCrosshairWidget(CrosshairWidget);
					}
				}

				PlayerWidgetInstance->AddToViewport(0);
			}
		}
	}
}

void AGS_TpsController::StartAutoMoveForward()
{
	if (!IsLocalController())
	{
		Client_StartAutoMoveForward();
		return;
	}

	bIsAutoMoving = true;

	GetWorld()->GetTimerManager().SetTimer(AutoMoveTickHandle, this, &AGS_TpsController::AutoMoveTick, 0.01f, true);
}

void AGS_TpsController::StopAutoMoveForward()
{
	if (!IsLocalController())
	{
		Client_StopAutoMoveForward();
		return;
	}

	bIsAutoMoving = false;
	GetWorld()->GetTimerManager().ClearTimer(AutoMoveTickHandle);
}

void AGS_TpsController::AutoMoveTick()
{
	if (!bIsAutoMoving)
		return;

	/*UE_LOG(LogTemp, Warning, TEXT("Authority: %s"), HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));*/
	const FRotator YawRotation(0.f, GetPawn()->GetActorRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	if (AGS_Character* ControlledPawn = Cast<AGS_Character>(GetPawn()))
	{
		if (UCharacterMovementComponent* Movement = ControlledPawn->GetCharacterMovement())
		{
			/*UE_LOG(LogTemp, Warning, TEXT("MovementMode: %d | Velocity: %s | MaxWalkSpeed: %f | RootMotion: %s"),
				(int32)Movement->MovementMode,
				*Movement->Velocity.ToString(),
				Movement->MaxWalkSpeed,
				ControlledPawn->IsPlayingRootMotion() ? TEXT("true") : TEXT("false"));*/
		}
		ControlledPawn->AddMovementInput(ForwardDirection, 3.0f);
	}
}

void AGS_TpsController::Client_StopAutoMoveForward_Implementation()
{
	StopAutoMoveForward();
}

void AGS_TpsController::Client_StartAutoMoveForward_Implementation()
{
	StartAutoMoveForward();
}

void AGS_TpsController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		TestFunction();
	}

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
		//ServerRPCTestFunction();
		TestFunction();
	}
}