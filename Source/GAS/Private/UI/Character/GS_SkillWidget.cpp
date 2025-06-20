#include "UI/Character/GS_SkillWidget.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UGS_SkillWidget::UGS_SkillWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGS_SkillWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	AGS_Character* OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
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
		OnSkillCoolTimeChanged(0.f);
		CurrentCoolTimeText->SetVisibility(ESlateVisibility::Hidden);
		SkillImage->SetBrushFromTexture(Skill->GetSkillImage());
	}
}

void UGS_SkillWidget::OnSkillCoolTimeChanged(float InCurrentCoolTime) const
{
	float CoolTime = OwningCharcter->GetSkillComp()->GetSkillFromSkillMap(SkillSlot)->GetCoolTime();

	//finish skill
	if (CoolTime - InCurrentCoolTime < KINDA_SMALL_NUMBER)
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