#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DungeonEditor/Data/GS_DungeonEditorTypes.h"
#include "GS_PropWidget.generated.h"

class UGS_SelectBundleWidget;
class UWidgetSwitcher;
class UButton;
class UTextBlock;

UCLASS()
class GAS_API UGS_PropWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SwitcherButton0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SwitcherTitle0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SwitcherButton1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SwitcherTitle1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SwitcherButton2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SwitcherTitle2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SwitcherButton3;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SwitcherTitle3;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> BundleSwitcher;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_SelectBundleWidget> SelectorBundleWidgetClass;

	UFUNCTION()
	void PressedSwitcherBtn0();
	UFUNCTION()
	void PressedSwitcherBtn1();
	UFUNCTION()
	void PressedSwitcherBtn2();
	UFUNCTION()
	void PressedSwitcherBtn3();

	void SwitchBundle(EObjectType InObjectType);

protected:
	virtual void NativeConstruct() override;
	
private:
	TArray<UGS_SelectBundleWidget*> SelectorBundleWidgets;

	void InitializeBundleSwitcher();
	void OpenButton(EObjectType InObjectType);
	TObjectPtr<UButton> PrevClickSwitcherBtn;

	void ChangeButtonSize(TObjectPtr<UButton> InSwitcherBtn, FVector2d InSize);
};
