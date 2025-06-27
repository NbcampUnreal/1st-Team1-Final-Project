// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/Popup/GS_TutorialWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "CommonButtonBase.h"
#include "System/PlayerController/GS_MainMenuPC.h"

UGS_TutorialWidget::UGS_TutorialWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrPageIndex = 0;
	TutorialImageTable = nullptr;
}

void UGS_TutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PrevButton)
	{
		PrevButton->OnClicked().AddUObject(this, &UGS_TutorialWidget::OnPrevButtonClicked);
	}

	if (NextButton)
	{
		NextButton->OnClicked().AddUObject(this, &UGS_TutorialWidget::OnNextButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked().AddUObject(this, &UGS_TutorialWidget::OnExitButtonClicked);
	}

	InitTutorial();
}

void UGS_TutorialWidget::InitTutorial()
{
	if (!TutorialImageTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialDataTable is not set!"));
		return;
	}

	LoadTutorialData();
	CreatePageButtons();

	if (TutorialDataArray.Num() > 0)
	{
		GoToPage(0);
	}
}

void UGS_TutorialWidget::GoToPage(uint8 PageIndex)
{
	if (TutorialDataArray.IsValidIndex(PageIndex))
	{
		CurrPageIndex = PageIndex;
		UpdateCurrPage();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid page index: %d"), PageIndex);
	}
}

void UGS_TutorialWidget::GoToPrevPage()
{
	if (CurrPageIndex > 0)
	{
		GoToPage(CurrPageIndex - 1);
	}
	else if(CurrPageIndex == 0)
	{
		GoToPage(TutorialDataArray.Num()-1);
	}
}

void UGS_TutorialWidget::GoToNextPage()
{
	if (CurrPageIndex < TutorialDataArray.Num() - 1)
	{
		GoToPage(CurrPageIndex + 1);
	}
	else if (CurrPageIndex == TutorialDataArray.Num() - 1)
	{
		GoToPage(0);
	}
}

void UGS_TutorialWidget::LoadTutorialData()
{
	if (!TutorialImageTable)
	{
		return;
	}

	TArray<FName> RowNames = TutorialImageTable->GetRowNames();
	TutorialDataArray.Empty();

	for (const FName& RowName : RowNames)
	{
		FTutorialImageRow* RowData = TutorialImageTable->FindRow<FTutorialImageRow>(RowName, TEXT("LoadTutorialData"));
		if (RowData)
		{
			TutorialDataArray.Add(*RowData);
		}
	}

	TutorialDataArray.Sort([](const FTutorialImageRow& A, const FTutorialImageRow& B) {
		return A.PageIndex < B.PageIndex;
	});

	UE_LOG(LogTemp, Log, TEXT("Loaded %d tutorial pages"), TutorialDataArray.Num());
}

void UGS_TutorialWidget::CreatePageButtons()
{
	if (!PageButtonContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("PageButtonContainer is not set!"));
		return;
	}

	if (!PageButtonClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PageButtonClass is null!"));
		return;
	}

	PageButtonContainer->ClearChildren();
	PageButtons.Empty();

	for (int32 i = 0; i < TutorialDataArray.Num(); ++i)
	{
		UCommonButtonBase* PageButton = CreateWidget<UCommonButtonBase>(this, PageButtonClass);
		if (PageButton)
		{
			PageButton->SetVisibility(ESlateVisibility::Visible);
			PageButton->SetPadding(FMargin(10.f));
			PageButtonContainer->AddChild(PageButton);

			PageButtons.Add(PageButton);

			FText ButtonText = TutorialDataArray[i].Title;
			InitButton(PageButton, ButtonText);
		}
	}

	BindPageButtonEvents();
}

void UGS_TutorialWidget::UpdateCurrPage()
{
	if (!TutorialDataArray.IsValidIndex(CurrPageIndex))
	{
		return;
	}

	const FTutorialImageRow& CurrentData = TutorialDataArray[CurrPageIndex];

	if (TutorialImage && !CurrentData.TutorialImage.IsNull())
	{
		UTexture2D* LoadedTexture = CurrentData.TutorialImage.LoadSynchronous();
		if (LoadedTexture)
		{
			TutorialImage->SetBrushFromTexture(LoadedTexture);
		}
	}

	UpdatePageButtonStates();

	UE_LOG(LogTemp, Log, TEXT("Updated to page %d: %s"), CurrPageIndex, *CurrentData.Title.ToString());
}

void UGS_TutorialWidget::UpdatePageButtonStates()
{
	for (int32 i = 0; i < PageButtons.Num(); ++i)
	{
		if (PageButtons[i])
		{
			bool bIsCurrPage = (i == CurrPageIndex);
			ChangeButtonState(PageButtons[i], bIsCurrPage);
		}
	}
}

void UGS_TutorialWidget::OnPageButtonClicked(uint8 PageIndex)
{
	GoToPage(PageIndex);
}

void UGS_TutorialWidget::OnPrevButtonClicked()
{
	GoToPrevPage();
}

void UGS_TutorialWidget::OnNextButtonClicked()
{
	GoToNextPage();
}

void UGS_TutorialWidget::OnExitButtonClicked()
{
	AGS_MainMenuPC* PC = GetOwningPlayer<AGS_MainMenuPC>();
	if (PC)
	{
		PC->HideTutorialUI();
	}
}

void UGS_TutorialWidget::BindPageButtonEvents()
{
	for (int32 i = 0; i < PageButtons.Num(); ++i)
	{
		if (PageButtons[i])
		{
			const int32 PageIndex = i;
			PageButtons[i]->OnClicked().AddLambda([this, PageIndex]() {
				OnPageButtonClicked(PageIndex);
				});
		}
	}
}
