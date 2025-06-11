#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GS_BossLevelGS.generated.h"

UCLASS()
class GAS_API AGS_BossLevelGS : public AGameState
{
    GENERATED_BODY()

public:
    AGS_BossLevelGS();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category = "Timer")
    FText GetFormattedBossTime() const;

    // 보스 레벨의 남은 시간을 초 단위로 반환
    UFUNCTION(BlueprintPure, Category = "Timer")
    float GetRemainingBossTime() const;

    UPROPERTY(ReplicatedUsing = OnRep_BossCurrentTime, VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
    float BossCurrentTime;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
    float BossTotalTime;

    float LastServerTimeUpdate;

protected:
    FTimerHandle BossTimerHandle;

    void UpdateBossTime();

    UFUNCTION()
    void OnRep_BossCurrentTime();


};
