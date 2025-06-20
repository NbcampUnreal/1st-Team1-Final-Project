// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_CrossHairImage.h"
#include "Character/GS_Character.h"
#include "Components/Image.h"

void UGS_CrossHairImage::NativeConstruct()
{
	Super::NativeConstruct();
	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	
	bCurrentlyAiming = false;
	bIsAnimating = false;
	Center->SetVisibility(ESlateVisibility::Hidden);
	SetCrosshairVisibility(true);
}

void UGS_CrossHairImage::NativeOnInitialized()
{
	if (Aim_Anim)
	{
		AimAnimEndDelegate.Clear();
		AimAnimEndDelegate.BindDynamic(this, &UGS_CrossHairImage::OnAimAnimFinished);
		BindToAnimationFinished(Aim_Anim, AimAnimEndDelegate);
	}

	if (HitFeedback_Anim)
	{
		HitFeedbackAnimEndDelegate.Clear();
		HitFeedbackAnimEndDelegate.BindDynamic(this, &UGS_CrossHairImage::OnHitFeedbackAnimFinished);
		BindToAnimationFinished(HitFeedback_Anim, HitFeedbackAnimEndDelegate);
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

void UGS_CrossHairImage::OnHitFeedbackAnimFinished()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Hit Feedback Completed"));
	}
}