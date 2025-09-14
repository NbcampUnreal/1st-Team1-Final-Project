#include "UI/RTS/GS_AetherWidget.h"
#include "AI/RTS/GS_RTSController.h"


void UGS_AetherWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AGS_RTSController* RTSController = Cast<AGS_RTSController>(GetOwningPlayer());
	if (!RTSController)
	{
		UE_LOG(LogTemp, Error, TEXT("[AetherWidget] no RTS Controller"));
		return;
	}

	UGS_AetherComp* AetherComp = RTSController->GetAetherComp();
	if (!AetherComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[AetherWidget] no AetherComp"));
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("[AetherWidget]successfully added dynamic to UpdateAetherAmount"));
	AetherComp->OnAetherChanged.AddDynamic(this, &UGS_AetherWidget::UpdateAetherAmount);

	UpdateAetherAmount(AetherComp->GetCurrentAmount());
}

void UGS_AetherWidget::UpdateAetherAmount(float CurrentAmount)
{
	//UE_LOG(LogTemp, Warning, TEXT("[AetherWidget]UpdateAetherAmount"));
	FString TextValue = FString::Printf(TEXT("Aether : %.0f"), CurrentAmount);
	AetherText->SetText(FText::FromString(TextValue));
}

void UGS_AetherWidget::NativeDestruct()
{
	Super::NativeDestruct();

	AGS_RTSController* RTS_Controller = Cast<AGS_RTSController>(GetOwningPlayer());
	if (RTS_Controller)
	{
		UGS_AetherComp* AetherComp = RTS_Controller->GetAetherComp();
		if (AetherComp)
		{
			AetherComp->OnAetherChanged.RemoveDynamic(this, &UGS_AetherWidget::UpdateAetherAmount);
		}
	}
}