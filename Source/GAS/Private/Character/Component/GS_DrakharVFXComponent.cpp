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
	PrimaryComponentTick.bCanEverTick = false; // Tick 비활성화 - Timer 사용

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

void UGS_DrakharVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FlyingDustUpdateTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UGS_DrakharVFXComponent::UpdateFlyingDustVFXLocation()
{
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

			// Timer 시작 (0.1초마다 위치 업데이트)
			GetWorld()->GetTimerManager().SetTimer(
				FlyingDustUpdateTimerHandle,
				this,
				&UGS_DrakharVFXComponent::UpdateFlyingDustVFXLocation,
				0.1f,
				true  // Loop
			);
		}
	}

	OwnerDrakhar->BP_OnFlyStart();
}

void UGS_DrakharVFXComponent::OnFlyEnd()
{
	if (!OwnerDrakhar) return;
	if (GetWorld()->GetNetMode() == NM_DedicatedServer) { return; }

	// Timer 중지
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FlyingDustUpdateTimerHandle);
	}

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
		TWeakObjectPtr<UGS_DrakharVFXComponent> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(VFXCleanupTimer, [WeakThis]() {
			if (!WeakThis.IsValid()) return;

			if (WeakThis->ActiveWingRushVFXComponent && IsValid(WeakThis->ActiveWingRushVFXComponent))
			{
				WeakThis->ActiveWingRushVFXComponent->DestroyComponent();
			}
			WeakThis->ActiveWingRushVFXComponent = nullptr;
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
		TWeakObjectPtr<UGS_DrakharVFXComponent> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(DustVFXCleanupTimer, [WeakThis]() {
			if (!WeakThis.IsValid()) return;

			if (WeakThis->ActiveDustVFXComponent && IsValid(WeakThis->ActiveDustVFXComponent))
			{
				WeakThis->ActiveDustVFXComponent->DestroyComponent();
			}
			WeakThis->ActiveDustVFXComponent = nullptr;
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
		TWeakObjectPtr<UGS_DrakharVFXComponent> WeakThis = this;
        GetWorld()->GetTimerManager().SetTimer(GroundCrackVFXCleanupTimer, [WeakThis]() {
            if (!WeakThis.IsValid()) return;

            if (WeakThis->ActiveGroundCrackVFXComponent && IsValid(WeakThis->ActiveGroundCrackVFXComponent))
            {
                WeakThis->ActiveGroundCrackVFXComponent->DestroyComponent();
            }
            WeakThis->ActiveGroundCrackVFXComponent = nullptr;
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
		TWeakObjectPtr<UGS_DrakharVFXComponent> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(DustCloudVFXCleanupTimer, [WeakThis]() {
			if (!WeakThis.IsValid()) return;

			if (WeakThis->ActiveDustCloudVFXComponent && IsValid(WeakThis->ActiveDustCloudVFXComponent))
			{
				WeakThis->ActiveDustCloudVFXComponent->DestroyComponent();
			}
			WeakThis->ActiveDustCloudVFXComponent = nullptr;
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
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OwnerDrakhar->EarthquakeImpactVFX, ImpactLocation, FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::AutoRelease, true);
	}
}

void UGS_DrakharVFXComponent::PlayFeverEarthquakeImpactVFX(const FVector& ImpactLocation)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (OwnerDrakhar && OwnerDrakhar->FeverEarthquakeImpactVFX && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OwnerDrakhar->FeverEarthquakeImpactVFX, ImpactLocation, FRotator::ZeroRotator, FVector(1.5f), true, true, ENCPoolMethod::AutoRelease, true);
	}
}

void UGS_DrakharVFXComponent::HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer || !OwnerDrakhar) return;

	// 피버모드 상태 확인
	bool bIsFeverMode = OwnerDrakhar->GetIsFeverMode();
	
	// 피버모드와 노멀모드에 따라 다른 VFX 선택
	UNiagaraSystem* VFXToPlay = nullptr;
	if (bHitCharacter)
	{
		// 캐릭터 히트 시 폭발 VFX
		VFXToPlay = bIsFeverMode ? OwnerDrakhar->FeverDraconicProjectileExplosionVFX : OwnerDrakhar->DraconicProjectileExplosionVFX;
	}
	else
	{
		// 일반 임팩트 VFX
		VFXToPlay = bIsFeverMode ? OwnerDrakhar->FeverDraconicProjectileImpactVFX : OwnerDrakhar->DraconicProjectileImpactVFX;
	}
	
	if (VFXToPlay && GetWorld())
	{
		// 피버모드에 따른 VFX 스케일 및 강도 조정
		float BaseScale = bHitCharacter ? 1.5f : 1.0f;
		float FeverModeMultiplier = bIsFeverMode ? 1.5f : 1.0f;
		float FinalScale = BaseScale * FeverModeMultiplier;
		
		float BaseIntensity = bHitCharacter ? 2.0f : 1.0f;
		float FinalIntensity = BaseIntensity * (bIsFeverMode ? 1.3f : 1.0f);
		
		UNiagaraComponent* ImpactVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			ImpactLocation,
			FRotationMatrix::MakeFromZ(ImpactNormal).Rotator(),
			FVector(FinalScale, FinalScale, FinalScale),
			true, true, ENCPoolMethod::AutoRelease, true
		);
		
		if (ImpactVFXComponent)
		{
			ImpactVFXComponent->SetVectorParameter(FName("ImpactNormal"), ImpactNormal);
			ImpactVFXComponent->SetFloatParameter(FName("ImpactIntensity"), FinalIntensity);
			
			// 피버모드에 따른 색상 변경
			FLinearColor ImpactColor;
			if (bIsFeverMode)
			{
				// 피버모드 시 더 강렬한 색상 (붉은 불꽃)
				ImpactColor = bHitCharacter ? FLinearColor(1.0f, 0.2f, 0.0f, 1.0f) : FLinearColor(1.0f, 0.4f, 0.0f, 1.0f);
			}
			else
			{
				// 노멀모드 시 일반적인 색상
				ImpactColor = bHitCharacter ? FLinearColor::Red : FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
			}
			ImpactVFXComponent->SetColorParameter(FName("ImpactColor"), ImpactColor);
			
			// 피버모드 시 추가적인 파라미터 설정
			if (bIsFeverMode)
			{
				ImpactVFXComponent->SetFloatParameter(FName("FeverMode"), 1.0f);
				ImpactVFXComponent->SetFloatParameter(FName("EmissionRate"), 2.0f);
			}
		}
	}

	// 카메라 쉐이크 - 피버모드 시 더 강한 효과
	if (bHitCharacter && OwnerDrakhar && OwnerDrakhar->GetCameraShakeComponent())
	{
		FGS_CameraShakeInfo ImpactShakeInfo;
		ImpactShakeInfo.Intensity = bIsFeverMode ? 6.0f : 4.0f;
		ImpactShakeInfo.MaxDistance = bIsFeverMode ? 1500.0f : 1000.0f;
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

// ======================
// 디버프 VFX 함수 구현
// ======================

void UGS_DrakharVFXComponent::PlayDebuffVFX(EDebuffType DebuffType)
{
	if (!OwnerDrakhar || !OwnerDrakhar->HasAuthority()) return;

	UNiagaraSystem* VFXToPlay = GetDebuffVFX(DebuffType);
	if (!VFXToPlay) return;

	// 기존 VFX 제거
	RemoveDebuffVFX(DebuffType);

	// 새로운 VFX 스폰 위치 및 스케일 계산
	FVector Offset = GetDebuffVFXOffset(DebuffType);
	FVector SpawnLocation = OwnerDrakhar->GetActorLocation() + Offset;
	FVector Scale = GetDebuffVFXScale(DebuffType);

	// 지속시간 설정
	float Duration = GetDebuffVFXDuration(DebuffType);
	if (Duration > 0.f)
	{
		// 현재 제거할 디버프 타입 설정
		CurrentDebuffToRemove = DebuffType;

		GetWorld()->GetTimerManager().SetTimer(
			DebuffVFXTimerHandles.FindOrAdd(DebuffType),
			this,
			&UGS_DrakharVFXComponent::RemoveDebuffVFXTimerCallback,
			Duration,
			false
		);
	}

	// 멀티캐스트로 모든 클라이언트(서버 포함)에서 VFX 생성
	Multicast_PlayDebuffVFX(DebuffType, SpawnLocation, Scale);
}

void UGS_DrakharVFXComponent::RemoveDebuffVFX(EDebuffType DebuffType)
{
	if (UNiagaraComponent** VFXComponent = ActiveDebuffVFXComponents.Find(DebuffType))
	{
		if (*VFXComponent && IsValid(*VFXComponent))
		{
			(*VFXComponent)->Deactivate();
			(*VFXComponent)->DestroyComponent();
		}
		ActiveDebuffVFXComponents.Remove(DebuffType);
	}

	// 타이머 제거
	FTimerHandle* TimerHandle = DebuffVFXTimerHandles.Find(DebuffType);
	if (TimerHandle && GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(*TimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
	}
	DebuffVFXTimerHandles.Remove(DebuffType);

	// 클라이언트들에게 알림
	if (OwnerDrakhar && OwnerDrakhar->HasAuthority())
	{
		Multicast_RemoveDebuffVFX(DebuffType);
	}
}

void UGS_DrakharVFXComponent::PlayDebuffExpireVFX(EDebuffType DebuffType)
{
	if (!OwnerDrakhar || !OwnerDrakhar->HasAuthority()) return;

	UNiagaraSystem* VFXToPlay = GetDebuffExpireVFX(DebuffType);
	if (!VFXToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayDebuffExpireVFX: No Expire VFX found for debuff type: %d"), static_cast<uint8>(DebuffType));
		return;
	}

	FVector Offset = GetDebuffExpireVFXOffset(DebuffType);
	FVector SpawnLocation = OwnerDrakhar->GetActorLocation() + Offset;
	FVector Scale = GetDebuffVFXScale(DebuffType);

	UE_LOG(LogTemp, Warning, TEXT("PlayDebuffExpireVFX: Playing Expire VFX for type %d at location %s"), static_cast<uint8>(DebuffType), *SpawnLocation.ToString());

	// 멀티캐스트로 모든 클라이언트(서버 포함)에서 VFX 생성
	Multicast_PlayDebuffExpireVFX(DebuffType, SpawnLocation, Scale);
}

void UGS_DrakharVFXComponent::RemoveAllDebuffVFX()
{
	TArray<EDebuffType> ActiveDebuffTypes;
	ActiveDebuffVFXComponents.GetKeys(ActiveDebuffTypes);

	for (EDebuffType DebuffType : ActiveDebuffTypes)
	{
		RemoveDebuffVFX(DebuffType);
	}
}

bool UGS_DrakharVFXComponent::IsDebuffVFXActive(EDebuffType DebuffType) const
{
	return ActiveDebuffVFXComponents.Contains(DebuffType);
}

// ======================
// 디버프 VFX 헬퍼 함수 구현
// ======================

float UGS_DrakharVFXComponent::GetDebuffVFXDuration(EDebuffType DebuffType) const
{
	// 오버라이드 설정 우선 확인
	if (const float* OverrideDuration = OverrideDebuffVFXDurationMap.Find(DebuffType))
	{
		return *OverrideDuration;
	}

	// 데이터 에셋 설정 확인
	if (DebuffVFXSettings && DebuffVFXSettings->DebuffVFXDurationMap.Contains(DebuffType))
	{
		return DebuffVFXSettings->DebuffVFXDurationMap[DebuffType];
	}

	// 기본값 반환
	return 5.0f;
}

FVector UGS_DrakharVFXComponent::GetDebuffVFXScale(EDebuffType DebuffType) const
{
	// 오버라이드 설정 우선 확인
	if (const FVector* OverrideScale = OverrideDebuffVFXScaleMap.Find(DebuffType))
	{
		return *OverrideScale;
	}

	// 데이터 에셋 설정 확인
	if (DebuffVFXSettings && DebuffVFXSettings->DebuffVFXScaleMap.Contains(DebuffType))
	{
		return DebuffVFXSettings->DebuffVFXScaleMap[DebuffType];
	}

	// 기본값 반환
	return FVector(1.0f, 1.0f, 1.0f);
}

FVector UGS_DrakharVFXComponent::GetDebuffVFXOffset(EDebuffType DebuffType) const
{
	// 오버라이드 설정 우선 확인
	if (const FVector* OverrideOffset = OverrideDebuffVFXOffsetMap.Find(DebuffType))
	{
		return *OverrideOffset;
	}

	// 데이터 에셋 설정 확인
	if (DebuffVFXSettings && DebuffVFXSettings->DebuffVFXOffsetMap.Contains(DebuffType))
	{
		return DebuffVFXSettings->DebuffVFXOffsetMap[DebuffType];
	}

	// 기본값 반환 (Z축으로 50 유닛 위)
	return FVector(0.f, 0.f, 50.f);
}

FVector UGS_DrakharVFXComponent::GetDebuffExpireVFXOffset(EDebuffType DebuffType) const
{
	// 오버라이드 설정 우선 확인
	if (const FVector* OverrideOffset = OverrideDebuffExpireVFXOffsetMap.Find(DebuffType))
	{
		return *OverrideOffset;
	}

	// 데이터 에셋 설정 확인
	if (DebuffVFXSettings && DebuffVFXSettings->DebuffExpireVFXOffsetMap.Contains(DebuffType))
	{
		return DebuffVFXSettings->DebuffExpireVFXOffsetMap[DebuffType];
	}

	// 기본값 반환 (Z축으로 50 유닛 위)
	return FVector(0.f, 0.f, 50.f);
}

UNiagaraSystem* UGS_DrakharVFXComponent::GetDebuffVFX(EDebuffType DebuffType) const
{
	// 오버라이드 설정 우선 확인
	if (OverrideDebuffVFXMap.Contains(DebuffType))
	{
		return OverrideDebuffVFXMap[DebuffType];
	}

	// 데이터 에셋 설정 확인
	if (DebuffVFXSettings && DebuffVFXSettings->DebuffVFXMap.Contains(DebuffType))
	{
		return DebuffVFXSettings->DebuffVFXMap[DebuffType];
	}

	return nullptr;
}

UNiagaraSystem* UGS_DrakharVFXComponent::GetDebuffExpireVFX(EDebuffType DebuffType) const
{
	// 오버라이드 설정 우선 확인
	if (OverrideDebuffExpireVFXMap.Contains(DebuffType))
	{
		return OverrideDebuffExpireVFXMap[DebuffType];
	}

	// 데이터 에셋 설정 확인
	if (DebuffVFXSettings && DebuffVFXSettings->DebuffExpireVFXMap.Contains(DebuffType))
	{
		return DebuffVFXSettings->DebuffExpireVFXMap[DebuffType];
	}

	return nullptr;
}

void UGS_DrakharVFXComponent::RemoveDebuffVFXTimerCallback()
{
	RemoveDebuffVFX(CurrentDebuffToRemove);
}

// ======================
// 디버프 VFX 멀티캐스트 함수 구현
// ======================

void UGS_DrakharVFXComponent::Multicast_PlayDebuffVFX_Implementation(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale)
{
	if (!OwnerDrakhar || GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	UNiagaraSystem* VFXToPlay = GetDebuffVFX(DebuffType);
	if (!VFXToPlay) return;

	// SpawnSystemAtLocation을 사용하여 오프셋이 적용된 위치에 VFX 생성
	UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFXToPlay,
		SpawnLocation,
		FRotator::ZeroRotator,
		Scale,
		true,                // bAutoDestroy
		true,                // bAutoActivate
		ENCPoolMethod::None, // PoolingMethod
		true                 // bPreCullCheck
	);

	if (VFXComponent)
	{
		ActiveDebuffVFXComponents.Add(DebuffType, VFXComponent);
	}
}

void UGS_DrakharVFXComponent::Multicast_RemoveDebuffVFX_Implementation(EDebuffType DebuffType)
{
	if (GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (UNiagaraComponent** VFXComponent = ActiveDebuffVFXComponents.Find(DebuffType))
	{
		if (*VFXComponent && IsValid(*VFXComponent))
		{
			(*VFXComponent)->Deactivate();
		}
		ActiveDebuffVFXComponents.Remove(DebuffType);
	}
}

void UGS_DrakharVFXComponent::Multicast_PlayDebuffExpireVFX_Implementation(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale)
{
	if (!OwnerDrakhar) return;

	UNiagaraSystem* VFXToPlay = GetDebuffExpireVFX(DebuffType);
	if (!VFXToPlay)
	{
		UE_LOG(LogTemp, Error, TEXT("Multicast_PlayDebuffExpireVFX: VFXToPlay is NULL for type %d"), static_cast<uint8>(DebuffType));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayDebuffExpireVFX: Spawning Expire VFX %s at location %s, NetMode: %d"), 
		*VFXToPlay->GetName(), *SpawnLocation.ToString(), static_cast<int32>(GetWorld()->GetNetMode()));

	// 모든 클라이언트(서버 포함)에서 VFX 생성
	UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFXToPlay,
		SpawnLocation,
		FRotator::ZeroRotator,
		Scale,
		true,                // bAutoDestroy
		true,                // bAutoActivate
		ENCPoolMethod::None, // PoolingMethod
		true                 // bPreCullCheck
	);

	if (SpawnedVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayDebuffExpireVFX: Expire VFX spawned successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Multicast_PlayDebuffExpireVFX: Failed to spawn Expire VFX!"));
	}
} 