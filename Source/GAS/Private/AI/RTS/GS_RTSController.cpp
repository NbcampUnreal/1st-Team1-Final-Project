// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RTS/GS_RTSController.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AI/RTS/GS_RTSCamera.h"
#include "Character/Player/Monster/GS_Monster.h"

AGS_RTSController::AGS_RTSController()
{
	bShowMouseCursor = true;
}

void AGS_RTSController::BeginPlay()
{
	Super::BeginPlay();
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	InitCameraActor();
}

void AGS_RTSController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	{
		if (CameraMoveAction)
		{
			EnhancedInputComponent->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &AGS_RTSController::CameraMove);
			EnhancedInputComponent->BindAction(CameraMoveAction, ETriggerEvent::Completed, this, &AGS_RTSController::CameraMoveEnd);
		}
	}
}

void AGS_RTSController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 마우스 엣지 감지
	MouseEdgeDir = GetMouseEdgeDirection();
	
	FVector2D FinalDir = GetFinalDirection();
	if (!FinalDir.IsNearlyZero())
	{
		MoveCamera(FinalDir, DeltaTime);
	}
}

void AGS_RTSController::CameraMove(const FInputActionValue& value)
{
	const FVector2D MoveInput = value.Get<FVector2D>();
	KeyboardDir = MoveInput;
}

void AGS_RTSController::CameraMoveEnd()
{
	KeyboardDir = FVector2D::ZeroVector;
}

FVector2D AGS_RTSController::GetKeyboardDirection() const
{
	if (FMath::Abs(KeyboardDir.X) > 0.1f || FMath::Abs(KeyboardDir.Y) > 0.1f)
	{
		return KeyboardDir;
	}
	return FVector2D::ZeroVector;
}

FVector2D AGS_RTSController::GetMouseEdgeDirection() const
{
	float MouseX, MouseY;
	if (!GetMousePosition(MouseX, MouseY))
	{
		return FVector2D::ZeroVector;
	}

	int32 ViewportX, ViewportY;
	GetViewportSize(ViewportX, ViewportY);

	float EdgeW = ViewportX * EdgeScreenRatio;
	float EdgeH = ViewportY * EdgeScreenRatio;

	FVector2D Dir = FVector2D::ZeroVector;
	if (MouseX <= EdgeW) // 좌·우 엣지 판정
	{
		Dir.X = -1.f;
	}
	else if (MouseX >= ViewportX - EdgeW)
	{
		Dir.X = 1.f;
	}
	if (MouseY <= EdgeH) // 위·아래 엣지 판정 
	{
		Dir.Y = 1.f;
	}
	else if (MouseY >= ViewportY - EdgeH)
	{
		Dir.Y = -1.f;
	}

	return Dir;
}

// 키보드 입력이 1순위로 
FVector2D AGS_RTSController::GetFinalDirection() const
{
	FVector2D Dir = GetKeyboardDirection();
	return !Dir.IsNearlyZero() ? Dir : MouseEdgeDir;
}

void AGS_RTSController::MoveCamera(const FVector2D& Direction, float DeltaTime)
{
	FVector2D NormDir = Direction.GetSafeNormal();
	FVector Delta = FVector(NormDir.Y, NormDir.X, 0.f) * CameraSpeed * DeltaTime;
	CameraActor->AddActorWorldOffset(Delta, true);
}

void AGS_RTSController::InitCameraActor()
{
	for (TActorIterator<AGS_RTSCamera> It(GetWorld()); It; ++It)
	{
		CameraActor = *It;
		break;
	}

	if (!CameraActor)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		CameraActor = GetWorld()->SpawnActor<AGS_RTSCamera>(AGS_RTSCamera::StaticClass(), Params);
	}

	// 뷰 타깃으로 설정
	if (CameraActor && GetViewTarget() != CameraActor) 
	{
		SetViewTarget(CameraActor);
	}
}

void AGS_RTSController::AddUnitToSelection(AGS_Monster* Unit)
{
	if (!Unit)
	{
		return;
	}
	
	if (UnitSelection.AddUnique(Unit) != INDEX_NONE)
	{
		Unit->SetSelected(true);
	}
}

void AGS_RTSController::RemoveUnitFromSelection(AGS_Monster* Unit)
{
	if (!Unit)
	{
		return;
	}
	
	UnitSelection.Remove(Unit);
	Unit->SetSelected(false);
}

void AGS_RTSController::ClearUnitSelection()
{
	for (AGS_Monster* Unit : UnitSelection)
	{
		if (IsValid(Unit))
		{
			Unit->SetSelected(false);
		}
	}
		
	UnitSelection.Empty();
}
