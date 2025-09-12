// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_CrossHairImage.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "Character/GS_Character.h"
#include "UI/Character/GS_ArrowCounter.h"
#include "Weapon/Projectile/Seeker/GS_ArrowType.h"

void UGS_CrossHairImage::NativeConstruct()
{
	Super::NativeConstruct();
	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	
	bCurrentlyAiming = false;
	bIsAnimating = false;
	Center->SetVisibility(ESlateVisibility::Hidden);
	SetCrosshairVisibility(true);

	if (ArrowCounterSwitcher)
	{
		ArrowCounterSwitcher->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGS_CrossHairImage::NativeOnInitialized()
{
	if (Aim_Anim)
	{
		AimAnimEndDelegate.Clear();
		AimAnimEndDelegate.BindDynamic(this, &UGS_CrossHairImage::OnAimAnimFinished);
		BindToAnimationFinished(Aim_Anim, AimAnimEndDelegate);
	}
}

void UGS_CrossHairImage::SetOwingActor(AGS_Character* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
}

void UGS_CrossHairImage::PlayAimAnim(bool bIsAiming)
{
	if (!Aim_Anim || bCurrentlyAiming == bIsAiming)
	{
		return;
	}

	if (bIsAnimating)
	{
		StopAnimation(Aim_Anim);
	}

	bCurrentlyAiming = bIsAiming;
	bIsAnimating = true;

	if (bIsAiming)
	{
		PlayAnimationForward(Aim_Anim, 2.0f, false);
	}
	else
	{
		Center->SetVisibility(ESlateVisibility::Hidden);
		PlayAnimationReverse(Aim_Anim, 2.0f, false);
	}
}

void UGS_CrossHairImage::PlayHitFeedback()
{
	if (!HitFeedback_Anim)
	{
		return;
	}

	if (IsAnimationPlaying(HitFeedback_Anim))
	{
		StopAnimation(HitFeedback_Anim);
	}

	PlayAnimationForward(HitFeedback_Anim, 1.0f, false);
}

void UGS_CrossHairImage::SetCrosshairVisibility(bool bVisible)
{
	ESlateVisibility NewVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	SetVisibility(NewVisibility);
}

void UGS_CrossHairImage::UpdateArrowType(EArrowType NewType)
{
	if (!ArrowCounterSwitcher)
	{
		return;
	}

	switch (NewType)
	{
	case EArrowType::Axe:
		ArrowCounterSwitcher->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ArrowCounterSwitcher->SetActiveWidgetIndex(0);
		break;
	case EArrowType::Child:
		ArrowCounterSwitcher->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ArrowCounterSwitcher->SetActiveWidgetIndex(1);
		break;
	default:
		ArrowCounterSwitcher->SetVisibility(ESlateVisibility::Hidden);
		break;
	}
}

void UGS_CrossHairImage::UpdateArrowCnt(EArrowType ArrowType, int32 CurrCnt)
{
	switch (ArrowType)
	{
	case EArrowType::Axe:
		if (AxeArrowCounter)
		{
			AxeArrowCounter->UpdateArrowCnt(CurrCnt);
		}
		break;
	case EArrowType::Child:
		if (ChildArrowCounter)
		{
			ChildArrowCounter->UpdateArrowCnt(CurrCnt);
		}
		break;
	default:
		break;
	}
}

void UGS_CrossHairImage::InitArrowCounters()
{
	if (AxeArrowCounter)
	{
		AxeArrowCounter->Init(EArrowType::Axe);
	}

	if (ChildArrowCounter)
	{
		ChildArrowCounter->Init(EArrowType::Child);
	}
}

void UGS_CrossHairImage::OnAimAnimFinished()
{
	bIsAnimating = false;
	if(Center)
	{
		if (bCurrentlyAiming)
		{
			Center->SetVisibility(ESlateVisibility::Visible);
		}
	}
}