#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "System/GS_PlayerRole.h"
#include "GS_CustomLobbyUI.generated.h"

class UButton;
class UTextBlock;
class UCanvasPanel;
class UOverlay;

UCLASS()
class GAS_API UGS_CustomLobbyUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UUserWidget* JobSelectionButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* PerkOrDungeonButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* ReadyButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* RoleChangeButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PerkDungeonText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyText;
	UPROPERTY(meta = (BindWidgetOptional))
	UOverlay* ModalOverlay;

	UFUNCTION()
	void OnJobSelectionButtonClicked();
	UFUNCTION()
	void OnPerkOrDungeonButtonClicked();
	UFUNCTION()
	void OnReadyButtonClicked();
	UFUNCTION()
	void OnRoleChangeButtonClicked();

	void UpdateRoleSpecificText(EPlayerRole NewRole);
	void UpdateReadyButtonText(bool bIsReady);

	UOverlay* GetModalOverlay() const { return ModalOverlay; }

private:
	void ChangeRoleBtnIcon(EPlayerRole NewRole);
};
