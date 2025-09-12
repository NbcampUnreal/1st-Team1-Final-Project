// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GS_TutorialRow.generated.h"

class UMediaTexture;
class UMediaSource;
class UMediaPlayer;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bUseMedia;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TSoftObjectPtr<UMediaTexture> TutorialMediaTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TSoftObjectPtr<UMediaPlayer> TutorialMediaPlayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TSoftObjectPtr<UMediaSource> TutorialMediaSource;

    FTutorialImageRow()
    {
        PageIndex = 0;
        Title = FText::GetEmpty();
        TutorialImage = nullptr;
    	bUseMedia = false;
    	TutorialMediaTexture = nullptr;
    	TutorialMediaPlayer = nullptr;
    	TutorialMediaSource = nullptr;
    }
};
