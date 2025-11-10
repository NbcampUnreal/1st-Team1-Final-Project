// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GS_ManualRow.generated.h"

USTRUCT(BlueprintType)
struct GAS_API FManualImageRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual")
    uint8 PageIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual")
    TSoftObjectPtr<UTexture2D> ManualImage;
};