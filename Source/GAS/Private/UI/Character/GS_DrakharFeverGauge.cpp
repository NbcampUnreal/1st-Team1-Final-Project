#include "UI/Character/GS_DrakharFeverGauge.h"

#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Components/ProgressBar.h"

UGS_DrakharFeverGauge::UGS_DrakharFeverGauge(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGS_DrakharFeverGauge::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsValid(Drakhar))
	{
		Drakhar = Cast<AGS_Drakhar>(GetOwningPlayer()->GetPawn());
	}
	
	if (IsValid(Drakhar))
	{
		Drakhar->SetFeverGaugeWidget(this);
	}
}

void UGS_DrakharFeverGauge::InitializeGauge(float InCurrentGauge)
{
	OnCurrentFeverGaugeChanged(InCurrentGauge);
}

void UGS_DrakharFeverGauge::OnCurrentFeverGaugeChanged(float InCurrentGauge)
{
	DrakharFeverGauge->SetPercent(InCurrentGauge/Drakhar->GetMaxFeverGauge());
	
}

