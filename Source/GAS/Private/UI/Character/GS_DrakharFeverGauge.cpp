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
		Drakhar->SetFeverGageWidget(this);
	}
}

void UGS_DrakharFeverGauge::InitializeGage(float InCurrentGage)
{
	OnCurrentFeverGageChanged(InCurrentGage);
}

void UGS_DrakharFeverGauge::OnCurrentFeverGageChanged(float InCurrentGage)
{
	DrakharFeverGauge->SetPercent(InCurrentGage/Drakhar->GetMaxFeverGage());
}

