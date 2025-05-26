// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RTS/GS_RTSController.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AI/GS_AIController.h"
#include "AI/RTS/GS_RTSCamera.h"
#include "AI/RTS/GS_RTSHUD.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "AkGameplayStatics.h"
#include "Character/Player/Monster/GS_Monster.h"


AGS_RTSController::AGS_RTSController()
{
	bShowMouseCursor = true;

	CurrentCommand = ERTSCommand::None;
	KeyboardDir = FVector2D::ZeroVector;
	MouseEdgeDir = FVector2D::ZeroVector;
	CameraActor = nullptr;
	CameraSpeed = 2000.f;
	EdgeScreenRatio = 0.02f;
	UnitGroups.SetNum(9);
	bCtrlDown = false;
	bShiftDown = false;
}

void AGS_RTSController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
	{
		ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
	}
	if (!HasAuthority() && IsLocalController())
	{
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

		if (RTSWidgetClass)
		{
			UUserWidget* RTSWidget = CreateWidget<UUserWidget>(this, RTSWidgetClass);
			if (RTSWidget)
			{
				RTSWidget->AddToViewport();
			}
		}
	}
}

void AGS_RTSController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	{
		EnhancedInputComponent->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &AGS_RTSController::CameraMove);
		EnhancedInputComponent->BindAction(CameraMoveAction, ETriggerEvent::Completed, this, &AGS_RTSController::CameraMoveEnd);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &AGS_RTSController::OnCommandMove);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AGS_RTSController::OnCommandAttack);
		EnhancedInputComponent->BindAction(StopAction, ETriggerEvent::Started, this, &AGS_RTSController::OnCommandStop);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &AGS_RTSController::OnCommandSkill);
		
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AGS_RTSController::OnLeftMousePressed);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnLeftMouseReleased);
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &AGS_RTSController::OnRightMousePressed);
		
		EnhancedInputComponent->BindAction(CtrlAction, ETriggerEvent::Started,   this, &AGS_RTSController::OnCtrlPressed);
		EnhancedInputComponent->BindAction(CtrlAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnCtrlReleased);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started,   this, &AGS_RTSController::OnShiftPressed);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnShiftReleased);
		
		for (int32 i = 0; i < GroupKeyActions.Num(); ++i)
		{
			if (GroupKeyActions[i])
			{
				EnhancedInputComponent->BindAction(GroupKeyActions[i], ETriggerEvent::Started, this, &AGS_RTSController::OnGroupKey, i);
			}
		}

		for (int32 i = 0; i < CameraKeyActions.Num(); ++i)
		{
			if (CameraKeyActions[i])
			{
				EnhancedInputComponent->BindAction(CameraKeyActions[i], ETriggerEvent::Started, this, &AGS_RTSController::OnCameraKey, i);
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


void AGS_RTSController::OnCommandMove(const FInputActionValue& Value)
{
	MoveSelectedUnits();
}

void AGS_RTSController::MoveSelectedUnits()
{
	CurrentCommand = ERTSCommand::Move;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
}

void AGS_RTSController::OnCommandAttack(const FInputActionValue& Value)
{
	AttackSelectedUnits();
}

void AGS_RTSController::AttackSelectedUnits()
{
	CurrentCommand = ERTSCommand::Attack;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
}

void AGS_RTSController::OnCommandStop(const FInputActionValue& Value)
{
	StopSelectedUnits();
}

void AGS_RTSController::StopSelectedUnits()
{
	CurrentCommand = ERTSCommand::Stop;
	TArray<AGS_Monster*> Commandables;
	GatherCommandableUnits(Commandables);

	Server_RTSStop(Commandables);
}

void AGS_RTSController::OnCommandSkill(const FInputActionValue& Value)
{
	SkillSelectedUnits();
}

void AGS_RTSController::SkillSelectedUnits()
{
	CurrentCommand = ERTSCommand::Skill;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
}


void AGS_RTSController::OnLeftMousePressed()
{
	// Ctrl+클릭 → 같은 유닛 타입 전체 선택
	if (bCtrlDown && !bShiftDown)
	{
		SelectOnCtrlClick();
		return;
	}
	
	// Shift+클릭 → 현재 선택만 변경 
	if (bShiftDown)
	{
		ToggleOnShiftClick();
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("--- OnLeftMousePressed: Command=%d"), static_cast<int32>(CurrentCommand));
	
	FHitResult Hit;
	bool bHit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), true, Hit);
	UE_LOG(LogTemp, Log, TEXT("  bHit=%d, Hit.Actor=%s"), bHit, *GetNameSafe(Hit.GetActor()));
	
	TArray<AGS_Monster*> Units;
	GatherCommandableUnits(Units);
	UnlockTargets(Units);

	// 명령 모드에 따라 
	switch (CurrentCommand)
	{
	case ERTSCommand::Move:
		if (bHit)
		{
			Server_RTSMove(Units, Hit.Location);
		}
		break;
	case ERTSCommand::Attack:
		if (bHit)
		{
			if (AGS_Character* Target = Cast<AGS_Character>(Hit.GetActor()))
			{
				Server_RTSAttack(Units, Target);
			}
			else
			{
				Server_RTSAttackMove(Units, Hit.Location);
			}
		}
		break;
	case ERTSCommand::Skill:
		if (bHit)
		{
			Server_RTSSkill(Units, Hit.Location);
		}
		break;
	default:
		if (AGS_RTSHUD* HUD = Cast<AGS_RTSHUD>(GetHUD()))
		{
			HUD->StartSelection();
		}
		break;
	}
	
	CurrentCommand = ERTSCommand::None;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
}

void AGS_RTSController::OnLeftMouseReleased()
{
	if (bShiftDown || bCtrlDown)
	{
		return;
	}
	
	if (AGS_RTSHUD* HUD = Cast<AGS_RTSHUD>(GetHUD()))
	{
		HUD->StopSelection();
	}
}

void AGS_RTSController::OnRightMousePressed(const FInputActionValue& InputValue)
{
	// 커맨드 모드 중이면 취소
	if (CurrentCommand != ERTSCommand::None)
	{
		CurrentCommand = ERTSCommand::None;
		OnRTSCommandChanged.Broadcast(CurrentCommand);
		return;
	}
	
	FHitResult GroundHit;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), true, GroundHit))
	{
		return;
	}

	// 명령 가능한 유닛들만 
	TArray<AGS_Monster*> Units;
	GatherCommandableUnits(Units);
	UnlockTargets(Units);
	Server_RTSMove(Units, GroundHit.Location);
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
	if (CameraActor)
	{
		CameraActor->AddActorWorldOffset(Delta, true);
	}
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


void AGS_RTSController::SelectOnCtrlClick()
{
	FHitResult Hit;		
	bool bHit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), true, Hit);
	if (bHit && Hit.GetActor())
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(Hit.GetActor()))
		{
			if (!IsSelectable(Monster))
			{
				return;
			}
				
			ClearUnitSelection();
				
			ECharacterType MonsterType = Monster->GetCharacterType();
			TArray<AGS_Monster*> SameTypeUnits;
				
			// 월드에 있는 모든 몬스터를 순회 
			for (TActorIterator<AGS_Monster> It(GetWorld()); It; ++It)
			{
				AGS_Monster* M = *It;
				if (M->GetCharacterType() != MonsterType)
				{
					//continue;
					SameTypeUnits.Add(M);
				}

				//AddUnitToSelection(M);
				// 한 번에 선택하여 첫 번째 유닛만 소리 재생
				AddMultipleUnitsToSelection(SameTypeUnits);
			}
		}
	}
}

void AGS_RTSController::ToggleOnShiftClick()
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

	// 첫 번째로 추가되는 유닛만 소리 재생
	bool bShouldPlaySound = UnitSelection.IsEmpty();
	
	UnitSelection.AddUnique(Unit);
	OnSelectionChanged.Broadcast(UnitSelection);
	//Unit->SetSelected(true);
	Unit->SetSelected(true, bShouldPlaySound);
}

// 여러 유닛을 한번에 선택할 때 사용할 새로운 함수 추가
void AGS_RTSController::AddMultipleUnitsToSelection(const TArray<AGS_Monster*>& Units)
{
	if (Units.IsEmpty())
	{
		return;
	}
	
	bool bShouldPlaySound = UnitSelection.IsEmpty();
	
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		AGS_Monster* Unit = Units[i];
		if (!Unit)
		{
			continue;
		}
		
		UnitSelection.AddUnique(Unit);
		// 첫 번째 유닛만 소리 재생
		Unit->SetSelected(true, bShouldPlaySound && i == 0);
	}
	
	OnSelectionChanged.Broadcast(UnitSelection);
}

void AGS_RTSController::RemoveUnitFromSelection(AGS_Monster* Unit)
{
	if (!Unit)
	{
		return;
	}
	
	UnitSelection.Remove(Unit);
	OnSelectionChanged.Broadcast(UnitSelection);
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
	OnSelectionChanged.Broadcast(UnitSelection);
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


// 유닛 그룹 저장 + 불러오기 
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
		//UnitSelection = UnitGroups[GroupIdx].Units;
		//for (AGS_Monster* U : UnitSelection)
		//{
		//	U->SetSelected(true);
		//}

		// 부대 호출 시에도 첫 번째 유닛만 소리 재생
		AddMultipleUnitsToSelection(UnitGroups[GroupIdx].Units);

		UE_LOG(LogTemp, Log, TEXT("Loaded group %d (%d units)"), GroupIdx+1, UnitSelection.Num());
	}
}

// 카메라 위치 저장 + 불러오기 
void AGS_RTSController::OnCameraKey(const FInputActionInstance& InputInstance, int32 CameraIndex)
{
	if (bShiftDown) // 저장
	{
		if (CameraActor)
		{
			SavedCameraPositions.Add(CameraIndex, CameraActor->GetActorLocation());
			UE_LOG(LogTemp, Log, TEXT("Saved camera pos %d: %s"), CameraIndex, *CameraActor->GetActorLocation().ToString());
		}
	}
	else // 로드
	{
		if (CameraActor && SavedCameraPositions.Contains(CameraIndex))
		{
			CameraActor->SetActorLocation(SavedCameraPositions[CameraIndex]);
			UE_LOG(LogTemp, Log, TEXT("Moved camera to saved pos %d: %s"), CameraIndex, *SavedCameraPositions[CameraIndex].ToString());
		}
	}
}


void AGS_RTSController::MoveAIViaMinimap(const FVector& WorldLocation)
{
	// 명령 가능한 유닛들만 
	TArray<AGS_Monster*> Commandables;
	GatherCommandableUnits(Commandables);
	
	Server_RTSMove(Commandables, WorldLocation);
}

void AGS_RTSController::MoveCameraViaMinimap(const FVector& WorldLocation)
{
	if (CameraActor)
	{
		FVector NewLocation = FVector(WorldLocation.X, WorldLocation.Y, CameraActor->GetActorLocation().Z);
		CameraActor->SetActorLocation(NewLocation);
	}
}


void AGS_RTSController::Server_RTSMove_Implementation(const TArray<AGS_Monster*>& Units, const FVector& Dest)
{
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		AGS_Monster* Unit = Units[i];
		if (!IsValid(Unit)) continue;
		
		if (AGS_AIController* AIController = Cast<AGS_AIController>(Unit->GetController()))
		{
			AIController->UnlockTarget();
			
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Move));
				BlackboardComp->SetValueAsVector (AGS_AIController::MoveLocationKey, Dest);
				BlackboardComp->ClearValue(AGS_AIController::TargetActorKey);

				// 첫 번째 유닛만 이동 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}

void AGS_RTSController::Server_RTSAttackMove_Implementation(const TArray<AGS_Monster*>& Units, const FVector& Dest)
{
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		AGS_Monster* Unit = Units[i];
		if (!IsValid(Unit)) continue;
		
		if (AGS_AIController* AIController = Cast<AGS_AIController>(Unit->GetController()))
		{
			AIController->UnlockTarget();
			
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Attack));
				BlackboardComp->SetValueAsVector (AGS_AIController::MoveLocationKey, Dest);
				BlackboardComp->ClearValue(AGS_AIController::TargetActorKey);

				// 첫 번째 유닛만 공격 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}

void AGS_RTSController::Server_RTSAttack_Implementation(const TArray<AGS_Monster*>& Units, AGS_Character* TargetActor)
{
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		AGS_Monster* Unit = Units[i];
		if (!IsValid(Unit)) continue;
		
		if (AGS_AIController* AIController = Cast<AGS_AIController>(Unit->GetController()))
		{
			AIController->LockTarget(TargetActor);
			
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Attack));
				BlackboardComp->SetValueAsObject(AGS_AIController::TargetActorKey, TargetActor);
				BlackboardComp->ClearValue(AGS_AIController::MoveLocationKey);

				// 첫 번째 유닛만 공격 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}

void AGS_RTSController::Server_RTSStop_Implementation(const TArray<AGS_Monster*>& Units)
{
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		AGS_Monster* Unit = Units[i];
		if (!IsValid(Unit)) continue;
		
		if (AGS_AIController* AIController = Cast<AGS_AIController>(Unit->GetController()))
		{
			AIController->UnlockTarget();
			AIController->StopMovement();
			
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Stop));
				BlackboardComp->ClearValue(AGS_AIController::MoveLocationKey);
				BlackboardComp->ClearValue(AGS_AIController::TargetActorKey);

				// 첫 번째 유닛만 정지 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}

void AGS_RTSController::Server_RTSSkill_Implementation(const TArray<AGS_Monster*>& Units, const FVector& TargetLoc)
{
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		AGS_Monster* Unit = Units[i];
		if (!IsValid(Unit)) continue;
		
		if (AGS_AIController* AIController = Cast<AGS_AIController>(Unit->GetController()))
		{
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Skill));
				BlackboardComp->SetValueAsVector(AGS_AIController::MoveLocationKey, TargetLoc);
				BlackboardComp->ClearValue(AGS_AIController::TargetActorKey);

				// 첫 번째 유닛만 스킬 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}


void AGS_RTSController::GatherCommandableUnits(TArray<AGS_Monster*>& Out) const
{
	for (AGS_Monster* Unit : UnitSelection)
	{
		if (IsValid(Unit) && Unit->IsCommandable())
		{
			Out.Add(Unit);
		}
	}
}

bool AGS_RTSController::IsSelectable(AGS_Monster* Monster) const
{
	return IsValid(Monster) && Monster->IsSelectable();
}

void AGS_RTSController::UnlockTargets(const TArray<AGS_Monster*>& Units)
{
	for (AGS_Monster* Unit : Units)
	{
		if (!IsValid(Unit)) 
			continue;

		if (AGS_AIController* AI = Cast<AGS_AIController>(Unit->GetController()))
		{
			AI->UnlockTarget();
		}
	}
}