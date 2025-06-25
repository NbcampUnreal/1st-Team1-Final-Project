#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sound/SoundBase.h"
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
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TObjectPtr<UDataTable> CharacterInfoDataTable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int SlotCount;

	// === 캐릭터 선택 사운드들 ===
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Character Select", meta = (DisplayName = "아레스 선택 사운드"))
	USoundBase* AresSelectSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Character Select", meta = (DisplayName = "메르시 선택 사운드"))
	USoundBase* MerciSelectSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Character Select", meta = (DisplayName = "찬 선택 사운드"))
	USoundBase* ChanSelectSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Character Select", meta = (DisplayName = "레이나 선택 사운드"))
	USoundBase* ReinaSelectSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Character Select", meta = (DisplayName = "드라카 선택 사운드"))
	USoundBase* DrakharSelectSound;

	void CreateChildWidgets(EPlayerRole PlayerRole);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void AddSpacerInHorizeontalBox();
	void AddSpacerInVerticalBox();

	void OnCharacterSelectClicked(int32 CharacterID, EPlayerRole PlayerRole);

	// === 캐릭터 선택 사운드 재생 함수 ===
	void PlayCharacterSelectSound(int32 CharacterID, EPlayerRole PlayerRole);

	TArray<TObjectPtr<UCustomCommonButton>> ButtonRefs;

	// === 중복 선택 방지를 위한 변수 ===
	int32 LastSelectedCharacterID = -1;
	EPlayerRole LastSelectedPlayerRole = static_cast<EPlayerRole>(0);
};
