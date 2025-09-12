#include "UI/Character/GS_CommandWidget.h"
#include "Character/Player/GS_Player.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

UGS_CommandWidget::UGS_CommandWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGS_CommandWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_CommandWidget::SetKeyCommand(UTexture2D* IconTexture, const FString& KeyText)
{
	// 아이콘 설정
	if (IsValid(SkillIcon) && IconTexture)
	{
		SkillIcon->SetBrushFromTexture(IconTexture);
	}
} 