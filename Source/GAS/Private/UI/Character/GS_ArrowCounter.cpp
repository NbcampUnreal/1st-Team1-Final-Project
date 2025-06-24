// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/Character/GS_ArrowCounter.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_ArrowType.h"

void UGS_ArrowCounter::NativeConstruct()
{
	Super::NativeConstruct();

	MaxArrowCnt = 5;
}

void UGS_ArrowCounter::Init(EArrowType InArrowType)
{
	ArrowType = InArrowType;
	AGS_Merci* Merci = Cast<AGS_Merci>(GetOwningPlayerPawn());

	if (!Merci)
	{
		return;
	}

	switch (InArrowType)
	{
	case EArrowType::Axe:
		MaxArrowCnt = Merci->GetMaxAxeArrows();
		break;
	case EArrowType::Child:
		MaxArrowCnt = Merci->GetMaxChildArrows();
		break;
	default:
		break;
	}

	CreateIndicators();
}

void UGS_ArrowCounter::CreateIndicators()
{
	if (!ArrowContainer || !ArrowIndicatorClass)
	{
		return;
	}

	ClearIndicators();

	UTexture2D* TextureToUse = GetTextureForArrowType();

	if (!TextureToUse)
	{
		return;
	}

	for (uint8 i = 0; i < MaxArrowCnt; ++i)
	{
		UGS_ArrowIndicator* NewIndicator = CreateWidget<UGS_ArrowIndicator>(this, ArrowIndicatorClass);
		if (NewIndicator)
		{
			UE_LOG(LogTemp, Warning, TEXT("Created indicator %d"), i);
			NewIndicator->SetArrowTexture(TextureToUse);
			UHorizontalBoxSlot* HorizontalSlot = ArrowContainer->AddChildToHorizontalBox(NewIndicator);
			if(HorizontalSlot)
			{
				HorizontalSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
				HorizontalSlot->SetPadding(FMargin(2.0f));
				HorizontalSlot->SetHorizontalAlignment(HAlign_Center);
				HorizontalSlot->SetVerticalAlignment(VAlign_Center);
			}
			ArrowIndicators.Add(NewIndicator);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create indicator %d"), i);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Total indicators created: %d"), ArrowIndicators.Num());
}

UTexture2D* UGS_ArrowCounter::GetTextureForArrowType() const
{
	switch (ArrowType)
	{
	case EArrowType::Axe:
		return AxeArrowTexture;
	case EArrowType::Child:
		return ChildArrowTexture;
	default:
		return nullptr;
	}
}

void UGS_ArrowCounter::UpdateArrowCnt(uint8 CurrCnt)
{
	for (uint8 i = 0; i < ArrowIndicators.Num(); ++i)
	{
		if (ArrowIndicators[i])
		{
			bool bShoudBeActive = (i < CurrCnt);
			ArrowIndicators[i]->SetActive(bShoudBeActive);
		}
	}
}

void UGS_ArrowCounter::ClearIndicators()
{
	if (!ArrowContainer)
	{
		return;
	}

	ArrowContainer->ClearChildren();
	ArrowIndicators.Empty();
}
