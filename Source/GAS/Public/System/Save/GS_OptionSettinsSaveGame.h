// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GS_OptionSettinsSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_OptionSettinsSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UGS_OptionSettinsSaveGame();

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MouseSensitivity;
};
