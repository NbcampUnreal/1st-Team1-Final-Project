#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_FeverGaugeBoard.generated.h"


class AGS_Drakhar;
class UVerticalBox;
class UGS_DrakharFeverGauge;

UCLASS()
class GAS_API UGS_FeverGaugeBoard : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void InitDrakharFeverWidget();
	
	void SetOwningDrakhar(AGS_Drakhar* InOwningCharacter) { Drakhar = InOwningCharacter; }

	bool FindDrakhar();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> FeverWidgetList;

	//Fever mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_DrakharFeverGauge> DrakharFeverWidgetClass;
	UPROPERTY()
	TObjectPtr<UGS_DrakharFeverGauge> DrakharFeverWidgetInstance;

	UPROPERTY()
	TObjectPtr<AGS_Drakhar> Drakhar;
};
