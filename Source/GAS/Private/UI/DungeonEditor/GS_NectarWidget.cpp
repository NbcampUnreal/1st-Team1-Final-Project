#include "UI/DungeonEditor/GS_NectarWidget.h"

void UGS_NectarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AGS_BuildManager* BuildManager = Cast<AGS_BuildManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGS_BuildManager::StaticClass()));
	if (!BuildManager) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find BuildManager"));
		return;
	}

	UGS_NectarComp* NectarComp = BuildManager->FindComponentByClass<UGS_NectarComp>();
	if (!NectarComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find NectarComp"));
		return;
	}

	NectarComp->OnNectarChanged.AddDynamic(this, &UGS_NectarWidget::UpdateNectarAmount);
	UpdateNectarAmount(NectarComp->GetCurrentAmount());
}

void UGS_NectarWidget::UpdateNectarAmount(float CurrentAmount)
{
	FString TextValue = FString::Printf(TEXT("Nectar : %.0f"), CurrentAmount);
	NectarText->SetText(FText::FromString(TextValue));
}
