// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_ArcaneBoardLPS.generated.h"

class UGS_ArcaneBoardManager;

/**
 * 
 */
UCLASS()
class GAS_API UGS_ArcaneBoardLPS : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

    UPROPERTY()
    UGS_ArcaneBoardManager* BoardManager;

    UPROPERTY()
    TMap<FName, float> CalculatedStats;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ShowArcaneBoardUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void HideArcaneBoardUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ApplyBoardChanges();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SaveBoardConfig();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void LoadPerkConfig();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void UpdateCharacterStats();

private:
    UPROPERTY()
    TSubclassOf<UUserWidget> ArcaneBoardWidgetClass;

    void RequestServerStatsUpdate();
};
