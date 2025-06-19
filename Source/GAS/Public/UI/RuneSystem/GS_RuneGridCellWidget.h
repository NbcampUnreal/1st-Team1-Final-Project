// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RuneSystem/GS_ArcaneBoardTypes.h"
#include "GS_RuneGridCellWidget.generated.h"

UENUM(BlueprintType)
enum class EGridCellVisualState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Valid       UMETA(DisplayName = "Valid"),
    Invalid     UMETA(DisplayName = "Invalid")
};

class UImage;
class UGS_ArcaneBoardWidget;

/**
 * 룬 그리드 셀 위젯
 */
UCLASS()
class GAS_API UGS_RuneGridCellWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UGS_RuneGridCellWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void InitCell(const FGridCellData& InCellData, UGS_ArcaneBoardWidget* InParentBoard);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SetCellData(const FGridCellData& InCellData);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    FIntPoint GetCellPos() const;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    uint8 GetPlacedRuneID() const;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SetPreviewVisualState(EGridCellVisualState NewState);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SetRuneTexture(UTexture2D* Texture);

protected:
    UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
    FGridCellData CellData;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UImage* RuneImage;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UImage* CellBG;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UImage* PreviewImage;

    UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
    EGridCellVisualState VisualState;

    UPROPERTY()
    UGS_ArcaneBoardWidget* ParentBoardWidget;
};
