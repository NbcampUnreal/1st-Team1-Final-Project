// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Component/GS_WeaponVFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "VFX/GS_VFX_FunctionLibrary.h"

UGS_WeaponVFXComponent::UGS_WeaponVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// 기본값 초기화
	ActiveHitAuraVFXComponent = nullptr;
	TrailVFXComponent = nullptr;
	ChargeVFXComponent = nullptr;
	EnchantVFXComponent = nullptr;
	CurrentAuraType = ESeekerAuraType::Default;
	bHitAuraDeactivating = false;
	bEnchantDeactivating = false;
	bTrailDeactivating = false;
	
	// 캐시 초기화
	CachedWeaponMeshComponent = nullptr;
	CachedOwnerSeekerType = ESeekerAuraType::Default;
	CachedFallbackSlashVFX = nullptr;
}

void UGS_WeaponVFXComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ======================
	// 성능 최적화: 캐싱 초기화
	// ======================
	
	// 1. 무기 메시 컴포넌트 캐싱 (FindComponentByClass는 매우 느리므로 한 번만 호출)
	if (AActor* Owner = GetOwner())
	{
		if (USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			CachedWeaponMeshComponent = SkeletalMesh;
		}
		else if (UStaticMeshComponent* StaticMesh = Owner->FindComponentByClass<UStaticMeshComponent>())
		{
			CachedWeaponMeshComponent = StaticMesh;
		}
		else
		{
			CachedWeaponMeshComponent = Owner->GetRootComponent();
		}
	}
	
	// 2. 소유자 시커 타입 캐싱
	CacheOwnerSeekerType();
	
	// 3. 기본 Slash VFX 로드 및 캐싱 (런타임 로딩 방지)
	if (!CachedFallbackSlashVFX)
	{
		CachedFallbackSlashVFX = LoadObject<UNiagaraSystem>(nullptr, 
			TEXT("/Game/VFX/RealisticBlood/Burst/Niagara/NS_BloodBurst_Med.NS_BloodBurst_Med"));
	}
}

void UGS_WeaponVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 VFX 정리
	ClearAllVFX();
	
	// 타이머 정리 (성능 최적화: GetWorld() 한 번만 호출)
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(HitAuraTimerHandle);
		TimerManager.ClearTimer(HitAuraCleanupTimerHandle);
		TimerManager.ClearTimer(EnchantTimerHandle);
		TimerManager.ClearTimer(EnchantCleanupTimerHandle);
		TimerManager.ClearTimer(TrailCleanupTimerHandle);
		TimerManager.ClearTimer(BloodEffectDelayTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

// ======================
// 아우라 VFX 제어 함수
// ======================

void UGS_WeaponVFXComponent::ActivateHitAura(ESeekerAuraType SeekerType)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!IsValidForVFXOperation())
	{
		return;
	}

	// 이미 같은 타입의 아우라가 활성화되어 있다면 지속시간만 연장
	if (IsHitAuraActive() && CurrentAuraType == SeekerType)
	{
		// 기존 타이머 취소하고 새로 시작 (성능 최적화: GetWorld() 한 번만 호출)
		if (UWorld* World = GetWorld())
		{
			float Duration = GetVFXDuration(EWeaponVFXType::HitAura, SeekerType);
			FTimerManager& TimerManager = World->GetTimerManager();
			TimerManager.ClearTimer(HitAuraTimerHandle);
			TimerManager.SetTimer(HitAuraTimerHandle, this, &UGS_WeaponVFXComponent::DeactivateHitAuraTimerCallback, Duration, false);
		}
		return;
	}

	// 기존 아우라가 있거나 비활성화 중이라면 즉시 정리
	if (ActiveHitAuraVFXComponent != nullptr && IsValid(ActiveHitAuraVFXComponent))
	{
		// 즉시 정리 (기존 VFX 제거)
		CleanupHitAuraVFXComponent();
	}
	
	// 만약 비활성화 중이었다면 상태 리셋
	if (bHitAuraDeactivating)
	{
		bHitAuraDeactivating = false;
		// 비활성화 타이머도 정리 (성능 최적화)
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(HitAuraCleanupTimerHandle);
		}
	}

	// VFX 설정 가져오기
	FVector LocationOffset = GetVFXLocationOffset(EWeaponVFXType::HitAura, SeekerType);
	FRotator RotationOffset = GetVFXRotationOffset(EWeaponVFXType::HitAura, SeekerType);
	FVector Scale = GetVFXScale(EWeaponVFXType::HitAura, SeekerType);
	float Duration = GetVFXDuration(EWeaponVFXType::HitAura, SeekerType);

	// 현재 아우라 타입 설정
	CurrentAuraType = SeekerType;
	// 비활성화 상태 리셋
	bHitAuraDeactivating = false;

	// 멀티캐스트로 VFX 활성화
	Multicast_ActivateHitAura(SeekerType, LocationOffset, RotationOffset, Scale, Duration);

	// 자동 비활성화 타이머 설정 (성능 최적화)
	if (UWorld* World = GetWorld(); World && Duration > 0.0f)
	{
		World->GetTimerManager().SetTimer(HitAuraTimerHandle, this, &UGS_WeaponVFXComponent::DeactivateHitAuraTimerCallback, Duration, false);
	}
}

void UGS_WeaponVFXComponent::DeactivateHitAura()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!IsValidForVFXOperation())
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitAuraTimerHandle);
	}

	// 부드러운 비활성화 시작
	SoftDeactivateHitAura();
}

bool UGS_WeaponVFXComponent::IsHitAuraActive() const
{
	return ActiveHitAuraVFXComponent != nullptr && IsValid(ActiveHitAuraVFXComponent) && !bHitAuraDeactivating;
}

// ======================
// 슬래시 이펙트 제어 함수
// ======================

void UGS_WeaponVFXComponent::PlaySlashVFX(const FHitResult& HitResult, ESeekerAuraType AttackerSeekerType)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!IsValidForVFXOperation())
	{
		return;
	}

	// 무기 속도 가져오기 (velocity가 없으면 충돌 노멀로 대체)
	FVector WeaponVelocity = GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;
	if (WeaponVelocity.IsNearlyZero(1.f))
	{
		WeaponVelocity = HitResult.ImpactNormal * -100.0f; 
	}
	
	Multicast_PlaySlashVFX(HitResult.ImpactPoint, WeaponVelocity, AttackerSeekerType);
}

// ======================
// 확장 VFX 기능들
// ======================

void UGS_WeaponVFXComponent::ActivateTrailVFX(bool bActivate)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	ESeekerAuraType SeekerType = GetOwnerSeekerType();
	Multicast_ActivateTrailVFX(bActivate, SeekerType);
}

void UGS_WeaponVFXComponent::ActivateChargeVFX(float ChargeLevel)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	ESeekerAuraType SeekerType = GetOwnerSeekerType();
	Multicast_ActivateChargeVFX(ChargeLevel, SeekerType);
}

void UGS_WeaponVFXComponent::PlaySpecialAttackVFX(ESeekerAuraType SeekerType)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	Multicast_PlaySpecialAttackVFX(SeekerType);
}

void UGS_WeaponVFXComponent::PlayGuardSuccessVFX(const FHitResult& HitResult, ESeekerAuraType DefenderSeekerType)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	Multicast_PlayGuardSuccessVFX(HitResult.ImpactPoint, HitResult.ImpactNormal, DefenderSeekerType);
}

void UGS_WeaponVFXComponent::ActivateEnchantVFX(ESeekerAuraType SeekerType, float Duration)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (Duration < 0.0f)
	{
		Duration = GetVFXDuration(EWeaponVFXType::Enchant, SeekerType);
	}

	Multicast_ActivateEnchantVFX(SeekerType, Duration);

	// 자동 비활성화 타이머 설정 (성능 최적화: GetWorld() 한 번만 호출)
	if (UWorld* World = GetWorld(); World && Duration > 0.0f)
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(EnchantTimerHandle);
		TimerManager.SetTimer(EnchantTimerHandle, this, &UGS_WeaponVFXComponent::DeactivateEnchantTimerCallback, Duration, false);
	}
}

void UGS_WeaponVFXComponent::ClearAllVFX()
{
	// 모든 VFX 컴포넌트 정리
	CleanupHitAuraVFXComponent();
	CleanupTrailVFXComponent();
	CleanupChargeVFXComponent();
	CleanupEnchantVFXComponent();
	
	// 현재 아우라 타입 리셋
	CurrentAuraType = ESeekerAuraType::Default;
}

void UGS_WeaponVFXComponent::Multicast_ActivateHitAura_Implementation(ESeekerAuraType SeekerType, FVector LocationOffset, FRotator RotationOffset, FVector Scale, float Duration)
{
	// 안전성 검사
	if (!IsValidForVFXOperation())
	{
		return;
	}

	// VFX 시스템 가져오기
	UNiagaraSystem* VFXSystem = GetWeaponVFX(EWeaponVFXType::HitAura, SeekerType);
	if (!VFXSystem)
	{
		return;
	}

	// 기존 VFX 정리 (비활성화 상태 포함)
	if (ActiveHitAuraVFXComponent && IsValid(ActiveHitAuraVFXComponent))
	{
		ActiveHitAuraVFXComponent->DestroyComponent();
		ActiveHitAuraVFXComponent = nullptr;
	}
	
	// 기존 타이머들 정리 (성능 최적화)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitAuraCleanupTimerHandle);
	}
	
	bHitAuraDeactivating = false;

	// 무기 메시 컴포넌트 가져오기
	USceneComponent* MeshComponent = GetWeaponMeshComponent();
	if (!MeshComponent || !IsValid(MeshComponent))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		// 성능 최적화: TWeakObjectPtr로 안전한 캡처 및 불필요한 복사 방지
		TWeakObjectPtr<UNiagaraSystem> WeakVFXSystem = VFXSystem;
		TWeakObjectPtr<USceneComponent> WeakMeshComponent = MeshComponent;
		
		World->GetTimerManager().SetTimerForNextTick([this, WeakVFXSystem, WeakMeshComponent, LocationOffset, RotationOffset, Scale, SeekerType]()
		{
			// 타이머 콜백에서 실제 VFX 생성
			if (!IsValidForVFXOperation() || !WeakMeshComponent.IsValid() || !WeakVFXSystem.IsValid())
			{
				return;
			}

			// Niagara VFX 생성 및 무기에 부착
			ActiveHitAuraVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				WeakVFXSystem.Get(),
				WeakMeshComponent.Get(),
				AttachSocketName,
				LocationOffset,
				RotationOffset,
				EAttachLocation::KeepRelativeOffset,
				true
			);

			if (ActiveHitAuraVFXComponent)
			{
				// 스케일 적용
				ActiveHitAuraVFXComponent->SetRelativeScale3D(Scale);
				
				// 현재 아우라 타입 설정
				CurrentAuraType = SeekerType;
				
				// 비활성화 상태 리셋
				bHitAuraDeactivating = false;
			}
		});
		
		return; // 타이머 콜백에서 처리하므로 여기서 종료
	}

	// World가 없는 경우 기존 방식으로 시도
	ActiveHitAuraVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		VFXSystem,
		MeshComponent,
		AttachSocketName,
		LocationOffset,
		RotationOffset,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	if (ActiveHitAuraVFXComponent)
	{
		// 스케일 적용
		ActiveHitAuraVFXComponent->SetRelativeScale3D(Scale);
		
		// 현재 아우라 타입 설정
		CurrentAuraType = SeekerType;
		
		// 비활성화 상태 리셋
		bHitAuraDeactivating = false;
	}
}

void UGS_WeaponVFXComponent::Multicast_DeactivateHitAura_Implementation()
{
	// 부드러운 비활성화: Deactivate 호출로 나이아가라가 자연스럽게 페이드아웃
	if (ActiveHitAuraVFXComponent && IsValid(ActiveHitAuraVFXComponent))
	{
		ActiveHitAuraVFXComponent->Deactivate();

		// 상태 표시
		bHitAuraDeactivating = true;
		
		// 2초 후 완전 정리 (서버가 아닌 경우에만, 서버는 SoftDeactivateHitAura에서 처리)
		// 성능 최적화: GetWorld() 한 번만 호출
		if (!GetOwner()->HasAuthority())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(HitAuraCleanupTimerHandle, this, &UGS_WeaponVFXComponent::CleanupHitAuraTimerCallback, 2.0f, false);
			}
		}
	}
}

void UGS_WeaponVFXComponent::Multicast_ActivateTrailVFX_Implementation(bool bActivate, ESeekerAuraType SeekerType)
{
	if (!IsValidForVFXOperation())
	{
		return;
	}

	if (bActivate)
	{
		// 비활성화 중이었다면 타이머 취소하고 재활성화 (성능 최적화)
		if (bTrailDeactivating)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(TrailCleanupTimerHandle);
			}
			
			bTrailDeactivating = false;
			
			// 기존 컴포넌트가 있고 유효하면 재활성화
			if (TrailVFXComponent && IsValid(TrailVFXComponent))
			{
				TrailVFXComponent->Activate(true);
				return;
			}
		}
		
		UNiagaraSystem* VFXSystem = GetWeaponVFX(EWeaponVFXType::Trail, SeekerType);
		if (VFXSystem && !TrailVFXComponent)
		{
			USceneComponent* MeshComponent = GetWeaponMeshComponent();
			if (MeshComponent)
			{
				TrailVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
					VFXSystem,
					MeshComponent,
					AttachSocketName,
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::KeepRelativeOffset,
					true
				);
				
				bTrailDeactivating = false;
			}
		}
	}
	else
	{
		if (TrailVFXComponent && IsValid(TrailVFXComponent) && !bTrailDeactivating)
		{
			TrailVFXComponent->Deactivate(); // 새 파티클 생성 중지
			bTrailDeactivating = true;
			
			// 0.3초 후 완전 제거 (기존 파티클이 자연스럽게 사라지도록)
			// 성능 최적화: GetWorld() 한 번만 호출
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					TrailCleanupTimerHandle,
					this,
					&UGS_WeaponVFXComponent::CleanupTrailVFXComponent,
					0.3f,  // 페이드아웃 시간 (나이아가라 시스템의 Particle Lifetime에 맞춤)
					false
				);
			}
		}
	}
}

void UGS_WeaponVFXComponent::Multicast_ActivateChargeVFX_Implementation(float ChargeLevel, ESeekerAuraType SeekerType)
{
	if (!IsValidForVFXOperation())
	{
		return;
	}

	// 기존 차징 VFX 정리
	CleanupChargeVFXComponent();

	if (ChargeLevel > 0.0f)
	{
		UNiagaraSystem* VFXSystem = GetWeaponVFX(EWeaponVFXType::Charge, SeekerType);
		if (VFXSystem)
		{
			USceneComponent* MeshComponent = GetWeaponMeshComponent();
			if (MeshComponent)
			{
				ChargeVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
					VFXSystem,
					MeshComponent,
					AttachSocketName,
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::KeepRelativeOffset,
					true
				);

				if (ChargeVFXComponent)
				{
					// 차징 레벨에 따른 스케일 조정
					FVector Scale = GetVFXScale(EWeaponVFXType::Charge, SeekerType);
					ChargeVFXComponent->SetRelativeScale3D(Scale * ChargeLevel);
				}
			}
		}
	}
}

void UGS_WeaponVFXComponent::Multicast_PlaySpecialAttackVFX_Implementation(ESeekerAuraType SeekerType)
{
	if (!IsValidForVFXOperation())
	{
		return;
	}

	UNiagaraSystem* VFXSystem = GetWeaponVFX(EWeaponVFXType::SpecialAttack, SeekerType);
	if (VFXSystem)
	{
		USceneComponent* MeshComponent = GetWeaponMeshComponent();
		if (MeshComponent)
		{
			// 일회성 이펙트이므로 컴포넌트를 따로 저장하지 않음
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				VFXSystem,
				MeshComponent,
				AttachSocketName,
				GetVFXLocationOffset(EWeaponVFXType::SpecialAttack, SeekerType),
				GetVFXRotationOffset(EWeaponVFXType::SpecialAttack, SeekerType),
				EAttachLocation::KeepRelativeOffset,
				true
			);
		}
	}
}

void UGS_WeaponVFXComponent::Multicast_PlayGuardSuccessVFX_Implementation(FVector ImpactPoint, FVector ImpactNormal, ESeekerAuraType DefenderSeekerType)
{
	if (!IsValidForVFXOperation())
	{
		return;
	}

	UNiagaraSystem* VFXSystem = GetWeaponVFX(EWeaponVFXType::GuardSuccess, DefenderSeekerType);
	if (VFXSystem && GetWorld())
	{
		// 방패 메시 컴포넌트에 부착
		USceneComponent* MeshComponent = GetWeaponMeshComponent();
		if (MeshComponent)
		{
			// 방패 중앙에서 이펙트 재생
			UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				VFXSystem,
				MeshComponent,
				AttachSocketName,
				GetVFXLocationOffset(EWeaponVFXType::GuardSuccess, DefenderSeekerType),
				ImpactNormal.Rotation() + GetVFXRotationOffset(EWeaponVFXType::GuardSuccess, DefenderSeekerType),
				EAttachLocation::KeepRelativeOffset,
				true
			);
			
			// 스케일 별도 설정
			if (VFXComponent)
			{
				VFXComponent->SetRelativeScale3D(GetVFXScale(EWeaponVFXType::GuardSuccess, DefenderSeekerType));
			}
		}
		else
		{
			// 메시 컴포넌트가 없으면 충돌 지점에서 재생
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				VFXSystem,
				ImpactPoint,
				ImpactNormal.Rotation(),
				FVector(1.0f),
				true,
				true
			);
		}
	}
}

void UGS_WeaponVFXComponent::Multicast_ActivateEnchantVFX_Implementation(ESeekerAuraType SeekerType, float Duration)
{
	if (!IsValidForVFXOperation())
	{
		return;
	}

	// 기존 인챈트 VFX 정리
	CleanupEnchantVFXComponent();

	UNiagaraSystem* VFXSystem = GetWeaponVFX(EWeaponVFXType::Enchant, SeekerType);
	if (VFXSystem)
	{
		USceneComponent* MeshComponent = GetWeaponMeshComponent();
		if (MeshComponent)
		{
			EnchantVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				VFXSystem,
				MeshComponent,
				AttachSocketName,
				GetVFXLocationOffset(EWeaponVFXType::Enchant, SeekerType),
				GetVFXRotationOffset(EWeaponVFXType::Enchant, SeekerType),
				EAttachLocation::KeepRelativeOffset,
				true
			);

			if (EnchantVFXComponent)
			{
				EnchantVFXComponent->SetRelativeScale3D(GetVFXScale(EWeaponVFXType::Enchant, SeekerType));
			}
		}
	}
}

void UGS_WeaponVFXComponent::Multicast_PlaySlashVFX_Implementation(FVector ImpactPoint, FVector WeaponVelocity, ESeekerAuraType SeekerType)
{
	if (!IsValidForVFXOperation())
	{
		return;
	}

	// Slash Effect
	UNiagaraSystem* SlashVFX = GetWeaponVFX(EWeaponVFXType::Slash, SeekerType);

	// 무기 속도가 너무 낮으면 슬래시 이펙트를 표시하지 않음 (오차 방지)
	if (SlashVFX && !WeaponVelocity.IsNearlyZero(1.f))
	{
		// 슬래시 이펙트가 무기의 이동 방향을 따라 그려지도록 회전 설정
		const FRotator SlashRotation = WeaponVelocity.Rotation();
		const FVector ScaleVector = GetVFXScale(EWeaponVFXType::Slash, SeekerType);

		DelayedHitLocation = ImpactPoint;  // 위치 저장
		DelayedHitNormal = WeaponVelocity.GetSafeNormal(); 
		DelayedScale = ScaleVector.X;

		// 타이머 설정 (딜레이 후 DelayedBloodEffect 호출)
		// 성능 최적화: GetWorld() 한 번만 호출
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				BloodEffectDelayTimerHandle,
				this,
				&UGS_WeaponVFXComponent::DelayedBloodEffect,
				0.125f,  // 조정 가능한 딜레이
				false
			);
		}
	}
}

void UGS_WeaponVFXComponent::DeactivateHitAuraTimerCallback()
{
	if (GetOwner()->HasAuthority())
	{
		DeactivateHitAura();
	}
}

void UGS_WeaponVFXComponent::CleanupHitAuraTimerCallback()
{
	CleanupHitAuraVFXComponent();
	bHitAuraDeactivating = false;
	CurrentAuraType = ESeekerAuraType::Default;
}

void UGS_WeaponVFXComponent::DeactivateEnchantTimerCallback()
{
	SoftDeactivateEnchant();
}

void UGS_WeaponVFXComponent::CleanupEnchantTimerCallback()
{
	CleanupEnchantVFXComponent();
	bEnchantDeactivating = false;
}

void UGS_WeaponVFXComponent::DelayedBloodEffect()
{
	UGS_VFX_FunctionLibrary::PlayBloodEffect(this, GetWeaponVFX(EWeaponVFXType::Slash, GetOwnerSeekerType()), DelayedHitLocation, DelayedHitNormal.Rotation(), DelayedScale);
}

void UGS_WeaponVFXComponent::CleanupHitAuraVFXComponent()
{
	if (ActiveHitAuraVFXComponent && IsValid(ActiveHitAuraVFXComponent))
	{
		ActiveHitAuraVFXComponent->DestroyComponent();
		ActiveHitAuraVFXComponent = nullptr;
	}
}

void UGS_WeaponVFXComponent::SoftDeactivateHitAura()
{
	if (bHitAuraDeactivating)
	{
		return;
	}

	bHitAuraDeactivating = true;

	Multicast_DeactivateHitAura();

	// 성능 최적화: GetWorld() 한 번만 호출
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HitAuraCleanupTimerHandle, this, &UGS_WeaponVFXComponent::CleanupHitAuraTimerCallback, 2.0f, false);
	}
}

void UGS_WeaponVFXComponent::SoftDeactivateEnchant()
{
	if (bEnchantDeactivating)
	{
		return;
	}

	bEnchantDeactivating = true;

	if (EnchantVFXComponent && IsValid(EnchantVFXComponent))
	{
		EnchantVFXComponent->Deactivate();
	}

	// 성능 최적화: GetWorld() 한 번만 호출
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(EnchantCleanupTimerHandle, this, &UGS_WeaponVFXComponent::CleanupEnchantTimerCallback, 2.0f, false);
	}
}

void UGS_WeaponVFXComponent::CleanupTrailVFXComponent()
{
	if (TrailVFXComponent && IsValid(TrailVFXComponent))
	{
		TrailVFXComponent->DestroyComponent();
		TrailVFXComponent = nullptr;
	}
	bTrailDeactivating = false;  // 상태 리셋
}

void UGS_WeaponVFXComponent::CleanupChargeVFXComponent()
{
	if (ChargeVFXComponent && IsValid(ChargeVFXComponent))
	{
		ChargeVFXComponent->DestroyComponent();
		ChargeVFXComponent = nullptr;
	}
}

void UGS_WeaponVFXComponent::CleanupEnchantVFXComponent()
{
	if (EnchantVFXComponent && IsValid(EnchantVFXComponent))
	{
		EnchantVFXComponent->DestroyComponent();
		EnchantVFXComponent = nullptr;
	}
}

// ======================
// 헬퍼 함수들
// ======================

USceneComponent* UGS_WeaponVFXComponent::GetWeaponMeshComponent() const
{
	// 캐싱된 컴포넌트 반환 (성능 최적화: FindComponentByClass 호출 제거)
	return CachedWeaponMeshComponent;
}

bool UGS_WeaponVFXComponent::IsValidForVFXOperation() const
{
	return GetOwner() && GetWorld() && !GetWorld()->bIsTearingDown;
}

ESeekerAuraType UGS_WeaponVFXComponent::GetOwnerSeekerType() const
{
	// 캐싱된 시커 타입 반환 (성능 최적화: Cast 연산 제거)
	return CachedOwnerSeekerType;
}

void UGS_WeaponVFXComponent::CacheOwnerSeekerType()
{
	// 무기의 Owner를 통해 시커 타입 자동 감지 후 캐싱
	if (AActor* WeaponOwner = GetOwner())
	{
		// 직접 무기 소유자가 시커인지 확인 (무기가 캐릭터에 직접 소속된 경우)
		if (Cast<AGS_Chan>(WeaponOwner))
		{
			CachedOwnerSeekerType = ESeekerAuraType::Chan;
			return;
		}
		else if (Cast<AGS_Ares>(WeaponOwner))
		{
			CachedOwnerSeekerType = ESeekerAuraType::Ares;
			return;
		}
		else if (Cast<AGS_Merci>(WeaponOwner))
		{
			CachedOwnerSeekerType = ESeekerAuraType::Merci;
			return;
		}
		
		// 무기의 Owner의 Owner를 확인 (중첩된 소유 구조인 경우)
		if (AActor* CharacterOwner = WeaponOwner->GetOwner())
		{
			if (Cast<AGS_Chan>(CharacterOwner))
			{
				CachedOwnerSeekerType = ESeekerAuraType::Chan;
				return;
			}
			else if (Cast<AGS_Ares>(CharacterOwner))
			{
				CachedOwnerSeekerType = ESeekerAuraType::Ares;
				return;
			}
			else if (Cast<AGS_Merci>(CharacterOwner))
			{
				CachedOwnerSeekerType = ESeekerAuraType::Merci;
				return;
			}
		}
	}
	
	CachedOwnerSeekerType = ESeekerAuraType::Default;
}

// VFX 설정 가져오기 헬퍼 함수들
UNiagaraSystem* UGS_WeaponVFXComponent::GetWeaponVFX(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const
{
	// 오버라이드 설정이 있으면 우선 사용
	// 성능 최적화: Contains() + operator[] 대신 Find() 사용 (2번 조회 → 1번 조회)
	if (const FWeaponVFXSeekerSettings* OverrideSettings = OverrideVFXSettingsMap.Find(VFXType))
	{
		if (UNiagaraSystem* const* VFXSystem = OverrideSettings->VFXSystemMap.Find(SeekerType))
		{
			return *VFXSystem;
		}
	}

	// 공통 설정에서 찾기
	// 성능 최적화: Contains() + operator[] 대신 Find() 사용
	if (WeaponVFXSettings)
	{
		if (const FWeaponVFXSeekerSettings* CommonSettings = WeaponVFXSettings->VFXSettingsMap.Find(VFXType))
		{
			if (UNiagaraSystem* const* VFXSystem = CommonSettings->VFXSystemMap.Find(SeekerType))
			{
				return *VFXSystem;
			}
		}
	}

	// Default 타입으로 fallback
	if (SeekerType != ESeekerAuraType::Default)
	{
		return GetWeaponVFX(VFXType, ESeekerAuraType::Default);
	}

	// Slash 타입이고 설정이 없으면 캐싱된 기본 혈흔 이펙트 반환
	// (성능 최적화: 런타임 LoadObject 호출 제거)
	if (VFXType == EWeaponVFXType::Slash)
	{
		return CachedFallbackSlashVFX;
	}

	return nullptr;
}

float UGS_WeaponVFXComponent::GetVFXDuration(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const
{
	// 오버라이드 설정이 있으면 우선 사용
	// 성능 최적화: Find() 사용
	if (const FWeaponVFXSeekerSettings* OverrideSettings = OverrideVFXSettingsMap.Find(VFXType))
	{
		if (const float* Duration = OverrideSettings->VFXDurationMap.Find(SeekerType))
		{
			return *Duration;
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings)
	{
		if (const FWeaponVFXSeekerSettings* CommonSettings = WeaponVFXSettings->VFXSettingsMap.Find(VFXType))
		{
			if (const float* Duration = CommonSettings->VFXDurationMap.Find(SeekerType))
			{
				return *Duration;
			}
		}
	}

	// Default 타입으로 fallback
	if (SeekerType != ESeekerAuraType::Default)
	{
		return GetVFXDuration(VFXType, ESeekerAuraType::Default);
	}

	return DefaultVFXDuration;
}

FVector UGS_WeaponVFXComponent::GetVFXScale(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const
{
	// 오버라이드 설정이 있으면 우선 사용
	// 성능 최적화: Find() 사용
	if (const FWeaponVFXSeekerSettings* OverrideSettings = OverrideVFXSettingsMap.Find(VFXType))
	{
		if (const FVector* Scale = OverrideSettings->VFXScaleMap.Find(SeekerType))
		{
			return *Scale;
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings)
	{
		if (const FWeaponVFXSeekerSettings* CommonSettings = WeaponVFXSettings->VFXSettingsMap.Find(VFXType))
		{
			if (const FVector* Scale = CommonSettings->VFXScaleMap.Find(SeekerType))
			{
				return *Scale;
			}
		}
	}

	// Default 타입으로 fallback
	if (SeekerType != ESeekerAuraType::Default)
	{
		return GetVFXScale(VFXType, ESeekerAuraType::Default);
	}

	return FVector(1.0f);
}

FVector UGS_WeaponVFXComponent::GetVFXLocationOffset(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const
{
	// 오버라이드 설정이 있으면 우선 사용
	// 성능 최적화: Find() 사용
	if (const FWeaponVFXSeekerSettings* OverrideSettings = OverrideVFXSettingsMap.Find(VFXType))
	{
		if (const FVector* LocationOffset = OverrideSettings->VFXLocationOffsetMap.Find(SeekerType))
		{
			return *LocationOffset;
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings)
	{
		if (const FWeaponVFXSeekerSettings* CommonSettings = WeaponVFXSettings->VFXSettingsMap.Find(VFXType))
		{
			if (const FVector* LocationOffset = CommonSettings->VFXLocationOffsetMap.Find(SeekerType))
			{
				return *LocationOffset;
			}
		}
	}

	// Default 타입으로 fallback
	if (SeekerType != ESeekerAuraType::Default)
	{
		return GetVFXLocationOffset(VFXType, ESeekerAuraType::Default);
	}

	return FVector::ZeroVector;
}

FRotator UGS_WeaponVFXComponent::GetVFXRotationOffset(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const
{
	// 오버라이드 설정이 있으면 우선 사용
	// 성능 최적화: Find() 사용
	if (const FWeaponVFXSeekerSettings* OverrideSettings = OverrideVFXSettingsMap.Find(VFXType))
	{
		if (const FRotator* RotationOffset = OverrideSettings->VFXRotationOffsetMap.Find(SeekerType))
		{
			return *RotationOffset;
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings)
	{
		if (const FWeaponVFXSeekerSettings* CommonSettings = WeaponVFXSettings->VFXSettingsMap.Find(VFXType))
		{
			if (const FRotator* RotationOffset = CommonSettings->VFXRotationOffsetMap.Find(SeekerType))
			{
				return *RotationOffset;
			}
		}
	}

	// Default 타입으로 fallback
	if (SeekerType != ESeekerAuraType::Default)
	{
		return GetVFXRotationOffset(VFXType, ESeekerAuraType::Default);
	}

	return FRotator::ZeroRotator;
}