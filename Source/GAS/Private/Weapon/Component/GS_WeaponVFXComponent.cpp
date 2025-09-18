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
// 슬래시 이펙트 제어 함수
// ======================

void UGS_WeaponVFXComponent::PlaySlashVFX(const FHitResult& HitResult, ESeekerAuraType AttackerSeekerType)
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

	// 무기 속도 가져오기 (velocity가 없으면 충돌 노멀로 대체)
	FVector WeaponVelocity = GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;
	if (WeaponVelocity.IsNearlyZero(1.f))
	{
		WeaponVelocity = HitResult.ImpactNormal * -100.0f; 
	}
	
	// 멀티캐스트로 모든 클라이언트에 이펙트 재생 요청
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
	
	// 기존 타이머들 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitAuraCleanupTimerHandle);
	}
	
	// 비활성화 상태 리셋
	bHitAuraDeactivating = false;

	// 무기 메시 컴포넌트 가져오기
	USceneComponent* MeshComponent = GetWeaponMeshComponent();
	if (!MeshComponent || !IsValid(MeshComponent))
	{
		return;
	}

	// 잠시 대기하여 이전 정리 작업 완료 보장
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, VFXSystem, MeshComponent, LocationOffset, RotationOffset, Scale, SeekerType]()
		{
			// 타이머 콜백에서 실제 VFX 생성
			if (!IsValidForVFXOperation() || !IsValid(MeshComponent))
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

		// 공통 함수 호출
		UGS_VFX_FunctionLibrary::PlayBloodEffect(this, SlashVFX, ImpactPoint, SlashRotation, ScaleVector.X);
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
		// 직접 무기 소유자가 시커인지 확인 (무기가 캐릭터에 직접 소속된 경우)
		if (Cast<AGS_Chan>(WeaponOwner))
		{
			return ESeekerAuraType::Chan;
		}
		else if (Cast<AGS_Ares>(WeaponOwner))
		{
			return ESeekerAuraType::Ares;
		}
		else if (Cast<AGS_Merci>(WeaponOwner))
		{
			return ESeekerAuraType::Merci;
		}
		
		// 무기의 Owner의 Owner를 확인 (중첩된 소유 구조인 경우)
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

	//  Slash 타입이고 설정이 없으면 기본 혈흔 이펙트 로드
	if (VFXType == EWeaponVFXType::Slash)
	{
		UNiagaraSystem* FallbackSlashVFX = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/RealisticBlood/Burst/Niagara/NS_BloodBurst_High.NS_BloodBurst_High"));
		if (FallbackSlashVFX)
		{
			return FallbackSlashVFX;
		}
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