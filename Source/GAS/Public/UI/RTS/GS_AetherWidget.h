#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GS_AetherWidget.generated.h"

UCLASS()
class GAS_API UGS_AetherWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AetherText;

	UFUNCTION()
	void UpdateAetherAmount(float CurrentAmount); 
};