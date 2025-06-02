#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_ExitPopup.generated.h"

UCLASS()
class GAS_API UGS_ExitPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UUserWidget* YesButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* NoButton;

	UFUNCTION()
	void OnYesButtonClicked();
	UFUNCTION()
	void OnNoButtonClicked();

protected:
	virtual void NativeConstruct() override;
};
