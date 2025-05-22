#include "UI/DungeonEditor/GS_PropWidget.h"

#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "DungeonEditor/GS_DEController.h"
#include "RuneSystem/GS_EnumUtils.h"
#include "UI/DungeonEditor/GS_SelectBundleWidget.h"

void UGS_PropWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeBundleSwitcher();

	if (SwitcherButton0)
	{
		SwitcherButton0->OnClicked.AddDynamic(this, &UGS_PropWidget::PressedSwitcherBtn0);
		if (SwitcherTitle0)
		{
			SwitcherTitle0->SetText(UGS_EnumUtils::GetEnumAsText<EObjectType>(EObjectType::Room));
		}
	}

	if (SwitcherButton1)
	{
		SwitcherButton1->OnClicked.AddDynamic(this, &UGS_PropWidget::PressedSwitcherBtn1);
		if (SwitcherTitle1)
		{
			SwitcherTitle1->SetText(UGS_EnumUtils::GetEnumAsText<EObjectType>(EObjectType::Trap));
		}
	}

	if (SwitcherButton2)
	{
		SwitcherButton2->OnClicked.AddDynamic(this, &UGS_PropWidget::PressedSwitcherBtn2);
		if (SwitcherTitle2)
		{
			SwitcherTitle2->SetText(UGS_EnumUtils::GetEnumAsText<EObjectType>(EObjectType::Monster));			
		}
	}
	
	if (BundleSwitcher)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
			{
				if (AGS_BuildManager* BuildManagerRef = DEPC->GetBuildManager())
				{
					if (TArray<FDataTableRowHandle>* DataTableRows = BuildManagerRef->GetBuildingsRows())
					{
						for (int i = 0; i < DataTableRows->Num(); ++i)
						{
							FDataTableRowHandle Data = DataTableRows->GetData()[i];

							FGS_PlaceableObjectsRow* RowPtr = Data.GetRow<FGS_PlaceableObjectsRow>(Data.RowName.ToString());
							EObjectType ObjectType = RowPtr->ObjectType;
							
							if (SelectorBundleWidgets[(int)ObjectType])
							{
								if (SelectorBundleWidgets[(int)ObjectType])
								{
									SelectorBundleWidgets[(int)ObjectType]->CreateSelector(Data);
								}
							}
						}
					}
				}
			}
		}			
	}
}

void UGS_PropWidget::PressedSwitcherBtn0()
{
	SwitchBundle(EObjectType::Room);
}

void UGS_PropWidget::PressedSwitcherBtn1()
{
	SwitchBundle(EObjectType::Trap);
}

void UGS_PropWidget::PressedSwitcherBtn2()
{
	SwitchBundle(EObjectType::Monster);
}

void UGS_PropWidget::InitializeBundleSwitcher()
{
	SelectorBundleWidgets.SetNum(UGS_EnumUtils::GetEnumCount<EObjectType>() - 2);
	for (int i = 0; i < SelectorBundleWidgets.Num(); ++i)
	{
		UGS_SelectBundleWidget* NewSelectBundleWidget = CreateWidget<UGS_SelectBundleWidget>(this, SelectorBundleWidgetClass);
		NewSelectBundleWidget->InitWidget((EObjectType)i);
		NewSelectBundleWidget->SetTitle(UGS_EnumUtils::GetEnumAsText<EObjectType>((EObjectType)i));
		BundleSwitcher->AddChild(NewSelectBundleWidget);
		SelectorBundleWidgets[i] = NewSelectBundleWidget;
	}
}

void UGS_PropWidget::OpenButton(EObjectType InObjectType)
{
	if (PrevClickSwitcherBtn != nullptr)
	{
		ChangeButtonSize(PrevClickSwitcherBtn, FVector2D(137.5f, 60.0f));
	}

	switch (InObjectType)
	{
	case EObjectType::Room:
		ChangeButtonSize(SwitcherButton0, FVector2D(137.5f, 80.0f));
		PrevClickSwitcherBtn = SwitcherButton0;
		break;
		
	case EObjectType::Trap:
		ChangeButtonSize(SwitcherButton1, FVector2D(137.5f, 80.0f));
		PrevClickSwitcherBtn = SwitcherButton1;
		break;
		
	case EObjectType::Monster:
		ChangeButtonSize(SwitcherButton2, FVector2D(137.5f, 80.0f));
		PrevClickSwitcherBtn = SwitcherButton2;
		break;
		
	default:
		break;
	}
}

void UGS_PropWidget::SwitchBundle(EObjectType InObjectType)
{
	if (!BundleSwitcher)
		return;
	
	switch (InObjectType)
	{
	case EObjectType::Room:
		OpenButton(EObjectType::Room);
		BundleSwitcher->SetActiveWidgetIndex((UINT)EObjectType::Room);
		break;
			
	case EObjectType::Trap:
		OpenButton(EObjectType::Trap);
		BundleSwitcher->SetActiveWidgetIndex((UINT)EObjectType::Trap);
		break;
		
	case EObjectType::Monster:
		OpenButton(EObjectType::Monster);
		BundleSwitcher->SetActiveWidgetIndex((UINT)EObjectType::Monster);
		break;
		
	default:
		break;
	}
}

void UGS_PropWidget::ChangeButtonSize(TObjectPtr<UButton> InSwitcherBtn, FVector2d InSize)
{
	if (InSwitcherBtn)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InSwitcherBtn->Slot))
		{
			CanvasSlot->SetSize(InSize);  
		}	
	}
}