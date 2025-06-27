// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GS_TutorialRow.generated.h"

USTRUCT(BlueprintType)
struct GAS_API FTutorialImageRow : public FTableRowBase
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    uint8 PageIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TSoftObjectPtr<UTexture2D> TutorialImage;

    FTutorialImageRow()
    {
        PageIndex = 0;
        Title = FText::GetEmpty();
        TutorialImage = nullptr;
    }
};
