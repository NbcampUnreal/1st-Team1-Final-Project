#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "System/GS_PlayerRole.h"
#include "Sound/SoundBase.h"
#include "GS_CustomLobbyUI.generated.h"

class UGS_CommonTwoBtnPopup;
class UButton;
class UTextBlock;
class UCanvasPanel;
class UOverlay;
class UGS_FriendListWidget;

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
	UUserWidget* BackButton;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* FriendListButton;
	UPROPERTY(meta = (BindWidget))
	UOverlay* ModalOverlay;
	UPROPERTY(meta = (BindWidget))
	UOverlay* FriendListOverlay;

	UPROPERTY(meta = (BindWidget))
	UGS_CommonTwoBtnPopup* CommonPopUpUI;

	UPROPERTY(EditDefaultsOnly, Category = "Friend")
	TSubclassOf<UGS_FriendListWidget> FriendListWidgetClass;
	UPROPERTY()
	TObjectPtr<UGS_FriendListWidget> FriendListWidgetInstance;

	UFUNCTION()
	void OnJobSelectionButtonClicked();
	UFUNCTION()
	void OnPerkOrDungeonButtonClicked();
	UFUNCTION()
	void OnReadyButtonClicked();
	UFUNCTION()
	void OnRoleChangeButtonClicked();
	UFUNCTION()
	void OnBackButtonClicked();
	UFUNCTION()
	void OnFriendListButtonClicked();

	void UpdateRoleSpecificText(EPlayerRole NewRole);
	void UpdateReadyButtonText(bool bIsReady);

	UOverlay* GetModalOverlay() const { return ModalOverlay; }

	void OnBackPopupYesButtonClicked();
	void OnBackPopupNoButtonClicked();

	//아케인보드 저장 관련 함수
	void ShowPerkSaveConfirmPopup();
	
	// 캐릭터별 준비 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Ready")
	TObjectPtr<USoundBase> AresReadySound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Ready")
	TObjectPtr<USoundBase> ChanReadySound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Ready")
	TObjectPtr<USoundBase> MerciReadySound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Ready")
	TObjectPtr<USoundBase> ReinaReadySound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Guardian|Ready")
	TObjectPtr<USoundBase> DrakharReadySound;
	
	// 캐릭터별 취소 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Cancel")
	TObjectPtr<USoundBase> AresCancelSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Cancel")
	TObjectPtr<USoundBase> ChanCancelSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Cancel")
	TObjectPtr<USoundBase> MerciCancelSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Seeker|Cancel")
	TObjectPtr<USoundBase> ReinaCancelSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Guardian|Cancel")
	TObjectPtr<USoundBase> DrakharCancelSound;
	
private:
	void ChangeRoleBtnIcon(EPlayerRole NewRole);
	void PlayCharacterReadySound();
};
