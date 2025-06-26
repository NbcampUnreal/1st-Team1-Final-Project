#include "DungeonEditor/GS_DEController.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "DungeonEditor/GS_DEPawn.h"
#include "UI/DungeonEditor/GS_DungeonEditorWidget.h"

AGS_DEController::AGS_DEController()
{
	ZoomSpeed = 300.0f;
	Is_DEActive = true;
}

void AGS_DEController::BeginPlay()
{
	Super::BeginPlay();

	// FindBuildManager In Level
	for (TActorIterator<AGS_BuildManager> It(GetWorld()); It; ++It)
	{
		BuildManagerRef = *It;
		break;
	}
	
	if (Is_DEActive)
	{
		EditorPawn = GetPawn();
		SetShowMouseCursor(true);

		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* SubSystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (nullptr != InputMappingContext)
				{
					SubSystem->AddMappingContext(InputMappingContext, 0);
				}
			}
		}

		// Create Widget
		CreateDEWidgets();
	}
}

void AGS_DEController::CreateDEWidgets()
{
	if (IsLocalController())
	{
		DungeonEditorWidget = CreateWidget<UGS_DungeonEditorWidget>(this, DungeonEditorWidgetClass);
		if (DungeonEditorWidget)
		{
			DungeonEditorWidget->AddToViewport();
		}
	}
}

void AGS_DEController::EnterEditorMode(AActor* SpawnPoint)
{
	if (!IsLocalController()) return;

	if (SpawnPoint)
	{
		Server_RequestEnterEditorMode(SpawnPoint);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnterEditorMode: SpawnPoint is null."));
	}
}

void AGS_DEController::Server_RequestEnterEditorMode_Implementation(AActor* SpawnPoint)
{
	if (!SpawnPoint) return;
    
	if (GetPawn())
	{
		UnPossess();
	}

	EditorPawn = GetWorld()->SpawnActor<AGS_DEPawn>(AGS_DEPawn::StaticClass(), SpawnPoint->GetActorTransform());

	if (EditorPawn)
	{
		Possess(EditorPawn);
        
		Client_OnEnteredEditorMode();
	}
}	

void AGS_DEController::Client_OnEnteredEditorMode_Implementation()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			SubSystem->ClearAllMappings();
			if (nullptr != InputMappingContext)
			{
				SubSystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	SetInputMode(FInputModeGameAndUI());
	//SetupInputComponent();
	
	CreateDEWidgets();
}

void AGS_DEController::ExitEditorMode()
{
	if (!IsLocalController()) return;
	Server_RequestExitEditorMode();
}

void AGS_DEController::Server_RequestExitEditorMode_Implementation()
{
	// 현재 폰이 DEPawn이면 파괴
	if (EditorPawn && EditorPawn->IsA<AGS_DEPawn>())
	{
		EditorPawn->Destroy();
		EditorPawn = nullptr;
	}

	// 빙의 해제
	UnPossess();
    
	// 클라이언트에게 작업 완료 알림
	Client_OnExitedEditorMode();
}

void AGS_DEController::Client_OnExitedEditorMode_Implementation()
{
	// 에디터 UI 제거
	if (DungeonEditorWidget)
	{
		DungeonEditorWidget->RemoveFromParent();
		DungeonEditorWidget = nullptr;
	}

	// 입력 컨텍스트 초기화 (로비로 돌아가서 다시 설정해야 함)
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			SubSystem->ClearAllMappings();
		}
	}

	SetInputMode(FInputModeUIOnly());
}


void AGS_DEController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 모든 입력 액션을 컨트롤러의 핸들러 함수에 바인딩합니다.
		if (MoveAction)
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGS_DEController::HandleMove);
		if (ZoomAction)
			EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AGS_DEController::HandleZoom);
		if (PropRotationAction)
			EnhancedInput->BindAction(PropRotationAction, ETriggerEvent::Started, this, &AGS_DEController::HandlePropRotation);
		if (ClickLMBAction)
		{
			EnhancedInput->BindAction(ClickLMBAction, ETriggerEvent::Triggered, this, &AGS_DEController::HandleClickLMB);
			EnhancedInput->BindAction(ClickLMBAction, ETriggerEvent::Completed, this, &AGS_DEController::HandleReleasedLMB);
		}
		if (ClickRMBAction)
		{
			EnhancedInput->BindAction(ClickRMBAction, ETriggerEvent::Triggered, this, &AGS_DEController::HandleClickRMB);
			EnhancedInput->BindAction(ClickRMBAction, ETriggerEvent::Completed, this, &AGS_DEController::HandleReleasedRMB);
		}
		if (ClickDeleteAction)
			EnhancedInput->BindAction(ClickDeleteAction, ETriggerEvent::Started, this, &AGS_DEController::HandleClickDelete);
	}
}

void AGS_DEController::HandleMove(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->Move(Value);
	}
}

void AGS_DEController::HandleZoom(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->Zoom(Value);
	}
}

void AGS_DEController::HandlePropRotation(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->PropRotation(Value);
	}
}

void AGS_DEController::HandleClickLMB(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->ClickLMB(Value);
	}
}

void AGS_DEController::HandleReleasedLMB(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->ReleasedLMB(Value);
	}
}

void AGS_DEController::HandleClickRMB(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->ClickRMB(Value);
	}
}

void AGS_DEController::HandleReleasedRMB(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->ReleasedRMB(Value);
	}
}

void AGS_DEController::HandleClickDelete(const FInputActionValue& Value)
{
	if (AGS_DEPawn* CurrentPawn = GetPawn<AGS_DEPawn>())
	{
		CurrentPawn->ClickDelete(Value);
	}
}
