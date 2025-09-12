// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_ChanAimingSkillBar.h"
#include "Components/ProgressBar.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Kismet/KismetMathLibrary.h"

void UGS_ChanAimingSkillBar::NativeConstruct()
{
	Super::NativeConstruct();
	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwningCharacter);
	if(ChanCharacter)
	{
		ChanCharacter->SetChanAimingSkillBarWidget(this);
	}
	ShowSkillBar(false);

	// 초기화
	if (AimingProgressBar)
	{
		AimingProgressBar->SetPercent(1.0f);
	}
	if (AimingProgressBar_back)
	{
		AimingProgressBar_back->SetPercent(1.0f);
	}
}

void UGS_ChanAimingSkillBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (AimingProgressBar_back)
	{
		float Current = AimingProgressBar_back->GetPercent();
		float NewValue = FMath::FInterpTo(Current, TargetBackPercent, InDeltaTime, BackBarInterpSpeed);
		AimingProgressBar_back->SetPercent(NewValue);
	}
}

void UGS_ChanAimingSkillBar::SetOwningActor(AGS_Character* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
}

void UGS_ChanAimingSkillBar::SetAimingProgress(float Progress)
{
	Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

	// 시간 소모 → 앞뒤 둘 다 즉시
	if (AimingProgressBar)
		AimingProgressBar->SetPercent(Progress);
	if (AimingProgressBar_back)
		AimingProgressBar_back->SetPercent(Progress);

	TargetBackPercent = Progress; // 싱크 맞춰줌
}

void UGS_ChanAimingSkillBar::SetAimingProgressByDamage(float Progress)
{
	Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

	// 앞바는 즉시
	if (AimingProgressBar)
		AimingProgressBar->SetPercent(Progress);

	// 뒤바는 보간 목표만 변경
	TargetBackPercent = Progress;
}

void UGS_ChanAimingSkillBar::ShowSkillBar(bool bShow)
{
	SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
