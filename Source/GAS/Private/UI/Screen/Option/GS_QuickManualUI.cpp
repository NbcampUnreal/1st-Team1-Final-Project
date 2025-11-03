// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/Screen/Option/GS_QuickManualUI.h"
#include "UI/Data/GS_ManualRow.h"
#include "AI/RTS/GS_RTSController.h"
#include "Character/Interface/GS_ManualDataInterface.h"
#include "Components/Image.h"
#include "CommonButtonBase.h"

void UGS_QuickManualUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExitButton)
	{
		ExitButton->OnClicked().AddUObject(this, &UGS_QuickManualUI::OnExitButtonClicked);
	}
}

void UGS_QuickManualUI::InitImage()
{
	FName RowName;
	APlayerController* PC = GetOwningPlayer();

	// RTS 시점일 경우
	AGS_RTSController* RTSCtrl = Cast<AGS_RTSController>(PC);
	if (RTSCtrl)
	{
		RowName = FName("Guardian");
	}

	// TPS 시점일 경우
	APawn* Pawn = GetOwningPlayer()->GetPawn();
	if (Pawn && Pawn->Implements<UGS_ManualDataInterface>())
	{
		RowName = IGS_ManualDataInterface::Execute_GetManualRowName(Pawn);
	}

	if(!RowName.IsNone())
	{
		FManualImageRow* RowData = ManualImageTable->FindRow<FManualImageRow>(RowName, TEXT("InitImage"));
		UTexture2D* LoadedTexture = RowData->ManualImage.LoadSynchronous();
		if (LoadedTexture)
		{
			KeyManualImage->SetBrushFromTexture(LoadedTexture);
		}
	}
}

void UGS_QuickManualUI::OnExitButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		SetVisibility(ESlateVisibility::Hidden);
	}
}
