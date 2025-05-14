// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_ArcaneBoardLPS.generated.h"

class UGS_ArcaneBoardManager;

/**
 * 룬 시스템을 관리하는 로컬 플레이어 서브 시스템
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
    FCharacterStats RuneSystemStats;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ShowArcaneBoardUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void HideArcaneBoardUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    bool TryCloseArcaneBoardUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void UpdateStatsUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ApplyBoardChanges();

    UFUNCTION()
    void OnBoardStatsChanged(const FCharacterStats& NewStats);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SaveBoardConfig();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void LoadBoardConfig();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void UpdateCharacterStats();

private:
    UPROPERTY()
    UUserWidget* CurrentBoardWidget;

    UPROPERTY()
    TSubclassOf<UUserWidget> ArcaneBoardWidgetClass;

    void RequestServerStatsUpdate();
};
