#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_PropsSelector.generated.h"

struct FGS_PlaceableObjectsRow;
class UGS_BuildingIconSlotWidget;
class UUniformGridPanel;
class UTextBlock;

UCLASS()
class GAS_API UGS_PropsSelector : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GridName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridSlot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_BuildingIconSlotWidget> IconSlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int SlotCount;

	void SetTitle(FText InText);
	void CreateIconSlot(const FDataTableRowHandle& InData);
	
protected:
	virtual void NativeConstruct() override;

private:
	int IconsAmount;
};
