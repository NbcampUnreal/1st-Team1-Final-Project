// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_UnitSelection.h"
#include "AI/RTS/GS_RTSController.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Character/GS_MiniPortrait.h"

void UGS_UnitSelection::NativeConstruct()
{
	Super::NativeConstruct();

	if (AGS_RTSController* RTSController = GetRTSController())
	{
		RTSController->OnSelectionChanged.AddDynamic(this, &UGS_UnitSelection::HandleSelectionChanged);
		HandleSelectionChanged(RTSController->GetUnitSelection());
	}
}

void UGS_UnitSelection::HandleSelectionChanged(const TArray<AGS_Monster*>& NewSelection)
{
	if (!SelectionSwitcher) return;

	if (NewSelection.Num() == 1)
	{
		SelectionSwitcher->SetActiveWidgetIndex(0);

		AGS_Monster* Monster = NewSelection[0];
		PortraitImage->SetBrushFromTexture(Monster->GetPortrait());
		NameText->SetText(Monster->GetMonsterName());
		DescText->SetText(Monster->GetDescription());
		TypeText->SetText(Monster->GetTypeName());

		if (UGS_StatComp* StatComp = Monster->GetStatComp())
		{
			if (BoundStatComp != StatComp)
			{
				if (BoundStatComp.IsValid())
				{
					BoundStatComp->OnCurrentHPChanged.RemoveAll(this);
				}
				
				StatComp->OnCurrentHPChanged.AddUObject(this, &UGS_UnitSelection::OnHPChanged);
				BoundStatComp = StatComp;
			}

			OnHPChanged(StatComp); 
		}

		if (UGS_DebuffComp* DebuffComp = Monster->GetDebuffComp())
		{
			if (BoundDebuffComp != DebuffComp)
			{
				if (BoundDebuffComp.IsValid())
				{
					BoundDebuffComp->OnDebuffListUpdated.RemoveAll(this);
				}
				
				DebuffComp->OnDebuffListUpdated.AddUObject(this, &UGS_UnitSelection::OnDebuffChanged);
				BoundDebuffComp = DebuffComp;
			}

			OnDebuffChanged(DebuffComp->GetDebuffList()); 
		}
	}
	else
	{
		SelectionSwitcher->SetActiveWidgetIndex(1);
		MultiIconsGrid->ClearChildren();

		const int32 Cols = 5;  
		int32 Index = 0;
		for (AGS_Monster* Monster : NewSelection)
		{
			if (!MiniPortraitWidgetClass)
			{
				continue;
			}

			UGS_MiniPortrait* W = CreateWidget<UGS_MiniPortrait>(this, MiniPortraitWidgetClass);
			W->Init(Monster);

			const int32 Row = Index / Cols;
			const int32 Col = Index % Cols;
			MultiIconsGrid->AddChildToUniformGrid(W, Row, Col);
			++Index;
		}
	}
}

void UGS_UnitSelection::OnHPChanged(UGS_StatComp* InStatComp)
{
	HPText->SetText(FText::FromString(FString::Printf(TEXT("%d"),FMath::RoundToInt(InStatComp->GetCurrentHealth()))));
}

void UGS_UnitSelection::OnDebuffChanged(const TArray<FDebuffRepInfo>& List)
{
	static const UEnum* EnumPtr = StaticEnum<EDebuffType>();
	FString Result;
	for (const FDebuffRepInfo& Debuff : List)
	{
		FText Display = EnumPtr->GetDisplayNameTextByValue(int64(Debuff.Type));
		Result += Display.ToString() + TEXT(" ");
	}
	
	DebuffText->SetText(FText::FromString(Result));
}

AGS_RTSController* UGS_UnitSelection::GetRTSController() const
{
	return Cast<AGS_RTSController>(GetOwningPlayer());
}
