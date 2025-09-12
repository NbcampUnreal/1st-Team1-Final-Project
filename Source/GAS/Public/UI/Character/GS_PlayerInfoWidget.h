// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_PlayerInfoWidget.generated.h"


class UImage;
class UTextBlock;
class UProgressBar;
class AGS_Player;
class UGS_PawnMappingDataAsset;

UCLASS()
class GAS_API UGS_PlayerInfoWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void InitializePlayerInfoWidget(AGS_Player* InPlayer);
	
	void SetOwningActor(AGS_Player* InOwningCharacter) { OwningCharacter = InOwningCharacter; }
	
	UFUNCTION()
	void OnCurrentHPBarChanged(UGS_StatComp* InStatComp);
	
protected:
	//seeker class
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr <UImage> PlayerClass;

	//steam name
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr <UTextBlock> PlayerName;

	//show health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr <UProgressBar> HPBarWidget; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Player> OwningCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TObjectPtr<UGS_PawnMappingDataAsset> PawnMappingData;
};
