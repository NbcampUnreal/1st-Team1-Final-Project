#include "UI/Character/GS_HPText.h"

#include "Character/Component/GS_StatComp.h"
#include "Character/GS_Character.h"
#include "ResourceSystem/Aether/GS_AetherExtractor.h"
#include "Components/ProgressBar.h"

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
	else if (AGS_AetherExtractor* OwningExtractor = Cast<AGS_AetherExtractor>(OwningActor))
	{
		OwningExtractor->SetHPTextWidget(this);
	}
}

void UGS_HPText::InitializeHPTextWidget(UGS_StatComp* InStatComp)
{
	OnCurrentHPChanged(InStatComp);
}

void UGS_HPText::OnCurrentHPChanged(UGS_StatComp* InStatComp)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HP")), true, true, FLinearColor::Green, 5.f);
	CurrentHPText->SetText(FText::FromString(FString::Printf(TEXT("%d"),FMath::RoundToInt(InStatComp->GetCurrentHealth()))));
	UE_LOG(LogTemp, Warning, TEXT("[GS_HPText]OnCurrentHP Changed to : %f"), InStatComp->GetCurrentHealth());
	HPBar->SetPercent(InStatComp->GetCurrentHealth()/InStatComp->GetMaxHealth());
}

void UGS_HPText::SetHPBarColor(bool bIsEnemy)
{
	if (!IsValid(HPBar))
	{
		return;
	}
	
	FProgressBarStyle CurrentStyle = HPBar->GetWidgetStyle();
	if (bIsEnemy)
	{
		CurrentStyle.FillImage.SetResourceObject(RedHPBarTexture);
	}
	else
	{
		CurrentStyle.FillImage.SetResourceObject(GreenHPBarTexture);
	}
	
	HPBar->SetWidgetStyle(CurrentStyle);
}
