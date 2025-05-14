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

void AGS_DEPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput
		= Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AGS_DEController* PlayerController
			= Cast<AGS_DEController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction
					, ETriggerEvent::Triggered
					,this
					, &AGS_DEPawn::Move);
			}

			if (PlayerController->ZoomAction)
			{
				EnhancedInput->BindAction(PlayerController->ZoomAction
					, ETriggerEvent::Triggered
					,this
					, &AGS_DEPawn::Zoom);
			}
		}
	}
}

void AGS_DEPawn::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2d MoveInput = Value.Get<FVector2d>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
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

