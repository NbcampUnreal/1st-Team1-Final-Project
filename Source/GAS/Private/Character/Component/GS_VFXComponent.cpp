// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Component/GS_VFXComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGS_VFXComponent::UGS_VFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); // VFX 멀티캐스트를 위해 복제 활성화

	// 기본값 설정
	DefaultVFXDuration = 5.0f;
}

void UGS_VFXComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGS_VFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 VFX 정리
	RemoveAllDebuffVFX();
	Super::EndPlay(EndPlayReason);
}

void UGS_VFXComponent::PlayDebuffVFX(EDebuffType DebuffType)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FVector Offset = GetVFXOffset(DebuffType);
	FVector SpawnLocation = GetOwner()->GetActorLocation() + Offset;
	FVector VFXScale = GetVFXScale(DebuffType);

	Multicast_PlayDebuffVFX(DebuffType, SpawnLocation, VFXScale);

}

void UGS_VFXComponent::RemoveDebuffVFX(EDebuffType DebuffType)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	Multicast_RemoveDebuffVFX(DebuffType);

}

void UGS_VFXComponent::PlayDebuffExpireVFX(EDebuffType DebuffType)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FVector Offset = GetExpireVFXOffset(DebuffType);
	FVector SpawnLocation = GetOwner()->GetActorLocation() + Offset;
	FVector VFXScale = GetVFXScale(DebuffType);

	Multicast_PlayDebuffExpireVFX(DebuffType, SpawnLocation, VFXScale);

}

void UGS_VFXComponent::RemoveAllDebuffVFX()
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

bool UGS_VFXComponent::IsDebuffVFXActive(EDebuffType DebuffType) const
{
	UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(DebuffType);
	return VFXComp && IsValid(VFXComp);
}

UNiagaraSystem* UGS_VFXComponent::GetDebuffVFX(EDebuffType DebuffType) const
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

UNiagaraSystem* UGS_VFXComponent::GetDebuffExpireVFX(EDebuffType DebuffType) const
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

void UGS_VFXComponent::Multicast_PlayDebuffVFX_Implementation(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale)
{
	// Dedicated Server에서는 VFX 생성 안 함
	if (GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

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

	// 새 VFX 생성 - 캐릭터에 어태치 (로컬 위치 사용)
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	USceneComponent* AttachComponent = OwnerCharacter ? (USceneComponent*)OwnerCharacter->GetMesh() : GetOwner()->GetRootComponent();

	UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		VFXSystem,
		AttachComponent,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Scale,
		EAttachLocation::KeepRelativeOffset,
		false,                    // bAutoDestroy - TMap에 저장하므로 false
		ENCPoolMethod::AutoRelease, // Pooling 활성화
		true,                      // bAutoActivate
		true                       // bPreCullCheck
	);

	if (VFXComponent)
	{
		ActiveVFXComponents.Add(DebuffType, VFXComponent);
	}
}

void UGS_VFXComponent::Multicast_RemoveDebuffVFX_Implementation(EDebuffType DebuffType)
{
	// VFX 컴포넌트 제거
	if (UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(DebuffType))
	{
		VFXComp->DestroyComponent();
		ActiveVFXComponents.Remove(DebuffType);
	}
}

void UGS_VFXComponent::Multicast_PlayDebuffExpireVFX_Implementation(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale)
{
	// Dedicated Server에서는 VFX 생성 안 함
	if (GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UNiagaraSystem* ExpireVFXSystem = GetDebuffExpireVFX(DebuffType);
	if (!ExpireVFXSystem)
	{
		return;
	}

	// 만료 VFX는 일회성이므로 월드에 스폰 (Pooling 활성화)
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ExpireVFXSystem,
		SpawnLocation,
		FRotator::ZeroRotator,
		Scale,
		true,                      // bAutoDestroy
		true,                      // bAutoActivate
		ENCPoolMethod::AutoRelease, // Pooling 활성화
		true                       // bPreCullCheck
	);
}

float UGS_VFXComponent::GetVFXDuration(EDebuffType DebuffType) const
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

FVector UGS_VFXComponent::GetVFXScale(EDebuffType DebuffType) const
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

FVector UGS_VFXComponent::GetVFXOffset(EDebuffType DebuffType) const
{
	// 1. 먼저 개별 오버라이드 확인
	if (const FVector* OverrideOffset = OverrideDebuffVFXOffsetMap.Find(DebuffType))
	{
		return *OverrideOffset;
	}

	// 2. 공통 설정에서 확인
	if (CommonVFXSettings)
	{
		if (const FVector* CommonOffset = CommonVFXSettings->DebuffVFXOffsetMap.Find(DebuffType))
		{
			return *CommonOffset;
		}
	}

	// 3. 기본값 반환 (Z축으로 50 유닛 위)
	return FVector(0.f, 0.f, 50.f);
}

FVector UGS_VFXComponent::GetExpireVFXOffset(EDebuffType DebuffType) const
{
	// 1. 먼저 개별 오버라이드 확인
	if (const FVector* OverrideOffset = OverrideDebuffExpireVFXOffsetMap.Find(DebuffType))
	{
		return *OverrideOffset;
	}

	// 2. 공통 설정에서 확인
	if (CommonVFXSettings)
	{
		if (const FVector* CommonOffset = CommonVFXSettings->DebuffExpireVFXOffsetMap.Find(DebuffType))
		{
			return *CommonOffset;
		}
	}

	// 3. 기본값 반환 (Z축으로 50 유닛 위)
	return FVector(0.f, 0.f, 50.f);
}

void UGS_VFXComponent::PlayOneShotVFX(UNiagaraSystem* VFXSystem, FVector LocationOffset, FVector Scale)
{
	if (!VFXSystem || !GetOwner())
	{
		return;
	}

	// 서버에서만 멀티캐스트 호출
	if (GetOwner()->HasAuthority())
	{
		Multicast_PlayOneShotVFX(VFXSystem, LocationOffset, Scale);
	}
}

void UGS_VFXComponent::Multicast_PlayOneShotVFX_Implementation(UNiagaraSystem* VFXSystem, FVector LocationOffset, FVector Scale)
{
	// Dedicated Server에서는 VFX 생성 안 함
	if (GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!VFXSystem || !GetOwner())
	{
		return;
	}

	// 로컬 위치 계산 (각 클라이언트에서 자신의 액터 위치 기준)
	FVector SpawnLocation = GetOwner()->GetActorLocation() + LocationOffset;

	// 모든 클라이언트에서 VFX 재생 (Pooling 활성화)
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFXSystem,
		SpawnLocation,
		FRotator::ZeroRotator,
		Scale,
		true,                      // bAutoDestroy
		true,                      // bAutoActivate
		ENCPoolMethod::AutoRelease, // Pooling 활성화
		true                       // bPreCullCheck
	);
}
