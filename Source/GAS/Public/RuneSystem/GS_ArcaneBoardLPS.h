// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_ArcaneBoardLPS.generated.h"

class UGS_ArcaneBoardManager;
class UGS_ArcaneBoardWidget;

/**
 * 룬 시스템을 관리하는 로컬 플레이어 서브 시스템
 */
UCLASS()
class GAS_API UGS_ArcaneBoardLPS : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
    void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY()
    UGS_ArcaneBoardManager* BoardManager;

    UPROPERTY()
    FGS_StatRow RuneSystemStats;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ShowArcaneBoardUI(UWidget* ParentWidget = nullptr);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    UGS_ArcaneBoardWidget* CreateArcaneBoardWidget();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void DestroyArcaneBoardWidget();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    bool TryCloseArcaneBoardUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    ECharacterClass GetCurrPlayerClass() const;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void UpdateStatsUI();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ApplyBoardChanges();

    UFUNCTION()
    void OnBoardStatsChanged(const FGS_StatRow& NewStats);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SaveBoardConfig();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void LoadBoardConfig();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void UpdateCharacterStats();

private:
    UPROPERTY()
    UGS_ArcaneBoardWidget* CurrBoardWidget;

    UPROPERTY()
    TSubclassOf<UGS_ArcaneBoardWidget> ArcaneBoardWidgetClass;

    void RequestServerStatsUpdate();
};
