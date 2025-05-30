// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_ArrowTypeWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Merci.h"


void UGS_ArrowTypeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwningCharacter);
	MerciCharacter->SetArrowTypeWidget(this);
	UpdateArrowImage(EArrowType::Normal);
	UpdateArrowCount(999);
}

void UGS_ArrowTypeWidget::SetOwningActor(AGS_Character* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
}

void UGS_ArrowTypeWidget::UpdateArrowImage(EArrowType InArrowType)
{
	if (!ArrowImage) 
	{
		return;
	}

	UTexture2D* NewTexture = nullptr;

	switch (InArrowType)
	{
	case EArrowType::Normal:
		NewTexture = NormalArrowTexture;
		break;
	case EArrowType::Axe:
		NewTexture = AxeArrowTexture;
		break;
	case EArrowType::Child:
		NewTexture = ChildArrowTexture;
		break;
	default:
		NewTexture = NormalArrowTexture;
		break;
	}

	if (NewTexture)
	{
		ArrowImage->SetBrushFromTexture(NewTexture);
	}
}

void UGS_ArrowTypeWidget::UpdateArrowCount(int32 InArrowCount)
{
	if (ArrowNumText)
	{
		if (InArrowCount > 900)
		{
			ArrowNumText->SetText(FText::GetEmpty());
		}
		else
		{
			ArrowNumText->SetText(FText::AsNumber(InArrowCount));
		}
	}
}
