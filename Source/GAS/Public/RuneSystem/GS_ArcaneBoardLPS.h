// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GS_ArcaneBoardTypes.h"
#include "System/GS_PlayerRole.h"
#include "System/GS_PlayerState.h"
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
    ECharacterClass GetPlayerCharacterClass() const;

    UFUNCTION()
    void OnPlayerJobChanged(ESeekerJob SeekerJob);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void InitializeRunes();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void RefreshBoardForCurrentCharacter();

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
    bool HasUnsavedChanges() const;

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    UGS_ArcaneBoardManager* GetOrCreateBoardManager();

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ForceApplyChanges();

    // 위젯 등록/해제 함수 추가
    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void SetCurrUIWidget(UGS_ArcaneBoardWidget* Widget);

    UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
    void ClearCurrUIWidget();

    //ENUM 통일 전 임시
    ECharacterClass MapSeekerJobToCharacterClass(ESeekerJob SeekerJob) const;

private:
    UPROPERTY()
    TWeakObjectPtr<AGS_PlayerState> BoundPlayerState;

    UPROPERTY()
    TWeakObjectPtr<UGS_ArcaneBoardWidget> CurrentUIWidget;
};
