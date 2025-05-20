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
	LClickAction = nullptr;
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
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.X);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.Y);
	}
}

void AGS_TpsController::Look(const FInputActionValue& InputValue)
{
	const FVector2D InputAxisVector = InputValue.Get<FVector2D>();
	if (AGS_Character* ControlledPawn = Cast<AGS_Character>(GetPawn()))
	{
		ControlledPawn->AddControllerYawInput(InputAxisVector.X);
		ControlledPawn->AddControllerPitchInput(InputAxisVector.Y);
	}
}

void AGS_TpsController::WalkToggle(const FInputActionValue& InputValue)
{
	
}

void AGS_TpsController::LClickPressed(const FInputActionValue& InputValue)
{
	if (AGS_Player* ControlledPlayer = Cast<AGS_Player>(GetPawn()))
	{
		if (ControlledPlayer->GetClass()->ImplementsInterface(UGS_AttackInterface::StaticClass()))
		{
			IGS_AttackInterface::Execute_LeftClickPressed(ControlledPlayer);
		}
	}
}

void AGS_TpsController::LClickRelease(const FInputActionValue& InputValue)
{
	if (AGS_Player* ControlledPlayer = Cast<AGS_Player>(GetPawn()))
	{
		if (ControlledPlayer->GetClass()->ImplementsInterface(UGS_AttackInterface::StaticClass()))
		{
			IGS_AttackInterface::Execute_LeftClickRelease(ControlledPlayer);
		}
	}
}

void AGS_TpsController::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() && IsLocalController())
	{
		check(InputMappingContext);

		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
		check(Subsystem);
		Subsystem->AddMappingContext(InputMappingContext, 0);
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
	if (LClickAction)
	{
		EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Started, this, &AGS_TpsController::LClickPressed);
		EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Completed, this, &AGS_TpsController::LClickRelease);
	}
}
