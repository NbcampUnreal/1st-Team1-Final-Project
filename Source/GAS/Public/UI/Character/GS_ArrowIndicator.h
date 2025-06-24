// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_ArrowIndicator.generated.h"

class UImage;

/**
 * 특수 화살 개수 표시 아이콘
 */
UCLASS()
class GAS_API UGS_ArrowIndicator : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGS_ArrowIndicator(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Indicator;

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void SetActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void SetArrowTexture(UTexture2D* NewTexture);

protected:
	virtual void NativeConstruct() override;

private:
	bool bIsActive;
	
	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
	float ActiveOpacity;

	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
	float InactiveOpacity;
};
