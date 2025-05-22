#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_SelectBundleWidget.generated.h"

enum class EObjectType : uint8;
class UGS_PropsSelector;
class UScrollBox;
class UTextBlock;

UCLASS()
class GAS_API UGS_SelectBundleWidget : public UUserWidget
{
	GENERATED_BODY()

public:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BandleTilte;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_PropsSelector> PropsSelectorWidgetClass;

	void SetTitle(FText InTitle);
	void CreateSelector(FDataTableRowHandle& InData);
	void InitWidget(EObjectType InObjectType);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

private:
	TArray<UGS_PropsSelector*> PropsSelectorWidgets;
};
