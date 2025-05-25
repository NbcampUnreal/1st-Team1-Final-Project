#include "UI/Character/GS_HPWidget.h"

#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/ProgressBar.h"

UGS_HPWidget::UGS_HPWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

void UGS_HPWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());

	if (IsValid(OwningCharacter))
	{
		OwningCharacter->SetHPBarWidget(this);
	}
}

void UGS_HPWidget::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void UGS_HPWidget::InitializeHPWidget(UGS_StatComp* InStatComp)
{
	OnCurrentHPBarChanged(InStatComp);
}

void UGS_HPWidget::OnCurrentHPBarChanged(UGS_StatComp* InStatComp)
{
 	HPBarWidget->SetPercent(InStatComp->GetCurrentHealth()/InStatComp->GetMaxHealth());
}