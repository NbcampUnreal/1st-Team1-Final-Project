// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Character/GS_ArrowIndicator.h"
#include "GS_ArrowCounter.generated.h"

class UHorizontalBox;

/**
 * 특수 화살 개수를 시각적으로 나타내는 위젯
 */
UCLASS()
class GAS_API UGS_ArrowCounter : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* ArrowContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
	TSubclassOf<UGS_ArrowIndicator> ArrowIndicatorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TObjectPtr<UTexture2D> AxeArrowTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TObjectPtr<UTexture2D> ChildArrowTexture;

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void Init(EArrowType InArrowType);

	UFUNCTION(BlueprintCallable , Category = "Arrow")
	void CreateIndicators();

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void UpdateArrowCnt(uint8 CurrCnt);

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void ClearIndicators();

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TArray<UGS_ArrowIndicator*> ArrowIndicators;

	UPROPERTY()
	EArrowType ArrowType;

	UPROPERTY()
	uint8 MaxArrowCnt;

	UTexture2D* GetTextureForArrowType() const;
};
