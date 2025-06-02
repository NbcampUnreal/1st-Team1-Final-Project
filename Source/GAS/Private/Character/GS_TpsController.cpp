// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GS_TpsController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Controller.h"
#include "Character/GS_Character.h"
#include "Character/Player/GS_Player.h"
#include "Character/Interface/GS_AttackInterface.h"

AGS_TpsController::AGS_TpsController()
{
	InputMappingContext = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	WalkToggleAction = nullptr;
	/*LClickAction = nullptr;*/
}

void AGS_TpsController::Move(const FInputActionValue& InputValue)
{
	const FVector2D InputAxisVector = InputValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.0f);
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
		if (ControlValues.bCanLookRight)
		{
			ControlledPawn->AddControllerYawInput(InputAxisVector.X);
		}
		if (ControlValues.bCanLookUp)
		{
			ControlledPawn->AddControllerPitchInput(InputAxisVector.Y);
		}
	}
}

void AGS_TpsController::WalkToggle(const FInputActionValue& InputValue)
{
	
}

FControlValue& AGS_TpsController::GetControlValue()
{
	return ControlValues;
}

void AGS_TpsController::BeginPlay()
{
	Super::BeginPlay();

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
	/*if (LClickAction)
	{
		EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Started, this, &AGS_TpsController::LClickPressed);
		EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Completed, this, &AGS_TpsController::LClickRelease);
	}*/
}
