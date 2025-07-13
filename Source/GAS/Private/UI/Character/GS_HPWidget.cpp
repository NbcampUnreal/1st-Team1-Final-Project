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

	if (!IsValid(OwningCharacter))
	{
		OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	}
	
	if (IsValid(OwningCharacter))
	{
		OwningCharacter->SetHPBarWidget(this);
	}
}

void UGS_HPWidget::InitializeHPWidget(UGS_StatComp* InStatComp)
{
	HPBarWidget->SetPercent(CurrentHPPercent);
	HPDelayBarWidget->SetPercent(CurrentHPPercent);
	HPSlider->SetValue(CurrentHPPercent);
	OnCurrentHPBarChanged(InStatComp);

}

void UGS_HPWidget::OnCurrentHPBarChanged(UGS_StatComp* InStatComp)
{
	TargetHPPercent = InStatComp->GetCurrentHealth() / InStatComp->GetMaxHealth();
	GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(DelayBeforInterpTimerHandle);
	
	if (TargetHPPercent < CurrentHPPercent)
	{
		//데미지
		CurrentHPPercent = TargetHPPercent; 
		HPBarWidget->SetPercent(TargetHPPercent); 
		HPSlider->SetValue(TargetHPPercent); 

	}
	else
	{
		//힐
		DelayedHPPercent = TargetHPPercent;

		HPDelayBarWidget->SetPercent(DelayedHPPercent);
		HPSlider->SetValue(DelayedHPPercent);
		CurrentHPPercent = HPBarWidget->GetPercent();
	}
	GetWorld()->GetTimerManager().SetTimer(DelayBeforInterpTimerHandle, this, &UGS_HPWidget::StartDelayBarInterp, 0.1f, false);
}

void UGS_HPWidget::StartDelayBarInterp()
{


	if (DelayedHPPercent > TargetHPPercent)
	{
		//데미지
		const float Duration = 0.05f;
		float Delta = FMath::Abs(DelayedHPPercent - TargetHPPercent);
		InterpSpeed = FMath::Max(Delta / Duration, 0.01f);
		GetWorld()->GetTimerManager().SetTimer(InterpTimerHandle, this, &UGS_HPWidget::UpdateDelayedHP, 0.01f, true);
	}
	else
	{
		//힐
		const float Duration = 0.15f;
		float Delta = FMath::Abs(TargetHPPercent - CurrentHPPercent);
		InterpSpeed = FMath::Max(Delta / Duration, 0.5f);
		GetWorld()->GetTimerManager().SetTimer(InterpTimerHandle, this, &UGS_HPWidget::UpdateMainHP, 0.01f, true);
	}
}

void UGS_HPWidget::UpdateDelayedHP()
{


	if (DelayedHPPercent > TargetHPPercent)
	{
		DelayedHPPercent = FMath::FInterpTo(DelayedHPPercent, TargetHPPercent, 0.01f, InterpSpeed);
		HPDelayBarWidget->SetPercent(DelayedHPPercent);
	}
	else
	{

		DelayedHPPercent = TargetHPPercent;
		HPDelayBarWidget->SetPercent(DelayedHPPercent);
		GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
	}
}

void UGS_HPWidget::UpdateMainHP()
{
	if (CurrentHPPercent < TargetHPPercent)
	{
		CurrentHPPercent = FMath::FInterpTo(CurrentHPPercent, TargetHPPercent, 0.01f, InterpSpeed);
		HPBarWidget->SetPercent(CurrentHPPercent);
	}
	else
	{
		CurrentHPPercent = TargetHPPercent;
		HPBarWidget->SetPercent(CurrentHPPercent);
		GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
	}
}