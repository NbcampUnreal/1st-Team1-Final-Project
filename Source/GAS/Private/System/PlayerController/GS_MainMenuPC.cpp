#include "System/PlayerController/GS_MainMenuPC.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h" //slot이 붙는 게 맞나?
#include "Kismet/GameplayStatics.h"
#include "System/GS_GameInstance.h"

AGS_MainMenuPC::AGS_MainMenuPC()
	: MainMenuWidgetInstance(nullptr)
	, LoadingScreenWidgetInstance(nullptr)
	, TutorialWidgetInstance(nullptr)
{
}

void AGS_MainMenuPC::HandleCustomGameRequest()
{
	UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>();
	if (GI)
	{
		ShowLoadingScreen();
		GI->GSFindSession(this);
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
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->RemoveFromParent();
			MainMenuWidgetInstance = nullptr;
		}

		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget()); // 키보드 포커스 설정
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 락 해제
			SetInputMode(InputModeData);
			SetShowMouseCursor(true);
		}
	}
}

void AGS_MainMenuPC::ShowLoadingScreen()
{
	if (LoadingScreenWidgetInstance || !LoadingScreenWidgetClass)
	{
		return;
	}

	LoadingScreenWidgetInstance = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass);
	if (LoadingScreenWidgetInstance)
	{
		LoadingScreenWidgetInstance->AddToViewport(1);

		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AGS_MainMenuPC::HideLoadingScreen()
{
	if (LoadingScreenWidgetInstance)
	{
		LoadingScreenWidgetInstance->RemoveFromParent();
		LoadingScreenWidgetInstance = nullptr;

		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AGS_MainMenuPC::ShowTutorialUI()
{
	if (!TutorialWidgetClass)
	{
		return;
	}

	TutorialWidgetInstance = CreateWidget<UUserWidget>(this, TutorialWidgetClass);
	if (TutorialWidgetInstance)
	{
		TutorialWidgetInstance->AddToViewport();

		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AGS_MainMenuPC::HideTutorialUI()
{
	if (TutorialWidgetInstance)
	{
		TutorialWidgetInstance->RemoveFromParent();
		LoadingScreenWidgetInstance = nullptr;

		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}
