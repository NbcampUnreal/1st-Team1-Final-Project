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
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Skill/Monster/GS_MonsterSkillBase.h"
#include "Character/Skill/Monster/GS_MonsterSkillComp.h"
#include "UI/Character/GS_HPTextWidgetComp.h"
#include "Sound/GS_AudioManager.h"


AGS_RTSController::AGS_RTSController()
{
	bShowMouseCursor = true;

	CurrentCommand = ERTSCommand::None;
	KeyboardDir = FVector2D::ZeroVector;
	MouseEdgeDir = FVector2D::ZeroVector;
	CameraActor = nullptr;
	CameraSpeed = 2000.f;
	EdgeScreenRatio = 0.01f;
	UnitGroups.SetNum(9);
	bCtrlDown = false;
	bShiftDown = false;
	MaxSelectableUnits = 12;
}

void AGS_RTSController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
	{
		ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
		ViewportClient->SetHideCursorDuringCapture(false);
		ViewportClient->SetMouseLockMode(EMouseLockMode::LockAlways);
	}
	SetRTSCursor(DefaultCursorTexture);
	bEnableMouseOverEvents = true; 
	
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
	
	for (AGS_Seeker* Seeker : TActorRange<AGS_Seeker>(GetWorld()))
	{
		if (IsValid(Seeker))
		{
			Seeker->HPTextWidgetComp->SetVisibility(true);
		}
	}

	// RTS 모드에서 BGM 시작 (로컬 플레이어에게만)
	if (IsLocalController())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UGS_AudioManager* AudioManager = GameInstance->GetSubsystem<UGS_AudioManager>())
			{
				AudioManager->StartMapBGM(this);
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
		EnhancedInputComponent->BindAction(HoldAction, ETriggerEvent::Started, this, &AGS_RTSController::OnCommandHold);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &AGS_RTSController::OnCommandSkill);
		
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AGS_RTSController::OnLeftMousePressed);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnLeftMouseReleased);
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &AGS_RTSController::OnRightMousePressed);
		
		EnhancedInputComponent->BindAction(CtrlAction, ETriggerEvent::Started,   this, &AGS_RTSController::OnCtrlPressed);
		EnhancedInputComponent->BindAction(CtrlAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnCtrlReleased);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started,   this, &AGS_RTSController::OnShiftPressed);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AGS_RTSController::OnShiftReleased);
		EnhancedInputComponent->BindAction(DoubleClickAction, ETriggerEvent::Completed,   this, &AGS_RTSController::SelectOnCtrlClick);
		
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
	
	UpdateCursorForEdgeScroll();
	
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
		if (CommandButtonSound)
	{
		UAkGameplayStatics::PostEvent(CommandButtonSound, this, 0, FOnAkPostEventCallback());
	}
}

void AGS_RTSController::MoveSelectedUnits()
{
	CurrentCommand = ERTSCommand::Move;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
}


void AGS_RTSController::OnCommandAttack(const FInputActionValue& Value)
{
	AttackSelectedUnits();
	if (CommandButtonSound)
	{
		UAkGameplayStatics::PostEvent(CommandButtonSound, this, 0, FOnAkPostEventCallback());
	}
}

void AGS_RTSController::AttackSelectedUnits()
{
	CurrentCommand = ERTSCommand::Attack;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
}


void AGS_RTSController::OnCommandStop(const FInputActionValue& Value)
{
	StopSelectedUnits();
	if (CommandButtonSound)
	{
		UAkGameplayStatics::PostEvent(CommandButtonSound, this, 0, FOnAkPostEventCallback());
	}
}

void AGS_RTSController::StopSelectedUnits()
{
	CurrentCommand = ERTSCommand::Stop;
	
	TArray<AGS_Monster*> Commandables;
	GatherCommandableUnits(Commandables);
	Server_RTSStop(Commandables);
}


void AGS_RTSController::OnCommandHold(const FInputActionValue& Value)
{
	HoldSelectedUnits();
	if (CommandButtonSound)
	{
		UAkGameplayStatics::PostEvent(CommandButtonSound, this, 0, FOnAkPostEventCallback());
	}
}

void AGS_RTSController::HoldSelectedUnits()
{
	CurrentCommand = ERTSCommand::Hold;
	
	TArray<AGS_Monster*> Commandables;
	GatherCommandableUnits(Commandables);
	Server_RTSHold(Commandables);
}


void AGS_RTSController::OnCommandSkill(const FInputActionValue& Value)
{
	SkillSelectedUnits();
	if (CommandButtonSound)
	{
		UAkGameplayStatics::PostEvent(CommandButtonSound, this, 0, FOnAkPostEventCallback());
	}
}

void AGS_RTSController::SkillSelectedUnits()
{
	CurrentCommand = ERTSCommand::Skill;
	OnRTSCommandChanged.Broadcast(CurrentCommand);

	TArray<AGS_Monster*> Commandables;
	GatherCommandableUnits(Commandables);
	Server_RTSSkill(Commandables);
}


void AGS_RTSController::OnLeftMousePressed()
{
	if (bCtrlDown && !bShiftDown)
	{
		SelectOnCtrlClick();
		return;
	}
	
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
	Server_RTSMove(Units, GroundHit.Location);
}

void AGS_RTSController::OnEscapeButtonClicked()
{
	if (CurrentCommand != ERTSCommand::None)
	{
		if (CommandCancelSound)
		{
			UAkGameplayStatics::PostEvent(CommandCancelSound, this, 0, FOnAkPostEventCallback());
		}
		CurrentCommand = ERTSCommand::None;
		OnRTSCommandChanged.Broadcast(CurrentCommand);
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

void AGS_RTSController::SetRTSCursor(UTexture2D* CursorTexture)
{
	if (!IsValid(CursorTexture))
	{
		if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
		{
			ViewportClient->SetHardwareCursor(EMouseCursor::Default, NAME_None, FIntPoint::ZeroValue);
		}
		return;
	}
	
	if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
	{
		FString CursorPath = CursorTexture->GetPathName();
		CursorPath = CursorPath.Replace(TEXT("/Game/"), TEXT(""));
		
		int32 LastDotIndex;
		if (CursorPath.FindLastChar('.', LastDotIndex))
		{
			CursorPath = CursorPath.Left(LastDotIndex);
		}
		
		ViewportClient->SetHardwareCursor(EMouseCursor::Default, FName(*CursorPath), FIntPoint::ZeroValue);
	}
}

void AGS_RTSController::UpdateCursorForEdgeScroll()
{
	bool bShouldShowEdgeCursor = !MouseEdgeDir.IsNearlyZero();
    
	if (bShouldShowEdgeCursor)
	{
		SetRTSCursor(ScrollCursorTexture);
	}
	else
	{
		UpdateCursorForCommand();
	}
}

void AGS_RTSController::UpdateCursorForCommand()
{
	switch (CurrentCommand)
	{
	case ERTSCommand::Move:
		SetRTSCursor(CommandCursorTexture);
		break;
	case ERTSCommand::Attack:
		SetRTSCursor(CommandCursorTexture);
		break;
	default:
		SetRTSCursor(DefaultCursorTexture);
		break;
	}
}

void AGS_RTSController::SelectOnCtrlClick()
{	
	int32 ViewportX, ViewportY;
	GetViewportSize(ViewportX, ViewportY);
	
	FHitResult Hit;
	bool bHit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), true, Hit);
	if (!bHit || !Hit.GetActor())
	{
		return;
	}

	AGS_Monster* Monster = Cast<AGS_Monster>(Hit.GetActor());
	if (!Monster || !IsSelectable(Monster))
	{
		return;
	}
	
	ECharacterType MonsterType = Monster->GetCharacterType();
	TArray<AGS_Monster*> SameTypeUnits;
	
	// 월드에 있는 모든 몬스터를 순회 
	for (TActorIterator<AGS_Monster> It(GetWorld()); It; ++It)
	{
		AGS_Monster* CurrentMonster = *It;
		if (CurrentMonster->GetCharacterType() != MonsterType)
		{
			continue;
		}

		// 월드 좌표를 스크린 좌표로 투영
		FVector WorldLoc = CurrentMonster->GetActorLocation();
		FVector2D ScreenPos;
		bool bProjected = ProjectWorldLocationToScreen(WorldLoc, ScreenPos, true);

		// HUD 제외 카메라 뷰에서만 보이는 몬스터만 선택되도록 
		if (bProjected && ScreenPos.X >= 0.0f && ScreenPos.X <= ViewportX && ScreenPos.Y >= 0.0f && ScreenPos.Y <= ViewportY*0.77)
		{
			SameTypeUnits.Add(CurrentMonster);
		}
	}

	// 유닛으로부터의 거리를 기준으로 정렬
	SameTypeUnits.Sort([Monster](const AGS_Monster& A, const AGS_Monster& B)
	{
		return Monster->GetDistanceTo(&A) < Monster->GetDistanceTo(&B);
	});

	// 클릭된 유닛 포함하여 가까이에 있는 12개만 선택되도록 
	TArray<AGS_Monster*> Selection;
	for (int32 i = 0; i < SameTypeUnits.Num() && Selection.Num() < MaxSelectableUnits; ++i)
	{
		AGS_Monster* UnitToAdd = SameTypeUnits[i];
		if (!Selection.Contains(UnitToAdd)) 
		{
			Selection.Add(UnitToAdd);
		}
	}

	// 한 번에 선택하여 첫 번째 유닛만 소리 재생
	AddMultipleUnitsToSelection(Selection);
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
		if (!IsSelectable(Monster))
		{
			return;
		}
		
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
	if (!Unit || !IsSelectable(Unit))
	{
		return;
	}

	if (UnitSelection.Num() >= MaxSelectableUnits)
	{
		return;
	}

	// 첫 번째로 추가되는 유닛만 소리 재생
	bool bShouldPlaySound = UnitSelection.IsEmpty();
	
	Unit->OnMonsterDead.AddUniqueDynamic(this, &AGS_RTSController::OnSelectedUnitDead);
	
	UnitSelection.AddUnique(Unit);
	OnSelectionChanged.Broadcast(UnitSelection);
	OnSelectedUnitsSkillChanged.Broadcast(HasAnySelectedUnitSkill()); 
	Unit->SetSelected(true, bShouldPlaySound);
}

// 여러 유닛을 한번에 선택할 때 사용할 새로운 함수 추가
void AGS_RTSController::AddMultipleUnitsToSelection(const TArray<AGS_Monster*>& Units)
{
	if (Units.IsEmpty())
	{
		return;
	}

	ClearUnitSelection();

	int32 AddedCount = 0;
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		if (AddedCount >= MaxSelectableUnits)
		{
			break;
		}
		
		AGS_Monster* Unit = Units[i];
		if (!Unit)
		{
			continue;
		}

		if (!IsSelectable(Unit))
		{
			continue;
		}

		Unit->OnMonsterDead.AddUniqueDynamic(this, &AGS_RTSController::OnSelectedUnitDead);
		
		UnitSelection.AddUnique(Unit);
		// 첫 번째 유닛만 소리 재생
		Unit->SetSelected(true, i == 0);
		AddedCount++;
	}
	
	OnSelectionChanged.Broadcast(UnitSelection);
	OnSelectedUnitsSkillChanged.Broadcast(HasAnySelectedUnitSkill()); 
}

void AGS_RTSController::SelectSameTypeFromSelection(AGS_Monster* Unit)
{
	if (!Unit)
	{
		return;
	}

	if (!UnitSelection.Contains(Unit))
	{
		return;
	}
	
	ECharacterType MonsterType = Unit->GetCharacterType();
	TArray<AGS_Monster*> SameTypeUnits;
	for (AGS_Monster* Monster : UnitSelection)
	{
		if (Monster->GetCharacterType() != MonsterType)
		{
			continue;
		}

		SameTypeUnits.Add(Monster);
	}
	
	AddMultipleUnitsToSelection(SameTypeUnits);
}

void AGS_RTSController::RemoveUnitFromSelection(AGS_Monster* Unit)
{
	if (!Unit)
	{
		return;
	}
	
	Unit->OnMonsterDead.RemoveDynamic(this, &AGS_RTSController::OnSelectedUnitDead);
	
	UnitSelection.Remove(Unit);
	OnSelectionChanged.Broadcast(UnitSelection);
	OnSelectedUnitsSkillChanged.Broadcast(HasAnySelectedUnitSkill()); 
	Unit->SetSelected(false);
}

void AGS_RTSController::ClearUnitSelection()
{
	for (AGS_Monster* Unit : UnitSelection)
	{
		if (IsValid(Unit))
		{
			Unit->OnMonsterDead.RemoveDynamic(this, &AGS_RTSController::OnSelectedUnitDead);
			
			Unit->SetSelected(false);
		}
	}
	
	UnitSelection.Empty();
	OnSelectionChanged.Broadcast(UnitSelection);
	OnSelectedUnitsSkillChanged.Broadcast(HasAnySelectedUnitSkill()); 
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
	if (bCtrlDown) // 부대 저장
	{
		UnitGroups[GroupIdx].Units = UnitSelection;
	}
	else // 부대 호출
	{
		if (!UnitGroups.IsValidIndex(GroupIdx))
		{
			return;
		}

		// 부대 호출 시에도 첫 번째 유닛만 소리 재생
		AddMultipleUnitsToSelection(UnitGroups[GroupIdx].Units);
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
		}
	}
	else // 로드
	{
		if (CameraActor && SavedCameraPositions.Contains(CameraIndex))
		{
			CameraActor->SetActorLocation(SavedCameraPositions[CameraIndex]);
		}
	}
}

void AGS_RTSController::MoveAIViaMinimap(const FVector& WorldLocation)
{
	TArray<AGS_Monster*> Commandables;
	GatherCommandableUnits(Commandables);
	
	Server_RTSMove(Commandables, WorldLocation);

	CurrentCommand = ERTSCommand::None;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
}

void AGS_RTSController::AttackAIViaMinimap(const FVector& WorldLocation)
{
	TArray<AGS_Monster*> Commandables;
	GatherCommandableUnits(Commandables);
	
	Server_RTSAttackMove(Commandables, WorldLocation);

	CurrentCommand = ERTSCommand::None;
	OnRTSCommandChanged.Broadcast(CurrentCommand);
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
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Move));
				BlackboardComp->SetValueAsVector (AGS_AIController::MoveLocationKey, Dest);
				BlackboardComp->ClearValue(AGS_AIController::TargetActorKey);
				BlackboardComp->SetValueAsBool(AGS_AIController::TargetLockedKey, false);

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
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Attack));
				BlackboardComp->SetValueAsVector (AGS_AIController::MoveLocationKey, Dest);
				BlackboardComp->SetValueAsBool(AGS_AIController::TargetLockedKey, false);

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
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Attack));
				BlackboardComp->SetValueAsObject(AGS_AIController::TargetActorKey, TargetActor);
				BlackboardComp->ClearValue(AGS_AIController::MoveLocationKey);
				BlackboardComp->SetValueAsBool(AGS_AIController::TargetLockedKey, true);

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
			AIController->StopMovement();
			
			if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
			{
				BlackboardComp->ClearValue(AGS_AIController::CommandKey);
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::None));
				BlackboardComp->SetValueAsBool(AGS_AIController::TargetLockedKey, false);

				// 첫 번째 유닛만 정지 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}

void AGS_RTSController::Server_RTSHold_Implementation(const TArray<AGS_Monster*>& Units)
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
				BlackboardComp->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::Hold));
				BlackboardComp->SetValueAsBool(AGS_AIController::TargetLockedKey, false);

				// 첫 번째 유닛만 정지 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}

void AGS_RTSController::Server_RTSSkill_Implementation(const TArray<AGS_Monster*>& Units)
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

				// 첫 번째 유닛만 스킬 소리 재생
				if (i == 0 && Unit->MoveSoundEvent)
				{
					UAkGameplayStatics::PostEvent(Unit->MoveSoundEvent, Unit, 0, FOnAkPostEventCallback());
				}
			}
		}
	}
}


bool AGS_RTSController::HasAnySelectedUnitSkill() const
{
	for (AGS_Monster* SelectedMonster : UnitSelection)
	{
		UGS_MonsterSkillComp* SkillComp = SelectedMonster->GetMonsterSkillComp();
		if (SkillComp && SkillComp->MonsterSkill)
		{
			return true; 
		}
	}
	return false; 
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
	if (!IsValid(Monster))
	{
		return false;
	}
	
	if (UGS_StatComp* Stat = Monster->GetStatComp())
	{
		if (Stat->GetCurrentHealth() <= 0.f)
		{
			return false;  
		}
	}
	
	return Monster->IsSelectable();
}

void AGS_RTSController::OnSelectedUnitDead(AGS_Monster* Monster)
{
	RemoveUnitFromSelection(Monster);
}
