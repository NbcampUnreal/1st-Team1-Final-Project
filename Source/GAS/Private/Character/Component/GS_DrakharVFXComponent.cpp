#include "Character/Component/GS_DrakharVFXComponent.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/ArrowComponent.h"
#include "Character/Component/GS_CameraShakeComponent.h"
#include "Character/Component/GS_FootManagerComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UGS_DrakharVFXComponent::UGS_DrakharVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize component pointers
	ActiveWingRushVFXComponent = nullptr;
	ActiveDustVFXComponent = nullptr;
	ActiveGroundCrackVFXComponent = nullptr;
	ActiveDustCloudVFXComponent = nullptr;
	ActiveFlyingDustVFXComponent = nullptr;
}

void UGS_DrakharVFXComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerDrakhar = Cast<AGS_Drakhar>(GetOwner());

	if (OwnerDrakhar)
	{
		FootManagerComponent = OwnerDrakhar->FindComponentByClass<UGS_FootManagerComponent>();
		
		// Find Arrow Components by name
		TArray<UArrowComponent*> ArrowComponents;
		OwnerDrakhar->GetComponents<UArrowComponent>(ArrowComponents);
		for (UArrowComponent* Arrow : ArrowComponents)
		{
			if (Arrow->GetFName() == TEXT("WingRushVFXSpawnPoint"))
			{
				WingRushVFXSpawnPoint = Arrow;
			}
			else if (Arrow->GetFName() == TEXT("EarthquakeVFXSpawnPoint"))
			{
				EarthquakeVFXSpawnPoint = Arrow;
			}
		}
	}
}

void UGS_DrakharVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveFlyingDustVFXComponent && IsValid(ActiveFlyingDustVFXComponent) && OwnerDrakhar)
	{
		FVector Start = OwnerDrakhar->GetActorLocation();
		FVector End = Start - FVector(0, 0, OwnerDrakhar->FlyingDustTraceDistance);
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerDrakhar);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
		{
			ActiveFlyingDustVFXComponent->SetWorldLocation(HitResult.ImpactPoint);
			if(!ActiveFlyingDustVFXComponent->IsActive())
			{
				ActiveFlyingDustVFXComponent->Activate(true);
			}
		}
		else
		{
			if(ActiveFlyingDustVFXComponent->IsActive())
			{
				ActiveFlyingDustVFXComponent->Deactivate();
			}
		}
	}
}

void UGS_DrakharVFXComponent::OnFlyStart()
{
	if (!OwnerDrakhar) return;
	if (GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }

	if (OwnerDrakhar->FlyingDustVFX && !IsValid(ActiveFlyingDustVFXComponent))
	{
		FVector Location = OwnerDrakhar->GetActorLocation() - FVector(0,0,OwnerDrakhar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		ActiveFlyingDustVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OwnerDrakhar->FlyingDustVFX, Location);
		if(ActiveFlyingDustVFXComponent)
		{
			ActiveFlyingDustVFXComponent->SetAutoDestroy(false);
			ActiveFlyingDustVFXComponent->Deactivate(); 
		}
	}
	
	OwnerDrakhar->BP_OnFlyStart();
}

void UGS_DrakharVFXComponent::OnFlyEnd()
{
	if (!OwnerDrakhar) return;
	if (GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }

	if (ActiveFlyingDustVFXComponent && IsValid(ActiveFlyingDustVFXComponent))
	{
		ActiveFlyingDustVFXComponent->DestroyComponent();
		ActiveFlyingDustVFXComponent = nullptr;
	}
	
	OwnerDrakhar->BP_OnFlyEnd();
}

void UGS_DrakharVFXComponent::OnUltimateStart()
{
	if (OwnerDrakhar)
	{
		OwnerDrakhar->BP_OnUltimateStart();
	}
}

void UGS_DrakharVFXComponent::OnEarthquakeStart()
{
	if (OwnerDrakhar)
	{
		UGS_CameraShakeComponent* CameraShakeComponent = OwnerDrakhar->GetCameraShakeComponent();
		if (CameraShakeComponent)
		{
			CameraShakeComponent->PlayCameraShake(OwnerDrakhar->EarthquakeShakeInfo);
		}
		
		StartGroundCrackVFX();
		StartDustCloudVFX();
		
		OwnerDrakhar->BP_OnEarthquakeStart();
	}
}

void UGS_DrakharVFXComponent::StartWingRushVFX()
{
	if (!OwnerDrakhar) return;
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveWingRushVFXComponent && IsValid(ActiveWingRushVFXComponent))
	{
		ActiveWingRushVFXComponent->DestroyComponent();
		ActiveWingRushVFXComponent = nullptr;
	}

	UNiagaraSystem* VFXToSpawn = OwnerDrakhar->GetIsFeverMode() ? OwnerDrakhar->FeverWingRushRibbonVFX : OwnerDrakhar->WingRushRibbonVFX;
	if (!VFXToSpawn) return;

	if (WingRushVFXSpawnPoint && IsValid(WingRushVFXSpawnPoint))
	{
		ActiveWingRushVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(VFXToSpawn, WingRushVFXSpawnPoint, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
	else
	{
		ActiveWingRushVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(VFXToSpawn, OwnerDrakhar->GetMesh(), FName("foot_l"), FVector(-20.f, 0.f, 0.f), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}

	if (ActiveWingRushVFXComponent)
	{
		FVector CurrentDashDirection = (OwnerDrakhar->DashEndLocation - OwnerDrakhar->DashStartLocation).GetSafeNormal();
		if (!CurrentDashDirection.IsZero())
		{
			ActiveWingRushVFXComponent->SetVectorParameter(FName("DashDirection"), CurrentDashDirection);
		}

		float DashSpeed = OwnerDrakhar->DashPower / OwnerDrakhar->DashDuration;
		ActiveWingRushVFXComponent->SetFloatParameter(FName("DashSpeed"), DashSpeed);
		ActiveWingRushVFXComponent->SetFloatParameter(FName("Scale"), 2.0f);
		ActiveWingRushVFXComponent->SetWorldScale3D(FVector(3.0f, 3.0f, 3.0f));
	}
}

void UGS_DrakharVFXComponent::StopWingRushVFX()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveWingRushVFXComponent && IsValid(ActiveWingRushVFXComponent))
	{
		ActiveWingRushVFXComponent->Deactivate();
		FTimerHandle VFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(VFXCleanupTimer, [this]() {
			if (ActiveWingRushVFXComponent && IsValid(ActiveWingRushVFXComponent))
			{
				ActiveWingRushVFXComponent->DestroyComponent();
			}
			ActiveWingRushVFXComponent = nullptr;
		}, 2.0f, false);
	}
}

void UGS_DrakharVFXComponent::StartDustVFX()
{
	if (!OwnerDrakhar) return;
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveDustVFXComponent && IsValid(ActiveDustVFXComponent))
	{
		ActiveDustVFXComponent->DestroyComponent();
		ActiveDustVFXComponent = nullptr;
	}

	UNiagaraSystem* VFXToSpawn = OwnerDrakhar->GetIsFeverMode() ? OwnerDrakhar->FeverDustVFX : OwnerDrakhar->DustVFX;
	if (!VFXToSpawn) return;

	if (WingRushVFXSpawnPoint && IsValid(WingRushVFXSpawnPoint))
	{
		ActiveDustVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(VFXToSpawn, WingRushVFXSpawnPoint, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
	else
	{
		ActiveDustVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(VFXToSpawn, OwnerDrakhar->GetMesh(), FName("foot_l"), FVector(-20.f, 0.f, 0.f), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}

	if (ActiveDustVFXComponent)
	{
		FVector CurrentDashDirection = (OwnerDrakhar->DashEndLocation - OwnerDrakhar->DashStartLocation).GetSafeNormal();
		if (!CurrentDashDirection.IsZero())
		{
			ActiveDustVFXComponent->SetVectorParameter(FName("DashDirection"), CurrentDashDirection);
		}

		float DashSpeed = OwnerDrakhar->DashPower / OwnerDrakhar->DashDuration;
		ActiveDustVFXComponent->SetFloatParameter(FName("DashSpeed"), DashSpeed);
		ActiveDustVFXComponent->SetFloatParameter(FName("Intensity"), 3.0f);
		ActiveDustVFXComponent->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
	}
}

void UGS_DrakharVFXComponent::StopDustVFX()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveDustVFXComponent && IsValid(ActiveDustVFXComponent))
	{
		ActiveDustVFXComponent->Deactivate();
		FTimerHandle DustVFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(DustVFXCleanupTimer, [this]() {
			if (ActiveDustVFXComponent && IsValid(ActiveDustVFXComponent))
			{
				ActiveDustVFXComponent->DestroyComponent();
			}
			ActiveDustVFXComponent = nullptr;
		}, 1.5f, false);
	}
}

void UGS_DrakharVFXComponent::StartGroundCrackVFX()
{
	if (!OwnerDrakhar) return;
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveGroundCrackVFXComponent && IsValid(ActiveGroundCrackVFXComponent))
	{
		ActiveGroundCrackVFXComponent->DestroyComponent();
		ActiveGroundCrackVFXComponent = nullptr;
	}

	if (!OwnerDrakhar->GroundCrackVFX) return;

	if (EarthquakeVFXSpawnPoint && IsValid(EarthquakeVFXSpawnPoint))
	{
		ActiveGroundCrackVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(OwnerDrakhar->GroundCrackVFX, EarthquakeVFXSpawnPoint, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
	else
	{
		FVector SpawnLocation = OwnerDrakhar->GetActorLocation() + FVector(0.f, 0.f, -90.f);
		ActiveGroundCrackVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OwnerDrakhar->GroundCrackVFX, SpawnLocation, OwnerDrakhar->GetActorRotation(), FVector(1.0f, 1.0f, 1.0f), true);
	}

	if (ActiveGroundCrackVFXComponent)
	{
		ActiveGroundCrackVFXComponent->SetFloatParameter(FName("CrackIntensity"), OwnerDrakhar->EarthquakeShakeInfo.Intensity);
		ActiveGroundCrackVFXComponent->SetFloatParameter(FName("CrackRadius"), OwnerDrakhar->EarthquakeShakeInfo.MaxDistance * 0.5f);
		ActiveGroundCrackVFXComponent->SetWorldScale3D(FVector(2.0f, 2.0f, 1.0f));
	}
}

void UGS_DrakharVFXComponent::StopGroundCrackVFX()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveGroundCrackVFXComponent && IsValid(ActiveGroundCrackVFXComponent))
	{
		ActiveGroundCrackVFXComponent->Deactivate();
		FTimerHandle GroundCrackVFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(GroundCrackVFXCleanupTimer, [this]() {
			if (ActiveGroundCrackVFXComponent && IsValid(ActiveGroundCrackVFXComponent))
			{
				ActiveGroundCrackVFXComponent->DestroyComponent();
			}
			ActiveGroundCrackVFXComponent = nullptr;
		}, 3.0f, false);
	}
}

void UGS_DrakharVFXComponent::StartDustCloudVFX()
{
	if (!OwnerDrakhar) return;
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveDustCloudVFXComponent && IsValid(ActiveDustCloudVFXComponent))
	{
		ActiveDustCloudVFXComponent->DestroyComponent();
		ActiveDustCloudVFXComponent = nullptr;
	}

	if (!OwnerDrakhar->DustCloudVFX) return;

	if (EarthquakeVFXSpawnPoint && IsValid(EarthquakeVFXSpawnPoint))
	{
		ActiveDustCloudVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(OwnerDrakhar->DustCloudVFX, EarthquakeVFXSpawnPoint, NAME_None, FVector(0.f, 0.f, 50.f), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
	else
	{
		FVector SpawnLocation = OwnerDrakhar->GetActorLocation() + FVector(0.f, 0.f, -40.f);
		ActiveDustCloudVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OwnerDrakhar->DustCloudVFX, SpawnLocation, OwnerDrakhar->GetActorRotation(), FVector(1.5f, 1.5f, 1.5f), true);
	}

	if (ActiveDustCloudVFXComponent)
	{
		ActiveDustCloudVFXComponent->SetFloatParameter(FName("DustIntensity"), OwnerDrakhar->EarthquakeShakeInfo.Intensity * 1.5f);
		ActiveDustCloudVFXComponent->SetFloatParameter(FName("DustRadius"), OwnerDrakhar->EarthquakeShakeInfo.MaxDistance * 0.3f);
		ActiveDustCloudVFXComponent->SetFloatParameter(FName("WindStrength"), 5.0f);
		ActiveDustCloudVFXComponent->SetWorldScale3D(FVector(3.0f, 3.0f, 2.0f));

		FTimerHandle DustCloudAutoStopTimer;
		GetWorld()->GetTimerManager().SetTimer(DustCloudAutoStopTimer, this, &UGS_DrakharVFXComponent::StopDustCloudVFX, 2.0f, false);
	}
}

void UGS_DrakharVFXComponent::StopDustCloudVFX()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (ActiveDustCloudVFXComponent && IsValid(ActiveDustCloudVFXComponent))
	{
		ActiveDustCloudVFXComponent->Deactivate();
		FTimerHandle DustCloudVFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(DustCloudVFXCleanupTimer, [this]() {
			if (ActiveDustCloudVFXComponent && IsValid(ActiveDustCloudVFXComponent))
			{
				ActiveDustCloudVFXComponent->DestroyComponent();
			}
			ActiveDustCloudVFXComponent = nullptr;
		}, 1.5f, false);
	}
}

void UGS_DrakharVFXComponent::PlayAttackHitVFX(FVector ImpactPoint)
{
	if (!OwnerDrakhar) return;
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	UNiagaraSystem* VFXToPlay = OwnerDrakhar->GetIsFeverMode() ? OwnerDrakhar->FeverAttackHitVFX : OwnerDrakhar->NormalAttackHitVFX;
	if (VFXToPlay)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFXToPlay, ImpactPoint);
	}
}

void UGS_DrakharVFXComponent::PlayEarthquakeImpactVFX(const FVector& ImpactLocation)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (OwnerDrakhar && OwnerDrakhar->EarthquakeImpactVFX && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OwnerDrakhar->EarthquakeImpactVFX, ImpactLocation, FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::None, true);
	}
}

void UGS_DrakharVFXComponent::PlayFeverEarthquakeImpactVFX(const FVector& ImpactLocation)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (OwnerDrakhar && OwnerDrakhar->FeverEarthquakeImpactVFX && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OwnerDrakhar->FeverEarthquakeImpactVFX, ImpactLocation, FRotator::ZeroRotator, FVector(1.5f), true, true, ENCPoolMethod::None, true);
	}
}

void UGS_DrakharVFXComponent::HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer || !OwnerDrakhar) return;

	UNiagaraSystem* VFXToPlay = bHitCharacter ? OwnerDrakhar->DraconicProjectileExplosionVFX : OwnerDrakhar->DraconicProjectileImpactVFX;
	if (VFXToPlay && GetWorld())
	{
		UNiagaraComponent* ImpactVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFXToPlay, ImpactLocation, FRotationMatrix::MakeFromZ(ImpactNormal).Rotator(), FVector(1.0f, 1.0f, 1.0f), true, true, ENCPoolMethod::None, true);
		if (ImpactVFXComponent)
		{
			ImpactVFXComponent->SetVectorParameter(FName("ImpactNormal"), ImpactNormal);
			ImpactVFXComponent->SetFloatParameter(FName("ImpactIntensity"), bHitCharacter ? 2.0f : 1.0f);
			float VFXScale = bHitCharacter ? 1.5f : 1.0f;
			ImpactVFXComponent->SetWorldScale3D(FVector(VFXScale, VFXScale, VFXScale));
			FLinearColor ImpactColor = bHitCharacter ? FLinearColor::Red : FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
			ImpactVFXComponent->SetColorParameter(FName("ImpactColor"), ImpactColor);
		}
	}

	if (bHitCharacter && OwnerDrakhar && OwnerDrakhar->GetCameraShakeComponent())
	{
		FGS_CameraShakeInfo ImpactShakeInfo;
		ImpactShakeInfo.Intensity = 4.0f;
		ImpactShakeInfo.MaxDistance = 1000.0f;
		ImpactShakeInfo.MinDistance = 100.0f;
		ImpactShakeInfo.PropagationSpeed = 500000.0f;
		ImpactShakeInfo.bUseFalloff = true;
		OwnerDrakhar->GetCameraShakeComponent()->PlayCameraShakeAtLocation(ImpactShakeInfo, ImpactLocation);
	}
}

void UGS_DrakharVFXComponent::OnFeverModeChanged(bool bIsFeverMode)
{
	if (FootManagerComponent)
	{
		if (bIsFeverMode)
		{
			FootManagerComponent->OverrideFootDustEffect(OwnerDrakhar->FeverFootstepVFX);
		}
		else
		{
			FootManagerComponent->ClearFootDustEffectOverride();
		}
	}

	if (bIsFeverMode)
	{
		ApplyFeverModeOverlay();
	}
	else
	{
		RemoveFeverModeOverlay();
	}
	
	if (OwnerDrakhar && OwnerDrakhar->HasAuthority() && !bIsFeverMode)
	{
		if (IsValid(ActiveDustVFXComponent) && ActiveDustVFXComponent->GetAsset() == OwnerDrakhar->FeverDustVFX)
		{
			StopDustVFX();
		}
	}
}

void UGS_DrakharVFXComponent::ApplyFeverModeOverlay()
{
	if (!OwnerDrakhar) return;
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;
	
	if (!OwnerDrakhar->FeverModeOverlayMaterial) return;

	USkeletalMeshComponent* MeshComp = OwnerDrakhar->GetMesh();
	if (!MeshComp) return;

	if (!FeverModeOverlayMID)
	{
		FeverModeOverlayMID = UMaterialInstanceDynamic::Create(OwnerDrakhar->FeverModeOverlayMaterial, this);
	}

	if (FeverModeOverlayMID)
	{
		FeverModeOverlayMID->SetScalarParameterValue(FName("Intensity"), OwnerDrakhar->FeverOverlayIntensity);
		FeverModeOverlayMID->SetVectorParameterValue(FName("OverlayColor"), OwnerDrakhar->FeverOverlayColor);
		MeshComp->SetOverlayMaterial(FeverModeOverlayMID);
	}
}

void UGS_DrakharVFXComponent::RemoveFeverModeOverlay()
{
	if (!OwnerDrakhar) return;
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;
	
	if (USkeletalMeshComponent* MeshComp = OwnerDrakhar->GetMesh())
	{
		MeshComp->SetOverlayMaterial(nullptr);
	}
	FeverModeOverlayMID = nullptr;
} 