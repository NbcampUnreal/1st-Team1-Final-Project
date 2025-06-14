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
	// 서버에서만 멀티캐스트 호출
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// 이미 해당 디버프 VFX가 재생 중이면 갱신
	if (IsDebuffVFXActive(DebuffType))
	{
		RemoveDebuffVFX(DebuffType);
	}

	// VFX 재생 위치와 스케일 설정
	FVector SpawnLocation = GetOwner()->GetActorLocation();
	FVector VFXScale = GetVFXScale(DebuffType);
	
	// 멀티캐스트로 모든 클라이언트에서 VFX 재생
	Multicast_PlayDebuffVFX(DebuffType, SpawnLocation, VFXScale);
	
	UE_LOG(LogTemp, Log, TEXT("%s: %s 디버프 VFX 재생"), 
		*GetOwner()->GetName(), 
		*UEnum::GetValueAsString(DebuffType));
}

void UGS_DebuffVFXComponent::RemoveDebuffVFX(EDebuffType DebuffType)
{
	// 서버에서만 멀티캐스트 호출
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	Multicast_RemoveDebuffVFX(DebuffType);
	
	UE_LOG(LogTemp, Log, TEXT("%s: %s 디버프 VFX 제거"), 
		*GetOwner()->GetName(), 
		*UEnum::GetValueAsString(DebuffType));
}

void UGS_DebuffVFXComponent::RemoveAllDebuffVFX()
{
	// 모든 활성 VFX 제거
	TArray<EDebuffType> ActiveTypes;
	ActiveVFXComponents.GetKeys(ActiveTypes);
	
	for (EDebuffType Type : ActiveTypes)
	{
		if (GetOwner()->HasAuthority())
		{
			Multicast_RemoveDebuffVFX(Type);
		}
		else
		{
			// 로컬에서 직접 제거 (서버가 아닌 경우)
			if (UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(Type))
			{
				VFXComp->DestroyComponent();
			}
			ActiveVFXComponents.Remove(Type);
			
			if (FTimerHandle* TimerHandle = VFXTimerHandles.Find(Type))
			{
				GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
				VFXTimerHandles.Remove(Type);
			}
		}
	}
}

bool UGS_DebuffVFXComponent::IsDebuffVFXActive(EDebuffType DebuffType) const
{
	UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(DebuffType);
	return VFXComp && IsValid(VFXComp);
}

void UGS_DebuffVFXComponent::Multicast_PlayDebuffVFX_Implementation(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale)
{
	// 해당 타입의 VFX 시스템 가져오기
	UNiagaraSystem* VFXSystem = DebuffVFXMap.FindRef(DebuffType);
	if (!VFXSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: %s 디버프 VFX가 설정되지 않음"), 
			*GetOwner()->GetName(), 
			*UEnum::GetValueAsString(DebuffType));
		return;
	}

	// 이전 VFX가 있다면 제거
	if (UNiagaraComponent* ExistingVFX = ActiveVFXComponents.FindRef(DebuffType))
	{
		ExistingVFX->DestroyComponent();
		ActiveVFXComponents.Remove(DebuffType);
	}

	// 기존 타이머 클리어
	if (FTimerHandle* ExistingTimer = VFXTimerHandles.Find(DebuffType))
	{
		GetWorld()->GetTimerManager().ClearTimer(*ExistingTimer);
		VFXTimerHandles.Remove(DebuffType);
	}

	// 새로운 VFX 컴포넌트 생성 (오너에 부착)
	UNiagaraComponent* NewVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		VFXSystem,
		GetOwner()->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true, // Auto Destroy
		true, // Auto Activate
		ENCPoolMethod::None
	);

	if (NewVFXComponent)
	{
		// 스케일 적용
		if (Scale != FVector::ZeroVector)
		{
			NewVFXComponent->SetWorldScale3D(Scale);
		}

		// 활성 VFX 목록에 추가
		ActiveVFXComponents.Add(DebuffType, NewVFXComponent);

		// 지속 시간 후 자동 제거 타이머 설정
		float Duration = GetVFXDuration(DebuffType);
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this, DebuffType]()
			{
				RemoveVFXTimerCallback(DebuffType);
			},
			Duration,
			false
		);
		VFXTimerHandles.Add(DebuffType, TimerHandle);

		UE_LOG(LogTemp, Log, TEXT("%s: %s 디버프 VFX 생성됨, %f초 후 자동 제거"), 
			*GetOwner()->GetName(), 
			*UEnum::GetValueAsString(DebuffType), 
			Duration);
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

	// 타이머 제거
	if (FTimerHandle* TimerHandle = VFXTimerHandles.Find(DebuffType))
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
		VFXTimerHandles.Remove(DebuffType);
	}
}

void UGS_DebuffVFXComponent::RemoveVFXTimerCallback(EDebuffType DebuffType)
{
	// 타이머에 의한 자동 제거
	if (UNiagaraComponent* VFXComp = ActiveVFXComponents.FindRef(DebuffType))
	{
		VFXComp->DestroyComponent();
		ActiveVFXComponents.Remove(DebuffType);
	}
	VFXTimerHandles.Remove(DebuffType);
	
	UE_LOG(LogTemp, Log, TEXT("%s: %s 디버프 VFX 타이머에 의해 제거됨"), 
		*GetOwner()->GetName(), 
		*UEnum::GetValueAsString(DebuffType));
}

float UGS_DebuffVFXComponent::GetVFXDuration(EDebuffType DebuffType) const
{
	if (const float* Duration = DebuffVFXDurationMap.Find(DebuffType))
	{
		return *Duration;
	}
	return DefaultVFXDuration;
}

FVector UGS_DebuffVFXComponent::GetVFXScale(EDebuffType DebuffType) const
{
	if (const FVector* Scale = DebuffVFXScaleMap.Find(DebuffType))
	{
		return *Scale;
	}
	return FVector(1.0f, 1.0f, 1.0f); // 기본 스케일
} 