// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_DetectionEffectComponent.generated.h"

class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionHUD, bool, bShow);

UCLASS(ClassGroup=(Effects), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_DetectionEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGS_DetectionEffectComponent();

    UPROPERTY(EditDefaultsOnly, Category="Detection")
    UMaterialInterface* DetectionEffectMaterial = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Detection", meta=(ClampMin="0"))
    int32 PostProcessPriority = 11;

    UPROPERTY(EditDefaultsOnly, Category="Detection|Material")
    FName DetectionIntensityParamName = TEXT("DetectionIntensity");

    UPROPERTY(BlueprintAssignable, Category="Detection|UI")
    FOnDetectionHUD OnDetectionHUD;

    void InitializeForOwner(AActor* InOwner, UPostProcessComponent* InPostProcessComp, UMaterialInterface* InMaterialOverride = nullptr);

    void OnDetectedChanged(bool bDetected);
    void SetIntensity(float Intensity01);

    UPostProcessComponent* GetPostProcessComponent() const { return ManagedPostProcessComp; }

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void EnsureMID();

private:
    TWeakObjectPtr<AActor> OwnerActor;
    UPostProcessComponent* ManagedPostProcessComp = nullptr;
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;
};


