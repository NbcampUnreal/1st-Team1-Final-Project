// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RuneSystem/GS_DragVisualWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/UniformGridPanel.h"

UGS_DragVisualWidget::UGS_DragVisualWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    RuneID = 0;
    ReferenceCellPos = FIntPoint::ZeroValue;
    ReferenceCellOffset = FVector2D::ZeroVector;
    BaseCellSize = FVector2D(64.0f, 64.0f);
    CurrentScaleFactor = 1.0f;
}

void UGS_DragVisualWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UGS_DragVisualWidget::Setup(uint8 InRuneID, UTexture2D* InTexture, const TMap<FIntPoint, UTexture2D*>& RuneShape, const FVector2D& InBaseCellSize, float ScaleFactor)
{
    RuneID = InRuneID;
    CachedRuneShape = RuneShape;
    BaseCellSize = InBaseCellSize;
    CurrentScaleFactor = ScaleFactor;

    if (IsValid(RuneGridPanel) && RuneShape.Num() > 0)
    {
        FIntPoint MinPos, MaxPos;
        CalculateGridBounds(RuneShape, MinPos, MaxPos);

        FIntPoint RuneGridSize = FIntPoint(
            MaxPos.X - MinPos.X + 1,
            MaxPos.Y - MinPos.Y + 1
        );

        FVector2D TotalDragVisualSize = FVector2D(
            RuneGridSize.Y * BaseCellSize.X * ScaleFactor,
            RuneGridSize.X * BaseCellSize.Y * ScaleFactor
        );

        if (IsValid(DragVisualSizeBox))
        {
            DragVisualSizeBox->SetWidthOverride(TotalDragVisualSize.X);
            DragVisualSizeBox->SetHeightOverride(TotalDragVisualSize.Y);
        }

        CreateRuneShapeGrid(RuneShape);
        CalculateInitReferenceCellOffset();
    }
}

FVector2D UGS_DragVisualWidget::GetReferenceCellOffset() const
{
    return ReferenceCellOffset;
}

void UGS_DragVisualWidget::CreateRuneShapeGrid(const TMap<FIntPoint, UTexture2D*>& RuneShape)
{
    if (!IsValid(RuneGridPanel))
    {
        return;
    }

    RuneGridPanel->ClearChildren();
    GridCellWidgets.Empty();
    CellToGridPosMap.Empty();

    FIntPoint MinPos, MaxPos;
    CalculateGridBounds(RuneShape, MinPos, MaxPos);

    ReferenceCellPos = RuneShape.Contains(FIntPoint::ZeroValue) ? FIntPoint::ZeroValue : MinPos;

    for (const auto& ShapePair : RuneShape)
    {
        FIntPoint CellPos = ShapePair.Key;
        UTexture2D* CellTexture = ShapePair.Value;

        if (CellTexture)
        {
            UImage* CellImage = NewObject<UImage>(this);
            CellImage->SetBrushFromTexture(CellTexture);

            FIntPoint GridPos = CellPos - MinPos;
            UUniformGridSlot* ImageSlot = RuneGridPanel->AddChildToUniformGrid(CellImage, GridPos.X, GridPos.Y);
            if (ImageSlot)
            {
                ImageSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
                ImageSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
            }

            CellToGridPosMap.Add(CellPos, GridPos);
            GridCellWidgets.Add(CellPos, CellImage);
        }
    }
}

void UGS_DragVisualWidget::CalculateGridBounds(const TMap<FIntPoint, UTexture2D*>& RuneShape, FIntPoint& MinPos, FIntPoint& MaxPos)
{
    if (RuneShape.Num() == 0)
    {
        MinPos = MaxPos = FIntPoint::ZeroValue;
        return;
    }

    MinPos = FIntPoint(INT_MAX, INT_MAX);
    MaxPos = FIntPoint(INT_MIN, INT_MIN);

    for (const auto& ShapePair : RuneShape)
    {
        FIntPoint Pos = ShapePair.Key;
        MinPos.X = FMath::Min(MinPos.X, Pos.X);
        MinPos.Y = FMath::Min(MinPos.Y, Pos.Y);
        MaxPos.X = FMath::Max(MaxPos.X, Pos.X);
        MaxPos.Y = FMath::Max(MaxPos.Y, Pos.Y);
    }
}

void UGS_DragVisualWidget::CalculateInitReferenceCellOffset()
{
    if (CachedRuneShape.Contains(ReferenceCellPos) && CellToGridPosMap.Contains(ReferenceCellPos))
    {
        FIntPoint ActualGridPos = CellToGridPosMap[ReferenceCellPos];
        FVector2D ActualCellSize = BaseCellSize * CurrentScaleFactor;

        FVector2D BaseOffset = FVector2D(
            ActualGridPos.Y * ActualCellSize.X + ActualCellSize.X * 0.5f,
            ActualGridPos.X * ActualCellSize.Y + ActualCellSize.Y * 0.5f
        );

        ReferenceCellOffset = BaseOffset;
    }
    else
    {
        // 폴백
        FIntPoint MinPos, MaxPos;
        CalculateGridBounds(CachedRuneShape, MinPos, MaxPos);

        FVector2D TotalSize = FVector2D(
            (MaxPos.Y - MinPos.Y + 1) * BaseCellSize.X * CurrentScaleFactor,
            (MaxPos.X - MinPos.X + 1) * BaseCellSize.Y * CurrentScaleFactor
        );

        ReferenceCellOffset = TotalSize * 0.5f;
    }
}