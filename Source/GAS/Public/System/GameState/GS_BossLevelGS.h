#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GS_BossLevelGS.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossTimerUpdated, const FText&, FormattedTime);

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

    UPROPERTY(BlueprintAssignable, Category = "Timer")
    FOnBossTimerUpdated OnBossTimerUpdatedDelegate;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_BossCurrentTime, VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
    float BossCurrentTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
    float BossTotalTime;

    FTimerHandle BossTimerHandle;

    void UpdateBossTime();

    UFUNCTION()
    void OnRep_BossCurrentTime();
	
	
};
