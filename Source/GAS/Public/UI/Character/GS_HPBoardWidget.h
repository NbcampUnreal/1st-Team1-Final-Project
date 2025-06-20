#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HPBoardWidget.generated.h"

class UGS_HPWidget;
class UVerticalBox;
class AGS_Character;

UCLASS()
class GAS_API UGS_HPBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void InitBoardWidget();

	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> HPWidgetList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_HPWidget> HPWidgetClass;

	UPROPERTY()
	TObjectPtr<AGS_Character> OwningCharacter;
};
