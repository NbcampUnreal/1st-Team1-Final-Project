#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_BuildingIconSlotWidget.generated.h"

class UButton;

UCLASS()
class GAS_API UGS_BuildingIconSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TempBtn;

	UFUNCTION()
	void PressedTempBtn();

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Property|Data")
	FDataTableRowHandle Data;

protected:
	virtual void NativeConstruct() override;
};
