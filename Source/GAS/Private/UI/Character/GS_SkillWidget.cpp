#include "UI/Character/GS_SkillWidget.h"
#include "Character/Player/GS_Player.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UGS_SkillWidget::UGS_SkillWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGS_SkillWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	OwningCharacter = Cast<AGS_Player>(GetOwningPlayer()->GetPawn());
	SetOwningActor(OwningCharacter);

	if (IsValid(OwningCharacter))
	{
		OwningCharacter->GetSkillComp()->InitializeSkillWidget(this);
	}
}

void UGS_SkillWidget::Initialize(UGS_SkillBase* Skill)
{
	if (Skill)
	{
		OnSkillCoolTimeChanged(SkillSlot, 0.f);
		CurrentCoolTimeText->SetVisibility(ESlateVisibility::Hidden);
		SkillImage->SetBrushFromTexture(Skill->GetSkillImage());
	}
}

void UGS_SkillWidget::OnSkillActivated(ESkillSlot InSkillSlot)
{
	if (InSkillSlot == SkillSlot)
	{
		PlayHeartbeatAnimation();
		PlayGlowEffect();
		PlaySkillActivationSound();
	}
}

void UGS_SkillWidget::OnSkillCoolTimeChanged(ESkillSlot InSkillSlot, float InCurrentCoolTime) const
{
	if (InSkillSlot != SkillSlot)
	{
		return;
	}
	
	float CoolTime = OwningCharacter->GetSkillComp()->GetSkillFromSkillMap(SkillSlot)->GetCoolTime();
	
	//finish skill
	if (InCurrentCoolTime < KINDA_SMALL_NUMBER)
	{
		CurrentCoolTimeText->SetVisibility(ESlateVisibility::Hidden);
		CoolTimeBar->SetVisibility(ESlateVisibility::Hidden);
	}
	//start skill
	else
	{
		CurrentCoolTimeText->SetVisibility(ESlateVisibility::Visible);
		CoolTimeBar->SetVisibility(ESlateVisibility::Visible);
		CurrentCoolTimeText->SetText(FText::FromString(FString::Printf(TEXT("%d"),FMath::RoundToInt(InCurrentCoolTime))));
		CoolTimeBar->SetPercent(InCurrentCoolTime/CoolTime);
	}
}

void UGS_SkillWidget::PlaySkillActivationSound()
{
	if (bEnableAudio && SkillActivationSound)
	{
		UGameplayStatics::PlaySound2D(this, SkillActivationSound, AudioVolume);
	}
}

void UGS_SkillWidget::OnSkillCooldownBlocked(ESkillSlot InSkillSlot)
{
	if (InSkillSlot == SkillSlot)
	{
		PlayCooldownBlockedAnimation();
		PlayRedFlashEffect();
		PlaySkillCooldownSound();
	}
}

void UGS_SkillWidget::PlaySkillCooldownSound()
{
	if (bEnableAudio && SkillCooldownSound)
	{
		UGameplayStatics::PlaySound2D(this, SkillCooldownSound, AudioVolume);
	}
}