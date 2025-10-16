// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_LowHealthEffectComponent.generated.h"

class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup=(Effects), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_LowHealthEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGS_LowHealthEffectComponent();

    UPROPERTY(EditDefaultsOnly, Category="LowHealth")
    UMaterialInterface* LowHealthEffectMaterial = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="LowHealth", meta=(ClampMin="0"))
    int32 PostProcessPriority = 10;

    UPROPERTY(EditDefaultsOnly, Category="LowHealth", meta=(ClampMin="0.01"))
    float EffectInterpSpeed = 6.0f;

    UPROPERTY(EditDefaultsOnly, Category="LowHealth", meta=(ClampMin="0.01"))
    float UpdateInterval = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category="LowHealth", meta=(ClampMin="0.0", ClampMax="1.0"))
    float LowHealthThreshold = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category="LowHealth|Material")
    FName HPRatioParamName = TEXT("HPRatio");

    void InitializeForOwner(AActor* InOwner, UPostProcessComponent* InPostProcessComp, UMaterialInterface* InMaterialOverride = nullptr);

    void OnHealthChanged(float Current, float Max);

    void ActivateEffect();
    void DeactivateEffect();

    void ApplyStrength(float Strength01);

    UPostProcessComponent* GetPostProcessComponent() const { return ManagedPostProcessComp; }

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void EnsureMID();
    void StartTimer();
    void StopTimer();
    void TickUpdate();

private:
    TWeakObjectPtr<AActor> OwnerActor;
    UPostProcessComponent* ManagedPostProcessComp = nullptr;
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;
    FTimerHandle UpdateTimerHandle;
    bool bIsActive = false;
    float CurrentStrength = 0.0f;
    float TargetStrength = 0.0f;
};


