// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_HPText.h"

#include "Character/Component/GS_StatComp.h"
#include "Character/GS_Character.h"

#include "Components/TextBlock.h"

UGS_HPText::UGS_HPText(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGS_HPText::NativeConstruct()
{
	Super::NativeConstruct();

	AGS_Character* OwningCharacter = Cast<AGS_Character>(OwningActor);
	if (IsValid(OwningCharacter))
	{
		OwningCharacter->SetHPTextWidget(this);
	}
}

void UGS_HPText::InitializeHPTextWidget(UGS_StatComp* InStatComp)
{
	OnCurrentHPChanged(InStatComp->GetCurrentHealth());
}

void UGS_HPText::OnCurrentHPChanged(float InCurrentHP)
{
	CurrentHPText->SetText(FText::FromString(FString::SanitizeFloat(InCurrentHP)));
}
