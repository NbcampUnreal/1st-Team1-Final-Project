// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GS_TpsController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Controller.h"
#include "Character/GS_Character.h"
#include "Character/Player/GS_Player.h"
#include "Character/Interface/GS_AttackInterface.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "System/GS_PlayerState.h"


AGS_TpsController::AGS_TpsController()
{
	InputMappingContext = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	WalkToggleAction = nullptr;
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

void AGS_TpsController::InitControllerPerWorld()
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
			if (GS_PS->bIsAlive) //[TODO] GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker 
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

void AGS_TpsController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		AGS_Character* GS_Character = Cast<AGS_Character>(GetPawn());
		if (IsValid(GS_Character))
		{
			TSubclassOf<UUserWidget> Widget = PlayerWidgetClasses[GS_Character->GetCharacterType()];
			if (IsValid(Widget))
			{
				PlayerWidgetInstance = CreateWidget<UUserWidget>(this, Widget);
				if (IsValid(PlayerWidgetInstance))
				{
					PlayerWidgetInstance->AddToViewport(0);
				}
			}
		}
	}
	
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
		AddWidget();
	}
}
