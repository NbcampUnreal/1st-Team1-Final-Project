#include "Character/Component/GS_LowHealthEffectComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

UGS_LowHealthEffectComponent::UGS_LowHealthEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGS_LowHealthEffectComponent::InitializeForOwner(AActor* InOwner, UPostProcessComponent* InPostProcessComp, UMaterialInterface* InMaterialOverride)
{
    OwnerActor = InOwner;
    ManagedPostProcessComp = InPostProcessComp;
    if (ManagedPostProcessComp)
    {
        ManagedPostProcessComp->bEnabled = false;
        ManagedPostProcessComp->Priority = PostProcessPriority;
        ManagedPostProcessComp->Settings.WeightedBlendables.Array.Empty();
    }

    if (InMaterialOverride)
    {
        LowHealthEffectMaterial = InMaterialOverride;
    }

    EnsureMID();
}

void UGS_LowHealthEffectComponent::EnsureMID()
{
    if (!DynamicMaterial && LowHealthEffectMaterial && OwnerActor.IsValid())
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(LowHealthEffectMaterial, OwnerActor.Get());
        if (ManagedPostProcessComp && DynamicMaterial)
        {
            ManagedPostProcessComp->Settings.WeightedBlendables.Array.Empty();
            ManagedPostProcessComp->Settings.AddBlendable(DynamicMaterial, 1.0f);
        }
    }
}

void UGS_LowHealthEffectComponent::OnHealthChanged(float Current, float Max)
{
    if (!OwnerActor.IsValid() || !ManagedPostProcessComp)
    {
        return;
    }

    const float SafeMax = FMath::Max(1.0f, Max);
    const float HealthRatio = FMath::Clamp(Current / SafeMax, 0.0f, 1.0f);
    const bool bShouldBeActive = (HealthRatio <= LowHealthThreshold) && (Current > KINDA_SMALL_NUMBER);

    if (bShouldBeActive)
    {
        TargetStrength = 1.0f - HealthRatio;
        ActivateEffect();
    }
    else
    {
        TargetStrength = 0.0f;
        if (bIsActive)
        {
            DeactivateEffect();
        }
    }
}

void UGS_LowHealthEffectComponent::ActivateEffect()
{
    EnsureMID();
    if (!ManagedPostProcessComp || !DynamicMaterial)
    {
        return;
    }

    if (!bIsActive)
    {
        bIsActive = true;
        ManagedPostProcessComp->bEnabled = true;
        StartTimer();
    }
}

void UGS_LowHealthEffectComponent::DeactivateEffect()
{
    if (!ManagedPostProcessComp)
    {
        return;
    }

    bIsActive = false;
    StopTimer();
    ApplyStrength(0.0f);
    ManagedPostProcessComp->bEnabled = false;
}

void UGS_LowHealthEffectComponent::ApplyStrength(float Strength01)
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue(HPRatioParamName, Strength01);
    }
}

void UGS_LowHealthEffectComponent::StartTimer()
{
    if (!OwnerActor.IsValid())
    {
        return;
    }
    if (UWorld* World = OwnerActor->GetWorld())
    {
        World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UGS_LowHealthEffectComponent::TickUpdate, UpdateInterval, true);
    }
}

void UGS_LowHealthEffectComponent::StopTimer()
{
    if (!OwnerActor.IsValid())
    {
        return;
    }
    if (UWorld* World = OwnerActor->GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }
}

void UGS_LowHealthEffectComponent::TickUpdate()
{
    if (!OwnerActor.IsValid())
    {
        StopTimer();
        return;
    }
    CurrentStrength = FMath::FInterpTo(CurrentStrength, TargetStrength, UpdateInterval, EffectInterpSpeed);
    ApplyStrength(CurrentStrength);
    if (!bIsActive && CurrentStrength < KINDA_SMALL_NUMBER)
    {
        if (ManagedPostProcessComp)
        {
            ManagedPostProcessComp->bEnabled = false;
        }
        StopTimer();
    }
}

void UGS_LowHealthEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopTimer();
    if (ManagedPostProcessComp)
    {
        ManagedPostProcessComp->bEnabled = false;
        ManagedPostProcessComp->Settings.WeightedBlendables.Array.Empty();
    }
    Super::EndPlay(EndPlayReason);
}


