#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_HPBoardWidget.generated.h"

class UGS_PlayerInfoWidget;
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
	//[TODO] fix variable name
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerInfoWidgetList;

	//[New - 6/19]
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_PlayerInfoWidget> PlayerInfoWidgetClass;

	//[Legacy]
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TSubclassOf<UGS_HPWidget> HPWidgetClass;

	UPROPERTY()
	TObjectPtr<AGS_Character> OwningCharacter;
};
