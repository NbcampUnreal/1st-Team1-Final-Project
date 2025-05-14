// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RTS/GS_RTSController.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AI/GS_AIController.h"
#include "AI/RTS/GS_RTSCamera.h"
#include "AI/RTS/GS_RTSHUD.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Monster/GS_Monster.h"

AGS_RTSController::AGS_RTSController()
{
	bShowMouseCursor = true;

	KeyboardDir = FVector2D::ZeroVector;
	MouseEdgeDir = FVector2D::ZeroVector;
	CameraSpeed = 2000.f;
	EdgeScreenRatio = 0.05f;
	UnitGroups.SetNum(9);
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
		EnhancedInputComponent->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &AGS_RTSController::CameraMove);
		EnhancedInputComponent->BindAction(CameraMoveAction, ETriggerEvent::Completed, this, &AGS_RTSController::CameraMoveEnd);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AGS_RTSController::OnLeftMousePressed);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnLeftMouseReleased);
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &AGS_RTSController::OnRightMousePressed);
		
		EnhancedInputComponent->BindAction(CtrlAction, ETriggerEvent::Started,   this, &AGS_RTSController::OnCtrlPressed);
		EnhancedInputComponent->BindAction(CtrlAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnCtrlReleased);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started,   this, &AGS_RTSController::OnShiftPressed);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnShiftReleased);
		
		for (int32 i = 0; i < GroupKeyActions.Num(); ++i)
		{
			if (UInputAction* IA = GroupKeyActions[i])
			{
				EnhancedInputComponent->BindAction(GroupKeyActions[i], ETriggerEvent::Started, this, &AGS_RTSController::OnGroupKey, i);
			}
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

void AGS_RTSController::CameraMove(const FInputActionValue& InputValue)
{
	const FVector2D MoveInput = InputValue.Get<FVector2D>();
	KeyboardDir = MoveInput;
}

void AGS_RTSController::CameraMoveEnd()
{
	KeyboardDir = FVector2D::ZeroVector;
}

void AGS_RTSController::OnLeftMousePressed()
{
	// Shift+클릭 → 현재 선택만 변경 
	if (bShiftDown)
	{
		DoShiftClickToggle();
		return;
	}
	
	if (AGS_RTSHUD* HUD = Cast<AGS_RTSHUD>(GetHUD()))
	{
		HUD->StartSelection();
	}

	// 클릭 시 빈 공간이면 기존 선택 해제
	FHitResult Hit;
	bool bHit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), true, Hit);
	if (!bHit)
	{
		ClearUnitSelection();
	}
}

void AGS_RTSController::OnLeftMouseReleased()
{
	if (bShiftDown)
	{
		return;
	}
	
	if (AGS_RTSHUD* HUD = Cast<AGS_RTSHUD>(GetHUD()))
	{
		HUD->StopSelection();
		return;
	}

	// 단순 클릭 처리
	FHitResult Hit;
	bool bHit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit);
	if (bHit && Hit.GetActor())
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(Hit.GetActor()))
		{
			ClearUnitSelection();
			AddUnitToSelection(Monster);
			return;
		}
	}

	// 빈 공간 클릭
	ClearUnitSelection();
}

void AGS_RTSController::OnRightMousePressed(const FInputActionValue& InputValue)
{
	FHitResult GroundHit;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, GroundHit))
	{
		return;
	}

	FVector TargetLocation = GroundHit.Location;
	for (AGS_Monster* Unit : UnitSelection)
	{
		if (AGS_AIController* AIController = Cast<AGS_AIController>(Unit->GetController()))
		{
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->SetValueAsBool(AGS_AIController::bUseRTSKey, true);
				BlackboardComp->SetValueAsVector(AGS_AIController::RTSTargetKey, TargetLocation);
			}
		}
	}
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

void AGS_RTSController::DoShiftClickToggle()
{
	FHitResult ShiftHit;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), true, ShiftHit))
	{
		return;
	}

	if (AGS_Monster* Monster = Cast<AGS_Monster>(ShiftHit.GetActor()))
	{
		if (UnitSelection.Contains(Monster))
		{
			RemoveUnitFromSelection(Monster);
		}
		else
		{
			AddUnitToSelection(Monster);
		}
	}
}

void AGS_RTSController::AddUnitToSelection(AGS_Monster* Unit)
{
	if (!Unit)
	{
		return;
	}
	
	UnitSelection.AddUnique(Unit);
	Unit->SetSelected(true);
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


void AGS_RTSController::OnCtrlPressed(const FInputActionInstance& InputInstance)
{
	bCtrlDown = true;
}

void AGS_RTSController::OnCtrlReleased(const FInputActionInstance& InputInstance)
{
	bCtrlDown = false;
}

void AGS_RTSController::OnShiftPressed(const FInputActionInstance& InputInstance)
{
	bShiftDown = true;
}

void AGS_RTSController::OnShiftReleased(const FInputActionInstance& InputInstance)
{
	bShiftDown = false;
}


void AGS_RTSController::OnGroupKey(const FInputActionInstance& InputInstance, int32 GroupIdx)
{
	if (bCtrlDown) // Ctrl+숫자 → 부대 저장
	{
		UnitGroups[GroupIdx].Units = UnitSelection;
		UE_LOG(LogTemp, Log, TEXT("Saved group %d (%d units)"), GroupIdx+1, UnitSelection.Num());
	}
	else // 숫자만 → 부대 호출
	{
		if (!UnitGroups.IsValidIndex(GroupIdx))
		{
			return;
		}
		
		ClearUnitSelection();
		
		UnitSelection = UnitGroups[GroupIdx].Units;
		for (AGS_Monster* U : UnitSelection)
		{
			U->SetSelected(true);
		}
		UE_LOG(LogTemp, Log, TEXT("Loaded group %d (%d units)"), GroupIdx+1, UnitSelection.Num());
	}
}