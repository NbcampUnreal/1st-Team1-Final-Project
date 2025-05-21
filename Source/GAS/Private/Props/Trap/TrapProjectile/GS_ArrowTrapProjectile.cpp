#include "Props/Trap/TrapProjectile/GS_ArrowTrapProjectile.h"
#include "Components/SphereComponent.h"
#include "Character/Player/GS_Player.h"


AGS_ArrowTrapProjectile::AGS_ArrowTrapProjectile()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ArrowMeshObj(TEXT("/Game/Weapons/Bow_02_Quiver/SKM_Arrow_02.SKM_Arrow_02"));
	if (ArrowMeshObj.Succeeded())
	{
		ProjectileMesh->SetSkeletalMesh(ArrowMeshObj.Object);
	}
}

void AGS_ArrowTrapProjectile::Init(AGS_TrigTrapBase* InTrap)
{
	OwningTrap = InTrap;
	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_ArrowTrapProjectile::OnBeginOverlap);
	}
}


void AGS_ArrowTrapProjectile::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	AGS_Player* Player = Cast<AGS_Player>(OtherActor);
	if (Player && OwningTrap)
	{
		OwningTrap->HandleTrapDamage(OtherActor);
	}
	StickWithVisualOnly(SweepResult);
}

void AGS_ArrowTrapProjectile::StickWithVisualOnly(const FHitResult& Hit)
{
	if (!ProjectileMesh)
	{
		return;
	}

	// 화살이 박힌 위치
	FVector SpawnLocation = Hit.ImpactPoint;

	// 방향 = 화살이 실제 날아온 방향
	FVector ArrowDirection = GetVelocity().GetSafeNormal();
	FRotator SpawnRotation = FRotationMatrix::MakeFromX(ArrowDirection).Rotator();

	// 메시가 +Y 방향을 앞이라고 가정 시 Y축 기준 -90도 회전
	FRotator AdjustedRotation = SpawnRotation + FRotator(0.f, -90.f, 0.f);
	SpawnLocation -= ArrowDirection * 25.f; // 약간 파고들게

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	TSubclassOf<AGS_ArrowVisualActor> VisualArrowClass = AGS_ArrowVisualActor::StaticClass();

	if (VisualArrowClass)
	{
		AGS_ArrowVisualActor* VisualArrow = GetWorld()->SpawnActor<AGS_ArrowVisualActor>(VisualArrowClass, SpawnLocation, AdjustedRotation, Params);
		UE_LOG(LogTemp, Warning, TEXT("VisualArrowClass exist"));
		if (VisualArrow)
		{
			UE_LOG(LogTemp, Warning, TEXT("VisualArrow exist"));
			VisualArrow->SetArrowMesh(ProjectileMesh->GetSkeletalMeshAsset());
			VisualArrow->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No VisualArrow"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No VisualArrowClass"));
	}
	

	// 본 화살 제거
	Destroy();
}
