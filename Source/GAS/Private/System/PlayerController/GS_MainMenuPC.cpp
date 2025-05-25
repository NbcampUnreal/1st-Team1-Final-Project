#include "System/PlayerController/GS_MainMenuPC.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h" //slot이 붙는 게 맞나?
#include "Kismet/GameplayStatics.h"
#include "System/GS_GameInstance.h"

AGS_MainMenuPC::AGS_MainMenuPC()
	: MainMenuWidgetInstance(nullptr)
{
}

void AGS_MainMenuPC::HandleCustomGameRequest()
{
	UE_LOG(LogTemp, Warning, TEXT("AGS_MainMenuPC: HandleCustomGameRequest() CALLED for %s."), *GetNameSafe(this));
	UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>();
	if (GI)
	{
		UE_LOG(LogTemp, Log, TEXT("AGS_MainMenuPC: Player %s is requesting a custom game session via GameInstance."), *GetNameSafe(this));
		GI->GSFindSession(this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGS_MainMenuPC: GetGameInstance<UGS_GameInstance>() FAILED. GI is NULL."));
		UGameInstance* GenericGI = GetGameInstance();
		if (GenericGI)
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_MainMenuPC: GameInstance is of type: %s. Did you set UGS_GameInstance in Project Settings?"), *GenericGI->GetClass()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGS_MainMenuPC: GetGameInstance() also returned NULL."));
		}
	}
}

void AGS_MainMenuPC::BeginPlay()
{
	Super::BeginPlay();

	ShowMainMenuUI();

	if (IsLocalController()) // MainLevel 앞에 TitleLevel 추가할 거면 이거도 옮겨야 됨!!!!
	{
		UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>();
		if (GI)
		{
			FString ConnectString = GI->GetAndClearPendingConnectString();
			if (!ConnectString.IsEmpty())
			{
				UE_LOG(LogTemp, Log, TEXT("AGS_MainMenuPC: Traveling to server from command line: %s"), *ConnectString);
				ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void AGS_MainMenuPC::SetupInputComponent()
{
	Super::SetupInputComponent();
	
}

void AGS_MainMenuPC::ShowMainMenuUI()
{
	if (!IsLocalController()) return;

	if (MainMenuWidgetClass)
	{
		if (MainMenuWidgetInstance && MainMenuWidgetInstance->IsInViewport())
		{
			UE_LOG(LogTemp, Log, TEXT("AGS_MainMenuPC: MainMenuUI is already visible."));
			return;
		}
		if (MainMenuWidgetInstance) {
			MainMenuWidgetInstance->RemoveFromParent();
			MainMenuWidgetInstance = nullptr;
		}

		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

		if (MainMenuWidgetInstance)
		{
			// 생성된 위젯을 뷰포트에 추가하여 화면에 표시합니다.
			MainMenuWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("AGS_MainMenuPC: MainMenuUI created and added to viewport."));
			//if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(MainMenuWidgetInstance))
			//{
			//	// 앵커를 중앙으로 설정 (Min: 0.5, 0.5 / Max: 0.5, 0.5)
			//	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			//	// 위젯의 정렬점도 중앙으로 설정 (위젯 자체의 (0.5, 0.5) 지점을 앵커에 맞춤)
			//	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			//	// 앵커로부터의 위치는 (0,0)으로 설정하여 정확히 중앙에 오도록 함
			//	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
			//	// 위젯이 콘텐츠 크기에 맞게 자동으로 크기가 조절되도록 설정할 수 있습니다.
			//	// 또는 특정 크기를 지정할 수도 있습니다.
			//	CanvasSlot->SetAutoSize(true); // UMG에서 위젯의 크기가 "Size to Content"로 설정되어 있다면 이게 잘 동작합니다.
			//	// 만약 고정 크기를 원한다면:
			//	// CanvasSlot->SetSize(FVector2D(원하는너비, 원하는높이));
			//}
			//else
			//{
			//	UE_LOG(LogTemp, Warning, TEXT("AGS_MainMenuPC: MainMenuWidgetInstance could not be cast to CanvasPanelSlot. Centering might not work as expected."));
			//}
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget()); // 키보드 포커스 설정
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 락 해제
			SetInputMode(InputModeData);
			SetShowMouseCursor(true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGS_MainMenuPC: Failed to create MainMenuWidget instance."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGS_MainMenuPC: MainMenuWidgetClass is not set in PlayerController's properties."));
	}
}
