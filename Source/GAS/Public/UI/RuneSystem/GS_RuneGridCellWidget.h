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
    Hover       UMETA(DisplayName = "Hover"),
    Valid       UMETA(DisplayName = "Valid"),
    Invalid     UMETA(DisplayName = "Invalid"),
    Special     UMETA(DisplayName = "Special"),
    Occupied    UMETA(DisplayName = "Occupied")
};

class UImage;
class UButton;
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

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SetCellData(const FGridCellData& InCellData);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    FIntPoint GetCellPos() const;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SetVisualState(EGridCellVisualState NewState);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SetRuneTexture(UTexture2D* Texture);

    UFUNCTION(BlueprintImplementableEvent, Category = "ArcaneBoard")
    void OnVisualStateChanged(EGridCellVisualState NewState);

protected:
    UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
    FGridCellData CellData;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UButton* DropZone;

    UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
    EGridCellVisualState VisualState;

    UPROPERTY()
    UGS_ArcaneBoardWidget* ParentBoardWidget;
};
