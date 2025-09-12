#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "DungeonEditor/GS_BuildManager.h"
#include "GS_NectarWidget.generated.h"

class UTextBlock;
UCLASS()
class GAS_API UGS_NectarWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NectarText;

	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	void UpdateNectarAmount(float CurrentAmount);


};
