// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "Weapon/Projectile/Component/GS_ArrowFXComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Character/Skill/Seeker/GS_FieldSkillActor.h"
#include "AkGameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"

AGS_SeekerMerciArrow::AGS_SeekerMerciArrow()
{
	// 화살 FX 컴포넌트 생성 (VFX + Sound)
	ArrowFXComponent = CreateDefaultSubobject<UGS_ArrowFXComponent>(TEXT("ArrowFXComponent"));
	
	if (HasAuthority())
	{
		// 화살 스폰 직후
		this->SetActorEnableCollision(false);
	}
}

void AGS_SeekerMerciArrow::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		if (CollisionComponent)
		{
			CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_SeekerMerciArrow::OnBeginOverlap);

			// Overlap 설정 강화
			CollisionComponent->SetGenerateOverlapEvents(true);
			CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		this->SetActorEnableCollision(true);

		AActor* IgnoredActor = GetInstigator();
		if (IgnoredActor && CollisionComponent)
		{
			CollisionComponent->IgnoreActorWhenMoving(IgnoredActor, true);
		}
	}	
}

void AGS_SeekerMerciArrow::StickWithVisualOnly(const FHitResult& Hit)
{
	if (!ProjectileMesh)
	{
		return;
	}

	if (bAlreadyStuck)
	{
		return;
	}
	bAlreadyStuck = true;

	UE_LOG(LogTemp, Error, TEXT("=== StickWithVisualOnly Called ==="));
	UE_LOG(LogTemp, Error, TEXT("Hit Component: %s"), Hit.Component.IsValid() ? *Hit.Component->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Error, TEXT("Hit BoneName: %s"), *Hit.BoneName.ToString());

	// 화살이 박힌 위치 (Hit 결과 사용)
	FVector SpawnLocation = Hit.ImpactPoint;

	// 화살의 실제 속도 방향 사용
	FVector ArrowDirection = GetVelocity().GetSafeNormal();
	if (ArrowDirection.IsNearlyZero())
	{
		// 속도가 0에 가까우면 Forward Vector 사용
		ArrowDirection = GetActorForwardVector();
	}

	FRotator SpawnRotation = FRotationMatrix::MakeFromX(ArrowDirection).Rotator();

	// 화살이 표면에서 약간 파고들도록 위치 조정
	SpawnLocation += Hit.ImpactNormal * 20.0f; // ImpactNormal 방향으로 파고들기

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (VisualArrowClass && GetWorld())
	{
		AGS_ArrowVisualActor* VisualArrow = GetWorld()->SpawnActor<AGS_ArrowVisualActor>(
			VisualArrowClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

		if (VisualArrow)
		{
			VisualArrow->SetArrowMesh(ProjectileMesh->GetSkeletalMeshAsset());
			VisualArrow->SetAttachedTargetActor(Hit.GetActor());
			// Bone 이름이 있으면 해당 Bone에 Attach
			if (Hit.BoneName != NAME_None && Hit.Component.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Attaching arrow to bone: %s"), *Hit.BoneName.ToString());
				VisualArrow->AttachToComponent(
					Hit.Component.Get(),
					FAttachmentTransformRules::KeepWorldTransform,
					Hit.BoneName
				);
			}
			else if (Hit.Component.IsValid())
			{
				// Bone은 없지만 Component는 있는 경우
				UE_LOG(LogTemp, Warning, TEXT("Attaching arrow to component (no bone)"));
				VisualArrow->AttachToComponent(
					Hit.Component.Get(),
					FAttachmentTransformRules::KeepWorldTransform
				);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("No valid component to attach arrow to!"));
			}
		}
	}

	// 이동 멈춤
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
	}
	SetActorEnableCollision(false);

	// 원래 화살 제거
	Destroy();
}

void AGS_SeekerMerciArrow::Multicast_InitHomingTarget_Implementation(AActor* Target)
{
	if (!ProjectileMovementComponent)
	{
		return;
	}

	if(Target)
	{
		if (!Target || !IsValid(Target))
		{
			UE_LOG(LogTemp, Warning, TEXT("타겟이 유효하지 않거나 이미 제거됨. 화살 파괴."));
			Destroy();  // 유효하지 않으면 화살도 제거
			return;
		}

		UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Target->GetRootComponent());
		if (!RootPrim)
		{
			UE_LOG(LogTemp, Error, TEXT("HomingTargetComponent is not a primitive component!"));
			return;
		}

		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		ProjectileMovementComponent->HomingTargetComponent = RootPrim;
		ProjectileMovementComponent->bIsHomingProjectile = true;
		ProjectileMovementComponent->InitialSpeed = 3000.f;
		ProjectileMovementComponent->MaxSpeed = 3000.f;
		ProjectileMovementComponent->HomingAccelerationMagnitude = 20000.f;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
		ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileMovementComponent->InitialSpeed;

		HomingTarget = Target;
		if (AGS_Character* TargetCharacter = Cast<AGS_Character>(Target))
		{
			TargetCharacter->OnDeathDelegate.AddDynamic(this, &AGS_SeekerMerciArrow::OnTargetDied);
		}
		UE_LOG(LogTemp, Warning, TEXT("HomingTarget set to %s"), *Target->GetName());
	}
	else
	{
		// 유도 해제 : 일반 직선 화살로 설정
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		ProjectileMovementComponent->HomingTargetComponent = nullptr;
		ProjectileMovementComponent->bIsHomingProjectile = false;
		ProjectileMovementComponent->InitialSpeed = 5000.f;
		ProjectileMovementComponent->MaxSpeed = 5000.0f;
		ProjectileMovementComponent->ProjectileGravityScale = 1.0f;

		HomingTarget = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("HomingTarget is null — switching to normal arrow"));

	}

	// 방향성 초기화
	ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileMovementComponent->InitialSpeed;
}

// === 개선된 FindClosestBoneName 함수 ===
FName AGS_SeekerMerciArrow::FindClosestBoneName(USkeletalMeshComponent* MeshComp, const FVector& WorldLocation)
{
	if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
	{
		return NAME_None;
	}

	const FReferenceSkeleton& RefSkeleton = MeshComp->GetSkeletalMeshAsset()->GetRefSkeleton();
	const TArray<FMeshBoneInfo>& BoneInfos = RefSkeleton.GetRefBoneInfo();

	if (BoneInfos.Num() == 0)
	{
		return NAME_None;
	}

	FName ClosestBone = NAME_None;
	float ClosestDistance = FLT_MAX;

	// === 주요 본들만 우선 검사 (성능 최적화) ===
	TArray<FName> PriorityBones = {
		TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
		TEXT("head"), TEXT("neck_01"),
		TEXT("upperarm_l"), TEXT("upperarm_r"),
		TEXT("lowerarm_l"), TEXT("lowerarm_r"),
		TEXT("thigh_l"), TEXT("thigh_r"),
		TEXT("calf_l"), TEXT("calf_r"),
		TEXT("chest"), TEXT("pelvis")
	};

	// 1. 우선순위 본들부터 검사
	for (const FName& PriorityBone : PriorityBones)
	{
		if (MeshComp->GetBoneIndex(PriorityBone) != INDEX_NONE)
		{
			FVector BoneWorldLocation = MeshComp->GetBoneLocation(PriorityBone, EBoneSpaces::WorldSpace);
			float Distance = FVector::Dist(WorldLocation, BoneWorldLocation);

			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestBone = PriorityBone;
			}
		}
	}

	// 2. 우선순위 본에서 찾지 못했거나 거리가 너무 멀면 전체 본 검사
	if (ClosestBone == NAME_None || ClosestDistance > 50.0f)
	{
		for (int32 BoneIndex = 0; BoneIndex < BoneInfos.Num(); ++BoneIndex)
		{
			FName BoneName = BoneInfos[BoneIndex].Name;
			FVector BoneWorldLocation = MeshComp->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
			float Distance = FVector::Dist(WorldLocation, BoneWorldLocation);

			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestBone = BoneName;
			}
		}
	}

	return ClosestBone;
}

void AGS_SeekerMerciArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);
	UE_LOG(LogTemp, Error, TEXT("Arrow hit actor: %s"), *OtherActor->GetName());
	
	// 맞은 대상 구분
	ETargetType TargetType = DetermineTargetType(OtherActor);

	// 이펙트와 사운드 처리 (가상함수로 만들어 자식에서 오버라이드 가능)
	ProcessHitEffects(TargetType, SweepResult);

	// 서버에서만 데미지 및 로직 처리
	if (HasAuthority())
	{
		// 데미지 처리 (자식 클래스에서 구현)
		ProcessDamageLogic(TargetType, SweepResult, OtherActor);

		// 기존 타겟 타입별 처리 (박힘/파괴 등)
		HandleTargetTypeGeneric(TargetType, SweepResult);
	}
	bool bShouldContinueMovement = false;

	if (HasAuthority())
	{
		// HandleTargetTypeGeneric에서 관통 여부를 반환값으로 받음
		bShouldContinueMovement = HandleTargetTypeGeneric(TargetType, SweepResult);

		if (!IsValid(this))
		{
			UE_LOG(LogTemp, Warning, TEXT("Arrow marked for destruction, stopping processing"));
			return;
		}
	}

	// 관통하는 경우 이동 중지하지 않고 함수 종료
	if (bShouldContinueMovement)
	{
		UE_LOG(LogTemp, Warning, TEXT("Arrow penetrating, continuing movement"));
		return;
	}

	// === 여기까지 왔다는 것은 관통하지 않는다는 뜻 ===
	bAlreadyHit = true;

	// 이동 중지 (관통하지 않는 경우에만 실행됨)
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
	}

	// 충돌 비활성화
	SetActorEnableCollision(false);

	// 박힘 처리를 위한 SkeletalMesh 검사 및 처리
	ProcessStickLogic(OtherActor, TargetType, SweepResult);
}

ETargetType AGS_SeekerMerciArrow::DetermineTargetType(AActor* OtherActor) const
{
	if (Cast<AGS_Monster>(OtherActor))
	{
		return ETargetType::DungeonMonster;
	}
	else if (Cast<AGS_Guardian>(OtherActor))
	{
		return ETargetType::Guardian;
	}
	else if (Cast<AGS_Seeker>(OtherActor))
	{
		return ETargetType::Seeker;
	}
	else if (Cast<AGS_SeekerMerciArrow>(OtherActor) || Cast<AGS_FieldSkillActor>(OtherActor))
	{
		return ETargetType::Skill;
	}
	else
	{
		return ETargetType::Structure;
	}
}

bool AGS_SeekerMerciArrow::HandleTargetTypeGeneric(ETargetType TargetType, const FHitResult& SweepResult)
{
	switch (TargetType)
	{
	case ETargetType::Skill:
		break;
	case ETargetType::Structure:
		StickWithVisualOnly(SweepResult);
		break;
	case ETargetType::Seeker:
		break;
	default:
		break;
	}

	return false; // 기본적으로는 이동 중지 (박힘)
}

void AGS_SeekerMerciArrow::ProcessHitEffects(ETargetType TargetType, const FHitResult& SweepResult)
{
	//Crosshair Hit Anim
	if (TargetType == ETargetType::Guardian || TargetType == ETargetType::DungeonMonster)
	{
		if (AGS_Merci* MerciPlayer = Cast<AGS_Merci>(GetOwner()))
		{
			MerciPlayer->Client_ShowCrosshairHitFeedback();
			MerciPlayer->Client_PlayHitFeedbackSound();
		}
	}

	// 히트 사운드 & VFX 재생 (컴포넌트로 위임)
	if (ArrowFXComponent)
	{
		ArrowFXComponent->PlayHitSound(TargetType, SweepResult);
		ArrowFXComponent->PlayHitVFX(TargetType, SweepResult);
	}
}

void AGS_SeekerMerciArrow::ProcessDamageLogic(ETargetType TargetType, const FHitResult& SweepResult, AActor* HitActor)
{
}

void AGS_SeekerMerciArrow::ProcessStickLogic(AActor* HitActor, ETargetType TargetType, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("=== ProcessStickLogic Debug ==="));
	UE_LOG(LogTemp, Warning, TEXT("Arrow Location: %s"), *GetActorLocation().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Arrow Velocity: %s"), *GetVelocity().ToString());
	UE_LOG(LogTemp, Warning, TEXT("SweepResult Valid: %s"), SweepResult.bBlockingHit ? TEXT("True") : TEXT("False"));
	if (SweepResult.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("SweepResult Impact: %s"), *SweepResult.ImpactPoint.ToString());
		UE_LOG(LogTemp, Warning, TEXT("SweepResult Normal: %s"), *SweepResult.ImpactNormal.ToString());
	}

	// 캐릭터의 SkeletalMeshComponent 찾기
	USkeletalMeshComponent* TargetMesh = Cast<USkeletalMeshComponent>(HitActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));

	// 로그로 어떤 컴포넌트가 들어오는지 확인
	UE_LOG(LogTemp, Warning, TEXT("OtherComp Type: %s"), SweepResult.Component.IsValid() ? *SweepResult.Component->GetClass()->GetName() : TEXT("NULL"));

	if (TargetMesh)
	{
		FHitResult MeshHit;
		FVector ArrowLocation = GetActorLocation();

		// === 화살 방향 결정 (개선된 방법) ===
		FVector ArrowDirection;

		// 1. SweepResult의 ImpactNormal 활용 (가장 신뢰할 수 있음)
		if (SweepResult.bBlockingHit && !SweepResult.ImpactNormal.IsZero())
		{
			ArrowDirection = -SweepResult.ImpactNormal;
		}
		// 2. 현재 속도 사용
		else if (!GetVelocity().IsZero())
		{
			ArrowDirection = GetVelocity().GetSafeNormal();
		}
		// 3. 폴백: 화살의 Forward 벡터
		else
		{
			ArrowDirection = GetActorForwardVector();
		}

		// === Trace 설정 (더 넓은 범위로) ===
		float TraceDistance = 200.f; // 더 긴 거리

		// 화살 위치 기준으로 앞뒤로 충분히 검사
		FVector TraceStart = ArrowLocation - (ArrowDirection * 100.f);
		FVector TraceEnd = ArrowLocation + (ArrowDirection * 100.f);

		// === LineTrace 시도 ===
		FCollisionQueryParams ComponentTraceParams;
		ComponentTraceParams.bTraceComplex = true;
		ComponentTraceParams.bReturnPhysicalMaterial = false;
		ComponentTraceParams.AddIgnoredActor(this);
		ComponentTraceParams.AddIgnoredActor(GetOwner());
		ComponentTraceParams.AddIgnoredActor(GetInstigator());

		bool bHit = TargetMesh->LineTraceComponent(
			MeshHit,
			TraceStart,
			TraceEnd,
			ComponentTraceParams
		);

		// === Component Trace 실패 시 World Trace로 폴백 ===
		if (!bHit)
		{
			FCollisionQueryParams WorldTraceParams = ComponentTraceParams;
			// Capsule이나 다른 collision component 무시
			if (SweepResult.Component.IsValid())
			{
				WorldTraceParams.AddIgnoredComponent(SweepResult.Component.Get());
			}

			bHit = GetWorld()->LineTraceSingleByChannel(
				MeshHit,
				TraceStart,
				TraceEnd,
				ECC_Pawn, // 또는 커스텀 채널
				WorldTraceParams
			);

			// Hit한 Component가 TargetMesh가 아니면 무효
			if (bHit && MeshHit.Component.Get() != TargetMesh)
			{
				bHit = false;
			}
		}

		// === Hit 결과 처리 ===
		if (bHit && MeshHit.Component.Get() == TargetMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("Arrow hit mesh successfully! Bone: %s, Impact: %s"),
				*MeshHit.BoneName.ToString(),
				*MeshHit.ImpactPoint.ToString());
#if WITH_EDITOR
			FVector CapsuleHitLocation = SweepResult.ImpactPoint;
			FVector MeshHitLocation = MeshHit.ImpactPoint;
			Client_DrawDebugLineTest(CapsuleHitLocation, MeshHitLocation, FColor::Yellow);
			Client_DrawDebugSphereTest(CapsuleHitLocation, FColor::Red);
			Client_DrawDebugSphereTest(MeshHitLocation, FColor::Green);
#endif

			StickWithVisualOnly(MeshHit);
		}
		else
		{
			// === 폴백: 수동으로 정확한 Hit 결과 생성 ===
			FHitResult FallbackHit = CreateFallbackHitResult(TargetMesh, HitActor, ArrowLocation, ArrowDirection, SweepResult);

			UE_LOG(LogTemp, Warning, TEXT("Using fallback hit result. Bone: %s, Impact: %s"),
				*FallbackHit.BoneName.ToString(),
				*FallbackHit.ImpactPoint.ToString());
			StickWithVisualOnly(FallbackHit);
		}
	}
	else
	{
		// SkeletalMesh가 없는 경우 기본 처리
		UE_LOG(LogTemp, Warning, TEXT("No SkeletalMesh found, using sweep result"));
		StickWithVisualOnly(SweepResult);
	}
}

// === 헬퍼 함수: 폴백 Hit 결과 생성 ===
FHitResult AGS_SeekerMerciArrow::CreateFallbackHitResult(USkeletalMeshComponent* TargetMesh, AActor* HitActor,
	const FVector& ArrowLocation, const FVector& ArrowDirection,
	const FHitResult& OriginalSweepResult)
{
	FHitResult FallbackHit;

	// === 기본 정보 설정 ===
	FallbackHit.Component = TargetMesh;
	FallbackHit.HitObjectHandle = FActorInstanceHandle(HitActor);
	FallbackHit.bBlockingHit = true;

	// === Impact 위치 결정 ===
	FVector ImpactPoint;
	FVector ImpactNormal;

	if (OriginalSweepResult.bBlockingHit)
	{
		// SweepResult가 유효하면 그 정보 사용
		ImpactPoint = OriginalSweepResult.ImpactPoint;
		ImpactNormal = OriginalSweepResult.ImpactNormal;
	}
	else
	{
		// SweepResult가 없으면 화살 위치 기준으로 계산
		ImpactPoint = ArrowLocation;
		ImpactNormal = -ArrowDirection;

		// TargetMesh의 바운딩 박스를 이용해 더 정확한 위치 추정
		FBox MeshBounds = TargetMesh->Bounds.GetBox();
		if (MeshBounds.IsValid)
		{
			// 화살 위치를 바운딩 박스 표면으로 클램프
			ImpactPoint = MeshBounds.GetClosestPointTo(ArrowLocation);
		}
	}

	FallbackHit.ImpactPoint = ImpactPoint;
	FallbackHit.ImpactNormal = ImpactNormal;
	FallbackHit.Distance = FVector::Dist(ArrowLocation, ImpactPoint);

	// === 가장 가까운 Bone 찾기 ===
	FallbackHit.BoneName = FindClosestBoneName(TargetMesh, ImpactPoint);

	return FallbackHit;
}

void AGS_SeekerMerciArrow::Client_DrawDebugLineTest_Implementation(FVector Start, FVector End, FColor Color)
{
	DrawDebugLine(GetWorld(), Start, End, Color, false, 3.0f, 0, 2.0f);
}

void AGS_SeekerMerciArrow::Client_DrawDebugSphereTest_Implementation(FVector Location, FColor Color)
{
	DrawDebugSphere(GetWorld(), Location, 5.0f, 12, Color, false, 3.0f);
}

void AGS_SeekerMerciArrow::OnTargetDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == HomingTarget)
	{
		Destroy();  // 화살 제거
	}
}

void AGS_SeekerMerciArrow::OnTargetDied()
{
	Destroy();
}