// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_ChanAimingSkillBar.h"
#include "Components/ProgressBar.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_ChanAimingSkillBar::NativeConstruct()
{
	Super::NativeConstruct();
	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwningCharacter);
	ChanCharacter->SetChanAimingSkillBarWidget(this);
	ShowSkillBar(false);
}

void UGS_ChanAimingSkillBar::SetOwningActor(AGS_Character* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
}

void UGS_ChanAimingSkillBar::SetAimingProgress(float Progress)
{
	if (AimingProgressBar)
	{
		AimingProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

void UGS_ChanAimingSkillBar::ShowSkillBar(bool bShow)
{
	SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
