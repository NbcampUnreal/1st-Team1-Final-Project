#include "DungeonEditor/GS_DEPawn.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "DungeonEditor/GS_DEController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"

AGS_DEPawn::AGS_DEPawn()
{
 	PrimaryActorTick.bCanEverTick = true;
	
	SceneComp = CreateDefaultSubobject<USceneComponent>("Scene");
	SetRootComponent(SceneComp);
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 5000;
	SpringArmComp->bUsePawnControlRotation = false;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->SetProjectionMode(ECameraProjectionMode::Type::Orthographic);
	CameraComp->SetOrthoWidth(5000);
	CameraComp->SetAutoCalculateOrthoPlanes(false);
	CameraComp->SetOrthoNearClipPlane(-2000.f);

	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));
	MovementComp->UpdatedComponent = RootComponent;
}

void AGS_DEPawn::BeginPlay()
{
	Super::BeginPlay();

	SpringArmComp->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

void AGS_DEPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// void AGS_DEPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
// {
// 	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
// 	if (UEnhancedInputComponent* EnhancedInput
// 		= Cast<UEnhancedInputComponent>(PlayerInputComponent))
// 	{
// 		if (AGS_DEController* PlayerController
// 			= Cast<AGS_DEController>(GetController()))
// 		{
// 			if (PlayerController->MoveAction)
// 			{
// 				EnhancedInput->BindAction(PlayerController->MoveAction
// 					, ETriggerEvent::Triggered
// 					,this
// 					, &AGS_DEPawn::Move);
// 			}
//
// 			if (PlayerController->ZoomAction)
// 			{
// 				EnhancedInput->BindAction(PlayerController->ZoomAction
// 					, ETriggerEvent::Triggered
// 					,this
// 					, &AGS_DEPawn::Zoom);
// 			}
//
// 			if (PlayerController->PropRotationAction)
// 			{
// 				EnhancedInput->BindAction(PlayerController->PropRotationAction
// 					, ETriggerEvent::Started
// 					,this
// 					, &AGS_DEPawn::PropRotation);
// 			}
// 			
// 			if (PlayerController->ClickLMBAction)
// 			{
// 				EnhancedInput->BindAction(PlayerController->ClickLMBAction
// 					, ETriggerEvent::Triggered
// 					,this
// 					, &AGS_DEPawn::ClickLMB);
//
// 				EnhancedInput->BindAction(PlayerController->ClickLMBAction
// 					, ETriggerEvent::Completed
// 					,this
// 					, &AGS_DEPawn::ReleasedLMB);
// 			}
// 			
// 			if (PlayerController->ClickRMBAction)
// 			{
// 				EnhancedInput->BindAction(PlayerController->ClickRMBAction
// 					, ETriggerEvent::Triggered
// 					,this
// 					, &AGS_DEPawn::ClickRMB);
//
// 				EnhancedInput->BindAction(PlayerController->ClickRMBAction
// 					, ETriggerEvent::Completed
// 					,this
// 					, &AGS_DEPawn::ReleasedRMB);
// 			}
//
// 			if (PlayerController->ClickDeleteAction)
// 			{
// 				EnhancedInput->BindAction(PlayerController->ClickDeleteAction
// 					, ETriggerEvent::Started
// 					,this
// 					, &AGS_DEPawn::ClickDelete);
// 			}
// 		}
// 	}
// }

void AGS_DEPawn::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2d MoveInput = Value.Get<FVector2d>();

	float MoveSpeed = CameraComp->OrthoWidth / 2500.0f;
	
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X * MoveSpeed);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y * MoveSpeed);
	}
}

void AGS_DEPawn::Zoom(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (AGS_DEController* DEController = Cast<AGS_DEController>(Controller))
	{
		const float ZoomInput = Value.Get<float>();

		if (!FMath::IsNearlyZero(ZoomInput))
		{
			CameraComp->OrthoWidth += ZoomInput * DEController->GetZoomSpeed();
			CameraComp->OrthoWidth = FMath::Clamp(CameraComp->OrthoWidth, 300.0f, CameraComp->OrthoWidth);
		}
	}
}

void AGS_DEPawn::PropRotation(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (AGS_DEController* DEController = Cast<AGS_DEController>(Controller))
	{
		const bool IsActiveRotation = Value.Get<bool>();

		if (IsActiveRotation)
		{
			DEController->GetBuildManager()->RotateProp();
		}
	}
}

void AGS_DEPawn::ClickLMB(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (AGS_DEController* DEController = Cast<AGS_DEController>(Controller))
	{
		const bool IsClickLBT = Value.Get<bool>();

		if (IsClickLBT)
		{
			DEController->GetBuildManager()->PressedLMB();
		}
	}
}

void AGS_DEPawn::ReleasedLMB(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (AGS_DEController* DEController = Cast<AGS_DEController>(Controller))
	{
		const bool IsClickLBT = Value.Get<bool>();

		if (!IsClickLBT)
		{
			DEController->GetBuildManager()->ReleasedLMB();
		}
	}
}

void AGS_DEPawn::ClickRMB(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (AGS_DEController* DEController = Cast<AGS_DEController>(Controller))
	{
		const bool IsClickRBT = Value.Get<bool>();

		if (IsClickRBT)
		{
			DEController->GetBuildManager()->PressedRMB();
		}
	}
}

void AGS_DEPawn::ReleasedRMB(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (AGS_DEController* DEController = Cast<AGS_DEController>(Controller))
	{
		const bool IsClickRBT = Value.Get<bool>();

		if (!IsClickRBT)
		{
			DEController->GetBuildManager()->ReleasedRMB();
		}
	}
}

void AGS_DEPawn::ClickDelete(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (AGS_DEController* DEController = Cast<AGS_DEController>(Controller))
	{
		const bool IsClickDel = Value.Get<bool>();

		if (IsClickDel)
		{
			DEController->GetBuildManager()->PressedDel();
		}
	}
}