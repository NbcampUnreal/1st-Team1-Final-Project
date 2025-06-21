// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Projectile/Component/GS_ArrowFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "AkGameplayStatics.h"

UGS_ArrowFXComponent::UGS_ArrowFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// 기본값 설정
	NormalArrowTrailVFX = nullptr;
	AxeArrowTrailVFX = nullptr;
	ChildArrowTrailVFX = nullptr;
	ActiveTrailVFXComponent = nullptr;
	OwnerActor = nullptr;
	HitPawnVFX = nullptr;
	HitStructureVFX = nullptr;
	HitPawnSoundEvent = nullptr;
	HitStructureSoundEvent = nullptr;
	
	// 네트워크 복제 설정
	SetIsReplicatedByDefault(true);
}

void UGS_ArrowFXComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerActor = GetOwner();
	
	// 화살이 스폰될 때 자동으로 트레일 VFX를 시작하지 않도록 변경
	// if (OwnerActor && OwnerActor->HasAuthority())
	// {
	// 	Multicast_StartArrowTrailVFX(EArrowType::Normal);
	// }
}

void UGS_ArrowFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// VFX 정리
	if (ActiveTrailVFXComponent)
	{
		ActiveTrailVFXComponent->DestroyComponent();
		ActiveTrailVFXComponent = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void UGS_ArrowFXComponent::StartArrowTrailVFX(EArrowType ArrowType)
{
	if (OwnerActor && OwnerActor->HasAuthority())
	{
		Multicast_StartArrowTrailVFX(ArrowType);
	}
}

void UGS_ArrowFXComponent::StopArrowTrailVFX()
{
	if (OwnerActor && OwnerActor->HasAuthority())
	{
		Multicast_StopArrowTrailVFX();
	}
}

void UGS_ArrowFXComponent::PlayHitVFX(ETargetType TargetType, const FHitResult& SweepResult)
{
	if (OwnerActor && OwnerActor->HasAuthority())
	{
		Multicast_PlayHitVFX(TargetType, SweepResult);
	}
}

void UGS_ArrowFXComponent::PlayHitSound(ETargetType TargetType, const FHitResult& SweepResult)
{
	if (OwnerActor && OwnerActor->HasAuthority())
	{
		Multicast_PlayHitSound(TargetType, SweepResult);
	}
}

void UGS_ArrowFXComponent::SetArrowType(EArrowType ArrowType)
{
	// 기존 VFX 중지하고 새 타입으로 시작
	StopArrowTrailVFX();
	StartArrowTrailVFX(ArrowType);
}

void UGS_ArrowFXComponent::Multicast_StartArrowTrailVFX_Implementation(EArrowType ArrowType)
{
	if (!OwnerActor)
	{
		return;
	}

	// 화살 타입에 따른 VFX 선택
	UNiagaraSystem* SelectedVFXSystem = nullptr;
	switch (ArrowType)
	{
	case EArrowType::Normal:
		SelectedVFXSystem = NormalArrowTrailVFX;
		break;
	case EArrowType::Axe:
		SelectedVFXSystem = AxeArrowTrailVFX;
		break;
	case EArrowType::Child:
		SelectedVFXSystem = ChildArrowTrailVFX;
		break;
	default:
		SelectedVFXSystem = NormalArrowTrailVFX; // 기본값
		break;
	}

	if (!SelectedVFXSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("No VFX System assigned for arrow type: %d"), static_cast<int32>(ArrowType));
		return;
	}
	
	// 이미 활성화된 VFX가 있으면 중지
	if (ActiveTrailVFXComponent)
	{
		ActiveTrailVFXComponent->DestroyComponent();
		ActiveTrailVFXComponent = nullptr;
	}
	
	// 새 나이아가라 컴포넌트 생성
	ActiveTrailVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SelectedVFXSystem,
		OwnerActor->GetRootComponent(),
		NAME_None, // 소켓 이름 (없으면 루트에 부착)
		TrailVFXLocationOffset,
		TrailVFXRotationOffset,
		TrailVFXScale,
		EAttachLocation::KeepRelativeOffset,
		true, // Auto Destroy
		ENCPoolMethod::None,
		true,  // Auto Activate
		true   // Pre Cull Check
	);
	
	UE_LOG(LogTemp, Log, TEXT("Arrow Trail VFX Started for type: %d"), static_cast<int32>(ArrowType));
}

void UGS_ArrowFXComponent::Multicast_StopArrowTrailVFX_Implementation()
{
	if (ActiveTrailVFXComponent)
	{
		ActiveTrailVFXComponent->Deactivate();
		ActiveTrailVFXComponent->DestroyComponent();
		ActiveTrailVFXComponent = nullptr;
		
		UE_LOG(LogTemp, Log, TEXT("Arrow Trail VFX Stopped"));
	}
}

void UGS_ArrowFXComponent::Multicast_PlayHitVFX_Implementation(ETargetType TargetType, const FHitResult& SweepResult)
{
	UNiagaraSystem* VFXToPlay = nullptr;

	switch (TargetType)
	{
	case ETargetType::Guardian:
	case ETargetType::DungeonMonster:
		// 적 캐릭터(가디언, 던전몬스터)에 맞았을 때 VFX
		VFXToPlay = HitPawnVFX;
		break;
	case ETargetType::Structure:
		// 벽이나 구조물에 맞았을 때 VFX
		VFXToPlay = HitStructureVFX;
		break;
	case ETargetType::Seeker:
	case ETargetType::Skill:
		// 아군 시커나 스킬에 맞았을 때는 VFX 없음
		break;
	default:
		break;
	}

	// VFX 재생
	if (VFXToPlay && OwnerActor && OwnerActor->GetWorld())
	{
		// 히트 포인트에서 VFX 재생
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			OwnerActor->GetWorld(),
			VFXToPlay,
			SweepResult.ImpactPoint,
			SweepResult.ImpactNormal.Rotation(), // 히트 표면의 법선 방향으로 VFX 회전
			FVector(1.0f), // Scale
			true, // Auto Destroy
			true  // Auto Activate
		);
		
		UE_LOG(LogTemp, Log, TEXT("Arrow Hit VFX Played"));
	}
}

void UGS_ArrowFXComponent::Multicast_PlayHitSound_Implementation(ETargetType TargetType, const FHitResult& SweepResult)
{
	UAkAudioEvent* SoundEventToPlay = nullptr;

	switch (TargetType)
	{
	case ETargetType::Guardian:
	case ETargetType::DungeonMonster:
		// 적 캐릭터(가디언, 던전몬스터)에 맞았을 때 Wwise 이벤트
		SoundEventToPlay = HitPawnSoundEvent;
		break;
	case ETargetType::Structure:
		// 벽이나 구조물에 맞았을 때 Wwise 이벤트
		SoundEventToPlay = HitStructureSoundEvent;
		break;
	case ETargetType::Seeker:
	case ETargetType::Skill:
		// 아군 시커나 스킬에 맞았을 때는 사운드 없음
		break;
	default:
		break;
	}

	// Wwise 사운드 재생
	if (SoundEventToPlay && OwnerActor && OwnerActor->GetWorld())
	{
		UAkGameplayStatics::PostEventAtLocation(
			SoundEventToPlay,
			SweepResult.ImpactPoint,
			FRotator::ZeroRotator,
			OwnerActor->GetWorld()
		);
		
		UE_LOG(LogTemp, Log, TEXT("Arrow Hit Sound Played"));
	}
} 