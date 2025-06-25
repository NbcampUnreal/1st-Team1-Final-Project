#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DungeonEditorWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class GAS_API UGS_DungeonEditorWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> LoadButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> SaveButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ResetButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> BackButton;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSaveButtonClicked();
	UFUNCTION()
	void OnLoadButtonClicked();
	UFUNCTION()
	void OnBackButtonClicked();
};
