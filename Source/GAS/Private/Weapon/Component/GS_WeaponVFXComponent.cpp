// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Component/GS_WeaponVFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Player/Seeker/GS_Merci.h"

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
}

void UGS_WeaponVFXComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGS_WeaponVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 VFX 정리
	ClearAllVFX();
	
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitAuraTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(EnchantTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

// ======================
// 아우라 VFX 제어 함수
// ======================

void UGS_WeaponVFXComponent::ActivateHitAura(ESeekerAuraType SeekerType)
{
	// 서버에서만 처리
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// 안전성 검사
	if (!IsValidForVFXOperation())
	{
		return;
	}

	// 이미 같은 타입의 아우라가 활성화되어 있다면 지속시간만 연장
	if (IsHitAuraActive() && CurrentAuraType == SeekerType)
	{
		// 기존 타이머 취소하고 새로 시작
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(HitAuraTimerHandle);
		}

		float Duration = GetVFXDuration(EWeaponVFXType::HitAura, SeekerType);
		GetWorld()->GetTimerManager().SetTimer(HitAuraTimerHandle, this, &UGS_WeaponVFXComponent::DeactivateHitAuraTimerCallback, Duration, false);
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
		// 비활성화 타이머도 정리
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(HitAuraCleanupTimerHandle);
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

	// 자동 비활성화 타이머 설정
	if (GetWorld() && Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(HitAuraTimerHandle, this, &UGS_WeaponVFXComponent::DeactivateHitAuraTimerCallback, Duration, false);
	}
}

void UGS_WeaponVFXComponent::DeactivateHitAura()
{
	// 서버에서만 처리
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// 안전성 검사
	if (!IsValidForVFXOperation())
	{
		return;
	}

	// 타이머 정리
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

	// 자동 비활성화 타이머 설정
	if (GetWorld() && Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(EnchantTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(EnchantTimerHandle, this, &UGS_WeaponVFXComponent::DeactivateEnchantTimerCallback, Duration, false);
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

// ======================
// 멀티캐스트 함수들
// ======================

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
		UE_LOG(LogTemp, Warning, TEXT("[WeaponVFX] No VFX System found for SeekerType: %d"), (int32)SeekerType);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[WeaponVFX] Activating Hit Aura VFX for SeekerType: %d"), (int32)SeekerType);

	// 기존 VFX 정리 (비활성화 상태 포함)
	if (ActiveHitAuraVFXComponent && IsValid(ActiveHitAuraVFXComponent))
	{
		ActiveHitAuraVFXComponent->DestroyComponent();
		ActiveHitAuraVFXComponent = nullptr;
	}
	
	// 기존 타이머들 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitAuraCleanupTimerHandle);
	}
	
	// 비활성화 상태 리셋
	bHitAuraDeactivating = false;

	// 무기 메시 컴포넌트 가져오기
	USceneComponent* MeshComponent = GetWeaponMeshComponent();
	if (!MeshComponent)
	{
		return;
	}

	// Niagara VFX 생성 및 무기에 부착
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
		
		UE_LOG(LogTemp, Warning, TEXT("[WeaponVFX] Hit Aura VFX successfully activated!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[WeaponVFX] Failed to create Hit Aura VFX component!"));
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
		if (!GetOwner()->HasAuthority() && GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(HitAuraCleanupTimerHandle, this, &UGS_WeaponVFXComponent::CleanupHitAuraTimerCallback, 2.0f, false);
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
			}
		}
	}
	else
	{
		CleanupTrailVFXComponent();
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

// ======================
// 타이머 콜백 함수들
// ======================

void UGS_WeaponVFXComponent::DeactivateHitAuraTimerCallback()
{
	// 서버에서 비활성화 처리
	if (GetOwner()->HasAuthority())
	{
		DeactivateHitAura();
	}
}

void UGS_WeaponVFXComponent::CleanupHitAuraTimerCallback()
{
	// 완전한 정리 (클라이언트에서도 실행)
	CleanupHitAuraVFXComponent();
	bHitAuraDeactivating = false;
	CurrentAuraType = ESeekerAuraType::Default;
}

void UGS_WeaponVFXComponent::DeactivateEnchantTimerCallback()
{
	// 부드러운 비활성화 시작
	SoftDeactivateEnchant();
}

void UGS_WeaponVFXComponent::CleanupEnchantTimerCallback()
{
	// 완전한 정리 (클라이언트에서도 실행)
	CleanupEnchantVFXComponent();
	bEnchantDeactivating = false;
}

// ======================
// 정리 함수들
// ======================

void UGS_WeaponVFXComponent::CleanupHitAuraVFXComponent()
{
	if (ActiveHitAuraVFXComponent && IsValid(ActiveHitAuraVFXComponent))
	{
		ActiveHitAuraVFXComponent->DestroyComponent();
		ActiveHitAuraVFXComponent = nullptr;
	}
}

// ======================
// 부드러운 VFX 비활성화 함수들
// ======================

void UGS_WeaponVFXComponent::SoftDeactivateHitAura()
{
	// 이미 비활성화 중이면 무시
	if (bHitAuraDeactivating)
	{
		return;
	}

	bHitAuraDeactivating = true;

	// 멀티캐스트로 부드러운 비활성화 시작
	Multicast_DeactivateHitAura();

	// 2초 후 완전 정리 (나이아가라 시스템이 자연스럽게 끝날 시간)
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(HitAuraCleanupTimerHandle, this, &UGS_WeaponVFXComponent::CleanupHitAuraTimerCallback, 2.0f, false);
	}
}

void UGS_WeaponVFXComponent::SoftDeactivateEnchant()
{
	// 이미 비활성화 중이면 무시
	if (bEnchantDeactivating)
	{
		return;
	}

	bEnchantDeactivating = true;

	// 나이아가라 VFX를 부드럽게 비활성화
	if (EnchantVFXComponent && IsValid(EnchantVFXComponent))
	{
		EnchantVFXComponent->Deactivate();
	}

	// 2초 후 완전 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(EnchantCleanupTimerHandle, this, &UGS_WeaponVFXComponent::CleanupEnchantTimerCallback, 2.0f, false);
	}
}

void UGS_WeaponVFXComponent::CleanupTrailVFXComponent()
{
	if (TrailVFXComponent && IsValid(TrailVFXComponent))
	{
		TrailVFXComponent->DestroyComponent();
		TrailVFXComponent = nullptr;
	}
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
	if (!GetOwner())
	{
		return nullptr;
	}

	// SkeletalMeshComponent 우선 검색
	if (USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		return SkeletalMesh;
	}

	// StaticMeshComponent 검색
	if (UStaticMeshComponent* StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
	{
		return StaticMesh;
	}

	// 기본적으로 RootComponent 반환
	return GetOwner()->GetRootComponent();
}

bool UGS_WeaponVFXComponent::IsValidForVFXOperation() const
{
	return GetOwner() && GetWorld() && !GetWorld()->bIsTearingDown;
}

ESeekerAuraType UGS_WeaponVFXComponent::GetOwnerSeekerType() const
{
	// 무기의 Owner를 통해 시커 타입 자동 감지
	if (AActor* WeaponOwner = GetOwner())
	{
		if (AActor* CharacterOwner = WeaponOwner->GetOwner())
		{
			if (Cast<AGS_Chan>(CharacterOwner))
			{
				return ESeekerAuraType::Chan;
			}
			else if (Cast<AGS_Ares>(CharacterOwner))
			{
				return ESeekerAuraType::Ares;
			}
			else if (Cast<AGS_Merci>(CharacterOwner))
			{
				return ESeekerAuraType::Merci;
			}
		}
	}
	
	return ESeekerAuraType::Default;
}

// VFX 설정 가져오기 헬퍼 함수들
UNiagaraSystem* UGS_WeaponVFXComponent::GetWeaponVFX(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const
{
	// 오버라이드 설정이 있으면 우선 사용
	if (OverrideVFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& OverrideSettings = OverrideVFXSettingsMap[VFXType];
		if (OverrideSettings.VFXSystemMap.Contains(SeekerType))
		{
			return OverrideSettings.VFXSystemMap[SeekerType];
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings && WeaponVFXSettings->VFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& CommonSettings = WeaponVFXSettings->VFXSettingsMap[VFXType];
		if (CommonSettings.VFXSystemMap.Contains(SeekerType))
		{
			return CommonSettings.VFXSystemMap[SeekerType];
		}
	}

	// Default 타입으로 fallback
	if (SeekerType != ESeekerAuraType::Default)
	{
		return GetWeaponVFX(VFXType, ESeekerAuraType::Default);
	}

	return nullptr;
}

float UGS_WeaponVFXComponent::GetVFXDuration(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const
{
	// 오버라이드 설정이 있으면 우선 사용
	if (OverrideVFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& OverrideSettings = OverrideVFXSettingsMap[VFXType];
		if (OverrideSettings.VFXDurationMap.Contains(SeekerType))
		{
			return OverrideSettings.VFXDurationMap[SeekerType];
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings && WeaponVFXSettings->VFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& CommonSettings = WeaponVFXSettings->VFXSettingsMap[VFXType];
		if (CommonSettings.VFXDurationMap.Contains(SeekerType))
		{
			return CommonSettings.VFXDurationMap[SeekerType];
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
	if (OverrideVFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& OverrideSettings = OverrideVFXSettingsMap[VFXType];
		if (OverrideSettings.VFXScaleMap.Contains(SeekerType))
		{
			return OverrideSettings.VFXScaleMap[SeekerType];
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings && WeaponVFXSettings->VFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& CommonSettings = WeaponVFXSettings->VFXSettingsMap[VFXType];
		if (CommonSettings.VFXScaleMap.Contains(SeekerType))
		{
			return CommonSettings.VFXScaleMap[SeekerType];
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
	if (OverrideVFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& OverrideSettings = OverrideVFXSettingsMap[VFXType];
		if (OverrideSettings.VFXLocationOffsetMap.Contains(SeekerType))
		{
			return OverrideSettings.VFXLocationOffsetMap[SeekerType];
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings && WeaponVFXSettings->VFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& CommonSettings = WeaponVFXSettings->VFXSettingsMap[VFXType];
		if (CommonSettings.VFXLocationOffsetMap.Contains(SeekerType))
		{
			return CommonSettings.VFXLocationOffsetMap[SeekerType];
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
	if (OverrideVFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& OverrideSettings = OverrideVFXSettingsMap[VFXType];
		if (OverrideSettings.VFXRotationOffsetMap.Contains(SeekerType))
		{
			return OverrideSettings.VFXRotationOffsetMap[SeekerType];
		}
	}

	// 공통 설정에서 찾기
	if (WeaponVFXSettings && WeaponVFXSettings->VFXSettingsMap.Contains(VFXType))
	{
		const FWeaponVFXSeekerSettings& CommonSettings = WeaponVFXSettings->VFXSettingsMap[VFXType];
		if (CommonSettings.VFXRotationOffsetMap.Contains(SeekerType))
		{
			return CommonSettings.VFXRotationOffsetMap[SeekerType];
		}
	}

	// Default 타입으로 fallback
	if (SeekerType != ESeekerAuraType::Default)
	{
		return GetVFXRotationOffset(VFXType, ESeekerAuraType::Default);
	}

	return FRotator::ZeroRotator;
}