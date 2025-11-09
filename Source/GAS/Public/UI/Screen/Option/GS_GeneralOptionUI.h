// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_GeneralOptionUI.generated.h"

class UStringTable;

/**
 * 
 */
UCLASS()
class GAS_API UGS_GeneralOptionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	UStringTable* LoadedTable;

	UFUNCTION(BlueprintCallable, Category = "Localization")
	FText GetLocalizedText(FString Key);
};
