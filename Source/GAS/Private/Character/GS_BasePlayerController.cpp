// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GS_BasePlayerController.h"
#include "EnhancedInputComponent.h"
#include "UI/Screen/Option/GS_InGameMenuUI.h"
#include "UI/Screen/Option/GS_QuickManualUI.h"
#include "System/GISubsys//GS_SeamlessTravelLoadingSubsystem.h"
#include "Engine/GameInstance.h"
#include "System/GS_BaseGM.h"

void AGS_BasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	MenuAction = nullptr;
	Server_NotifyPlayerIsReady();
}

void AGS_BasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	if (MenuAction)
	{
		EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Triggered, this, &AGS_BasePlayerController::OpenMenuUI);
	}

	if (KeyManualAction)
	{
		EnhancedInputComponent->BindAction(KeyManualAction, ETriggerEvent::Triggered, this, &AGS_BasePlayerController::OpenKeyManual);
	}
}

void AGS_BasePlayerController::OpenMenuUI(const FInputActionValue& InputValue)
{
	if (!InGameMenuUI)
	{
		if (InGameMenuUIClass)
		{
			InGameMenuUI = CreateWidget<UGS_InGameMenuUI>(this, InGameMenuUIClass);
			InGameMenuUI->AddToViewport(1);
		}
	}
	else
	{
		InGameMenuUI->SetVisibility(ESlateVisibility::Visible);
	}

	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}

void AGS_BasePlayerController::OpenKeyManual(const FInputActionValue& InputValue)
{
	if (!QuickManualUI)
	{
		if (QuickManualUIClass)
		{
			QuickManualUI = CreateWidget<UGS_QuickManualUI>(this, QuickManualUIClass);
			QuickManualUI->InitImage();
			QuickManualUI->AddToViewport(1);
			
			// UI와 게임 입력을 모두 받을 수 있도록 설정
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		else
		{
			return;
		}
	}
	else
	{
		// UI가 이미 보이는 상태면 닫기, 안 보이면 열기
		if (QuickManualUI->IsVisible())
		{
			QuickManualUI->SetVisibility(ESlateVisibility::Hidden);
			SetInputMode(FInputModeGameOnly());
			bShowMouseCursor = false;
		}
		else
		{
			QuickManualUI->SetVisibility(ESlateVisibility::Visible);
			
			// UI와 게임 입력을 모두 받을 수 있도록 설정
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void AGS_BasePlayerController::Server_NotifyPlayerIsReady_Implementation()
{
	if (AGS_BaseGM* GM = GetWorld()->GetAuthGameMode<AGS_BaseGM>())
	{
		GM->NotifyPlayerIsReady(this);
	}
}

void AGS_BasePlayerController::Client_StartGame_Implementation()
{
	
}

void AGS_BasePlayerController::Client_ShowSeamlessLoadingCover_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGS_SeamlessTravelLoadingSubsystem* Subsys = GI->GetSubsystem<UGS_SeamlessTravelLoadingSubsystem>())
		{
			if (ULocalPlayer* LP = GetLocalPlayer())
			{
				Subsys->ShowLoadingCover(LP);
			}
			else
			{
				// ShowLoadingCover 내부에서 어차피 GetPrimaryLocalPlayer() 호출함
				Subsys->ShowLoadingCover(nullptr);
			}
		}
	}
}

void AGS_BasePlayerController::Client_HideSeamlessLoadingCover_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGS_SeamlessTravelLoadingSubsystem* Subsys = GI->GetSubsystem<UGS_SeamlessTravelLoadingSubsystem>())
		{
			Subsys->HideLoadingCover();
		}
	}
}