#include "UI/Character/GS_HPText.h"

#include "Character/Component/GS_StatComp.h"
#include "Character/GS_Character.h"
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
}

void UGS_HPText::InitializeHPTextWidget(UGS_StatComp* InStatComp)
{
	OnCurrentHPChanged(InStatComp);
}

void UGS_HPText::OnCurrentHPChanged(UGS_StatComp* InStatComp)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HP")), true, true, FLinearColor::Green, 5.f);
	CurrentHPText->SetText(FText::FromString(FString::Printf(TEXT("%d"),FMath::RoundToInt(InStatComp->GetCurrentHealth()))));
	HPBar->SetPercent(InStatComp->GetCurrentHealth()/InStatComp->GetMaxHealth());
}
