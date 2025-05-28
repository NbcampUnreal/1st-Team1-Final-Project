#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "System/GS_PlayerRole.h"
#include "GS_CustomLobbyUI.generated.h"

class UButton;
class UTextBlock;
class UCanvasPanel;

UCLASS()
class GAS_API UGS_CustomLobbyUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UButton* JobSelectionButton;
	UPROPERTY(meta = (BindWidget))
	UButton* PerkOrDungeonButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;
	UPROPERTY(meta = (BindWidget))
	UButton* RoleChangeButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PerkDungeonText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyText;
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* ModalCanvasPanel;

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

	UCanvasPanel* GetModalCanvasPanel() const { return ModalCanvasPanel; }
};
