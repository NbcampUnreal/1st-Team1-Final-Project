// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/Screen/Option/GS_InGameManualWidget.h"
#include "CommonButtonBase.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"

UGS_InGameManualWidget::UGS_InGameManualWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrPageIndex = 0;
	ManualImageTable = nullptr;
}

void UGS_InGameManualWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PrevButton)
	{
		PrevButton->OnClicked().AddUObject(this, &UGS_InGameManualWidget::OnPrevButtonClicked);
	}

	if (NextButton)
	{
		NextButton->OnClicked().AddUObject(this, &UGS_InGameManualWidget::OnNextButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked().AddUObject(this, &UGS_InGameManualWidget::OnExitButtonClicked);
	}

	InitManual();
}

void UGS_InGameManualWidget::InitManual()
{
	if (!ManualImageTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ManualDataTable is not set!"));
		return;
	}

	LoadManualData();
	CreatePageButtons();

	if (ManualDataArray.Num() > 0)
	{
		GoToPage(0);
	}
}

void UGS_InGameManualWidget::GoToPage(uint8 PageIndex)
{
	if (ManualDataArray.IsValidIndex(PageIndex))
	{
		CurrPageIndex = PageIndex;
		UpdateCurrPage();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid page index: %d"), PageIndex);
	}
}

void UGS_InGameManualWidget::GoToPrevPage()
{
	if (CurrPageIndex > 0)
	{
		GoToPage(CurrPageIndex - 1);
	}
	else if (CurrPageIndex == 0)
	{
		GoToPage(ManualDataArray.Num() - 1);
	}
}

void UGS_InGameManualWidget::GoToNextPage()
{
	if (CurrPageIndex < ManualDataArray.Num() - 1)
	{
		GoToPage(CurrPageIndex + 1);
	}
	else if (CurrPageIndex == ManualDataArray.Num() - 1)
	{
		GoToPage(0);
	}
}

void UGS_InGameManualWidget::LoadManualData()
{
	if (!ManualImageTable)
	{
		return;
	}

	TArray<FName> RowNames = ManualImageTable->GetRowNames();
	ManualDataArray.Empty();

	for (const FName& RowName : RowNames)
	{
		FManualImageRow* RowData = ManualImageTable->FindRow<FManualImageRow>(RowName, TEXT("LoadManualData"));
		if (RowData)
		{
			ManualDataArray.Add(*RowData);
		}
	}

	ManualDataArray.Sort([](const FManualImageRow& A, const FManualImageRow& B) {
		return A.PageIndex < B.PageIndex;
		});

	UE_LOG(LogTemp, Log, TEXT("Loaded %d Manual pages"), ManualDataArray.Num());
}

void UGS_InGameManualWidget::CreatePageButtons()
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

	for (int32 i = 0; i < ManualDataArray.Num(); ++i)
	{
		UCommonButtonBase* PageButton = CreateWidget<UCommonButtonBase>(this, PageButtonClass);
		if (PageButton)
		{
			PageButton->SetVisibility(ESlateVisibility::Visible);
			PageButton->SetPadding(FMargin(10.f));
			PageButtonContainer->AddChild(PageButton);

			PageButtons.Add(PageButton);

			FText ButtonText = ManualDataArray[i].Title;
			InitButton(PageButton, ButtonText);
		}
	}

	BindPageButtonEvents();
}

void UGS_InGameManualWidget::UpdateCurrPage()
{
	if (!ManualDataArray.IsValidIndex(CurrPageIndex))
	{
		return;
	}

	const FManualImageRow& CurrentData = ManualDataArray[CurrPageIndex];

	if (ManualImage && !CurrentData.ManualImage.IsNull())
	{
		UTexture2D* LoadedTexture = CurrentData.ManualImage.LoadSynchronous();
		if (LoadedTexture)
		{
			ManualImage->SetBrushFromTexture(LoadedTexture);
			UpdatePageButtonStates();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Updated to page %d: %s"), CurrPageIndex, *CurrentData.Title.ToString());
}

void UGS_InGameManualWidget::UpdatePageButtonStates()
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

void UGS_InGameManualWidget::OnPageButtonClicked(uint8 PageIndex)
{
	GoToPage(PageIndex);
}

void UGS_InGameManualWidget::OnPrevButtonClicked()
{
	GoToPrevPage();
}

void UGS_InGameManualWidget::OnNextButtonClicked()
{
	GoToNextPage();
}

void UGS_InGameManualWidget::OnExitButtonClicked()
{
	RemoveFromParent();
}

void UGS_InGameManualWidget::BindPageButtonEvents()
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
