#include "Character/Component/GS_DetectionEffectComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"

UGS_DetectionEffectComponent::UGS_DetectionEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGS_DetectionEffectComponent::InitializeForOwner(AActor* InOwner, UPostProcessComponent* InPostProcessComp, UMaterialInterface* InMaterialOverride)
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
        DetectionEffectMaterial = InMaterialOverride;
    }

    EnsureMID();
}

void UGS_DetectionEffectComponent::EnsureMID()
{
    if (!DynamicMaterial && DetectionEffectMaterial && OwnerActor.IsValid())
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(DetectionEffectMaterial, OwnerActor.Get());
        if (ManagedPostProcessComp && DynamicMaterial)
        {
            ManagedPostProcessComp->Settings.WeightedBlendables.Array.Empty();
            ManagedPostProcessComp->Settings.AddBlendable(DynamicMaterial, 1.0f);
            DynamicMaterial->SetScalarParameterValue(DetectionIntensityParamName, 0.0f);
        }
    }
}

void UGS_DetectionEffectComponent::OnDetectedChanged(bool bDetected)
{
    if (!ManagedPostProcessComp)
    {
        return;
    }
    if (bDetected)
    {
        ManagedPostProcessComp->bEnabled = true;
        OnDetectionHUD.Broadcast(true);
    }
    else
    {
        ManagedPostProcessComp->bEnabled = false;
        OnDetectionHUD.Broadcast(false);
        SetIntensity(0.0f);
    }
}

void UGS_DetectionEffectComponent::SetIntensity(float Intensity01)
{
    if (!DynamicMaterial)
    {
        EnsureMID();
    }
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue(DetectionIntensityParamName, Intensity01);
    }
}

void UGS_DetectionEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ManagedPostProcessComp)
    {
        ManagedPostProcessComp->bEnabled = false;
        ManagedPostProcessComp->Settings.WeightedBlendables.Array.Empty();
    }
    Super::EndPlay(EndPlayReason);
}


