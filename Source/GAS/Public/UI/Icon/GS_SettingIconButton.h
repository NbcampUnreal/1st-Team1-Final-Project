#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_SettingIconButton.generated.h"

class UGS_OptionMenuUI;

UCLASS()
class GAS_API UGS_SettingIconButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SettingButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_OptionMenuUI> OptionMenuUIClass;
	UPROPERTY()
	UGS_OptionMenuUI* OptionMenuUI;

	UFUNCTION()
	void OnSettingButtonClicked();
	
	void OptionMenuAddressClear();
	
protected:
	virtual void NativeConstruct() override;
};
