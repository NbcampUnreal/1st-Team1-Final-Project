#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_CharacterSelectList.generated.h"

enum class EPlayerRole : uint8;
class UHorizontalBox;
class UCustomCommonButton;
class UScrollBox;

UCLASS()
class GAS_API UGS_CharacterSelectList : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> CharacterSlotList;
	UPROPERTY()
	TArray<TObjectPtr<UHorizontalBox>> HorizontalBoxes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCustomCommonButton> ButtonSlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int SlotCount;

	void CreateChildWidgets(EPlayerRole PlayerRole);
	
protected:
	virtual void NativeConstruct() override;

private:
	void AddSpacerInHorizeontalBox();
	void AddSpacerInVerticalBox();
};
