// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Component/GS_DebuffVFXComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGS_DebuffVFXComponent::UGS_DebuffVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false); // VFX는 로컬에서 처리
	
	// 기본값 설정
	DefaultVFXDuration = 5.0f;
}

void UGS_DebuffVFXComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGS_DebuffVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 VFX 정리
	RemoveAllDebuffVFX();
	Super::EndPlay(EndPlayReason);
}

void UGS_DebuffVFXComponent::PlayDebuffVFX(EDebuffType DebuffType)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FVector SpawnLocation = GetOwner()->GetActorLocation();
	FVector VFXScale = GetVFXScale(DebuffType);
	
	Multicast_PlayDebuffVFX(DebuffType, SpawnLocation, VFXScale);
	
}

void UGS_DebuffVFXComponent::RemoveDebuffVFX(EDebuffType DebuffType)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	Multicast_RemoveDebuffVFX(DebuffType);
	
}

void UGS_DebuffVFXComponent::PlayDebuffExpireVFX(EDebuffType DebuffType)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FVector SpawnLocation = GetOwner()->GetActorLocation();
	FVector VFXScale = GetVFXScale(DebuffType);
	
	Multicast_PlayDebuffExpireVFX(DebuffType, SpawnLocation, VFXScale);
	
}

void UGS_DebuffVFXComponent::RemoveAllDebuffVFX()
{
	// 서버에서만 멀티캐스트 호출
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// 모든 활성 VFX 제거
		TArray<EDebuffType> ActiveTypes;
		ActiveVFXComponents.GetKeys(ActiveTypes);
		
		for (EDebuffType Type : ActiveTypes)
		{
			Multicast_RemoveDebuffVFX(Type);
		}
	}
}

bool UGS_DebuffVFXComponent::IsDebuffVFXActive(EDebuffType DebuffType) const
{
	UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(DebuffType);
	return VFXComp && IsValid(VFXComp);
}

UNiagaraSystem* UGS_DebuffVFXComponent::GetDebuffVFX(EDebuffType DebuffType) const
{
	// 1. 먼저 개별 오버라이드 확인
	if (UNiagaraSystem* const* OverrideVFX = OverrideDebuffVFXMap.Find(DebuffType))
	{
		if (*OverrideVFX)
		{
			return *OverrideVFX;
		}
	}
	
	// 2. 공통 설정에서 확인
	if (CommonVFXSettings)
	{
		if (UNiagaraSystem* const* CommonVFX = CommonVFXSettings->DebuffVFXMap.Find(DebuffType))
		{
			return *CommonVFX;
		}
	}
	
	return nullptr;
}

UNiagaraSystem* UGS_DebuffVFXComponent::GetDebuffExpireVFX(EDebuffType DebuffType) const
{
	// 1. 먼저 개별 오버라이드 확인
	if (UNiagaraSystem* const* OverrideVFX = OverrideDebuffExpireVFXMap.Find(DebuffType))
	{
		if (*OverrideVFX)
		{
			return *OverrideVFX;
		}
	}
	
	// 2. 공통 설정에서 확인
	if (CommonVFXSettings)
	{
		if (UNiagaraSystem* const* CommonVFX = CommonVFXSettings->DebuffExpireVFXMap.Find(DebuffType))
		{
			return *CommonVFX;
		}
	}
	
	return nullptr;
}

void UGS_DebuffVFXComponent::Multicast_PlayDebuffVFX_Implementation(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale)
{
	UNiagaraSystem* VFXSystem = GetDebuffVFX(DebuffType);
	
	if (!VFXSystem)
	{
		
		return;
	}

	// 기존 VFX가 있다면 제거
	if (UNiagaraComponent** ExistingComponent = ActiveVFXComponents.Find(DebuffType))
	{
		if (*ExistingComponent && IsValid(*ExistingComponent))
		{
			(*ExistingComponent)->DestroyComponent();
		}
		ActiveVFXComponents.Remove(DebuffType);
	}

	// 새 VFX 생성
	UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		VFXSystem,
		GetOwner()->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Scale,
		EAttachLocation::KeepRelativeOffset,
		true,                // bAutoDestroy
		ENCPoolMethod::None, // PoolingMethod
		true,                // bAutoActivate
		true                 // bPreCullCheck
	);

	if (VFXComponent)
	{
		ActiveVFXComponents.Add(DebuffType, VFXComponent);

		// 자동 제거 타이머 설정
		float Duration = GetVFXDuration(DebuffType);
		if (Duration > 0.0f)
		{
			FTimerHandle TimerHandle;
			GetOwner()->GetWorldTimerManager().SetTimer(
				TimerHandle,
				FTimerDelegate::CreateUObject(this, &UGS_DebuffVFXComponent::RemoveVFXTimerCallback, DebuffType),
				Duration,
				false
			);
			VFXTimerHandles.Add(DebuffType, TimerHandle);
		}
		
	}
}

void UGS_DebuffVFXComponent::Multicast_RemoveDebuffVFX_Implementation(EDebuffType DebuffType)
{
	// VFX 컴포넌트 제거
	if (UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(DebuffType))
	{
		VFXComp->DestroyComponent();
		ActiveVFXComponents.Remove(DebuffType);
	}

	// 타이머 제거 (각 클라이언트에서 로컬 타이머 정리)
	if (FTimerHandle* TimerHandle = VFXTimerHandles.Find(DebuffType))
	{
		if (GetOwner() && GetOwner()->GetWorldTimerManager().IsTimerActive(*TimerHandle))
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(*TimerHandle);
		}
		VFXTimerHandles.Remove(DebuffType);
	}
}

void UGS_DebuffVFXComponent::Multicast_PlayDebuffExpireVFX_Implementation(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale)
{
	UNiagaraSystem* ExpireVFXSystem = GetDebuffExpireVFX(DebuffType);
	
	if (!ExpireVFXSystem)
	{
		
		return;
	}

	// 만료 VFX는 일회성이므로 월드에 스폰
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ExpireVFXSystem,
		SpawnLocation,
		FRotator::ZeroRotator,
		Scale,
		true,                // bAutoDestroy
		true,                // bAutoActivate
		ENCPoolMethod::None, // PoolingMethod
		true                 // bPreCullCheck
	);

	
}

void UGS_DebuffVFXComponent::RemoveVFXTimerCallback(EDebuffType DebuffType)
{
	// 타이머에 의한 자동 제거 (로컬에서만 실행)
	if (UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(DebuffType))
	{
		if (IsValid(VFXComp))
		{
			VFXComp->DestroyComponent();
		}
		ActiveVFXComponents.Remove(DebuffType);
	}
	VFXTimerHandles.Remove(DebuffType);
}

float UGS_DebuffVFXComponent::GetVFXDuration(EDebuffType DebuffType) const
{
	// 1. 먼저 개별 오버라이드 확인
	if (const float* OverrideDuration = OverrideDebuffVFXDurationMap.Find(DebuffType))
	{
		return *OverrideDuration;
	}
	
	// 2. 공통 설정에서 확인
	if (CommonVFXSettings)
	{
		if (const float* CommonDuration = CommonVFXSettings->DebuffVFXDurationMap.Find(DebuffType))
		{
			return *CommonDuration;
		}
	}
	
	// 3. 기본값 반환
	return DefaultVFXDuration;
}

FVector UGS_DebuffVFXComponent::GetVFXScale(EDebuffType DebuffType) const
{
	// 1. 먼저 개별 오버라이드 확인
	if (const FVector* OverrideScale = OverrideDebuffVFXScaleMap.Find(DebuffType))
	{
		return *OverrideScale;
	}
	
	// 2. 공통 설정에서 확인
	if (CommonVFXSettings)
	{
		if (const FVector* CommonScale = CommonVFXSettings->DebuffVFXScaleMap.Find(DebuffType))
		{
			return *CommonScale;
		}
	}
	
	// 3. 기본값 반환 (1, 1, 1)
	return FVector::OneVector;
} 