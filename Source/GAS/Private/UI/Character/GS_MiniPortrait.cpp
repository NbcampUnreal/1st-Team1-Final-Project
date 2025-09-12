// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_MiniPortrait.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UGS_MiniPortrait::Init(AGS_Monster* Monster)
{
	BoundMonster = Monster;
	PortraitImage->SetBrushFromTexture(Monster->GetPortrait());
	DebuffColor = DebuffBorder->GetBrushColor(); 

	if (UGS_StatComp* StatComp = Monster->GetStatComp())
	{
		BoundStatComp = StatComp;
		StatComp->OnCurrentHPChanged.AddUObject(this, &UGS_MiniPortrait::OnHPChanged);
		OnHPChanged(StatComp); 
	}

	if (UGS_DebuffComp* DebuffComp = Monster->GetDebuffComp())
	{
		BoundDebuffComp = DebuffComp;
		DebuffComp->OnDebuffListUpdated.AddUObject(this, &UGS_MiniPortrait::OnDebuffChanged);
		OnDebuffChanged(DebuffComp->GetDebuffList()); 
	}
}

void UGS_MiniPortrait::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (BoundStatComp.IsValid())
	{
		BoundStatComp->OnCurrentHPChanged.RemoveAll(this);
	}

	if (BoundDebuffComp.IsValid())
	{
		BoundDebuffComp->OnDebuffListUpdated.RemoveAll(this);
	}
}

void UGS_MiniPortrait::OnHPChanged(UGS_StatComp* InStatComp)
{
	HPText->SetText(FText::FromString(FString::Printf(TEXT("%d"),FMath::RoundToInt(InStatComp->GetCurrentHealth()))));
}

void UGS_MiniPortrait::OnDebuffChanged(const TArray<FDebuffRepInfo>& List)
{
	bool bHasDebuff = List.Num() > 0;
	
	DebuffColor.A = bHasDebuff ? 1.f : 0.f;
	DebuffBorder->SetBrushColor(DebuffColor);
}

void UGS_MiniPortrait::OnPortraitClicked()
{
	UE_LOG(LogTemp, Log, TEXT("몬스터: %s"), *BoundMonster->GetName());
}