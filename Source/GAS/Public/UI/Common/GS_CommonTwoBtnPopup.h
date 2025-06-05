#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_CommonTwoBtnPopup.generated.h"

class UTextBlock;
DECLARE_DELEGATE(FOnYesClickedDelegate);
DECLARE_DELEGATE(FOnNoClickedDelegate);

UCLASS()
class GAS_API UGS_CommonTwoBtnPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* YesButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* NoButton;

	FOnYesClickedDelegate OnYesClicked;
	FOnNoClickedDelegate OnNoClicked;

	void SetDescription(const FText& Description);
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnYesButtonClicked();
	UFUNCTION()
	void OnNoButtonClicked();
};
