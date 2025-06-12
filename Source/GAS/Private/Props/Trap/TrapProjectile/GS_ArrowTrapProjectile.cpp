#include "Props/Trap/TrapProjectile/GS_ArrowTrapProjectile.h"
#include "Components/SphereComponent.h"
#include "Character/Player/GS_Player.h"
#include "Character/GS_Character.h"
#include "Props/Trap/TrapProjectile/GS_TrapVisualProjectile.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Props/Trap/TrapProjectile/GS_ProjectilePoolComp.h"
#include "Net/UnrealNetwork.h"


AGS_ArrowTrapProjectile::AGS_ArrowTrapProjectile()
{
}

void AGS_ArrowTrapProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(4.0f);
	ProjectileMesh->SetSimulatePhysics(false);
	CollisionComponent->SetSimulatePhysics(false);
}


void AGS_ArrowTrapProjectile::Init(AGS_NonTrigTrapBase* InTrap)
{
	OwningTrap = InTrap;
	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AGS_ArrowTrapProjectile::OnBeginOverlap);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_ArrowTrapProjectile::OnBeginOverlap);
		CollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel3);
	}
}


void AGS_ArrowTrapProjectile::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this)
	{
		return;
	}

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (Seeker && OtherComp == Seeker->GetMesh() && OwningTrap)
	{
		//수정
		OwningTrap->HandleTrapDamage(OtherActor);
		StickWithVisualOnly(SweepResult);
	}
	if(!OtherActor->IsA<APawn>())
	{
		StickWithVisualOnly(SweepResult);
	}
}

void AGS_ArrowTrapProjectile::StickWithVisualOnly(const FHitResult& Hit)
{
	if (!ProjectileMesh)
	{
		return;
	}

	// 화살이 박힌 위치
	FVector SpawnLocation = Hit.ImpactPoint;

	FVector ArrowDirection = GetVelocity().GetSafeNormal();
	FRotator SpawnRotation = FRotationMatrix::MakeFromX(ArrowDirection).Rotator();

	FRotator AdjustedRotation = SpawnRotation + FRotator(0.f, -90.f, 0.f);

	// 약간 박히게 밀어넣기
	SpawnLocation = Hit.ImpactPoint - ArrowDirection * 1.f;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	

	AGS_TrapVisualProjectile* VisualArrow = GetWorld()->SpawnActor<AGS_TrapVisualProjectile>(AGS_TrapVisualProjectile::StaticClass(), SpawnLocation, AdjustedRotation, Params);
	
	if (!VisualArrow)
	{
		return;
	}
	
	if (VisualArrow)
	{
		VisualArrow->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);

	}

	if (OwningPool)
	{
		OwningPool->ReturnProjectile(this);
	}
}


bool AGS_ArrowTrapProjectile::IsReady() const
{
	//숨겨져 있으면 true를 반환 
	return IsHidden() && !IsPendingKillPending();
}

void AGS_ArrowTrapProjectile::ActivateProjectile(const FVector& SpawnLocation, const FRotator& Rotation, float Speed)
{
	
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
	
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
		ProjectileMovementComponent->UpdateComponentVelocity();
	}

	SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(Rotation);
	
	if (ProjectileMovementComponent)
	{
		const FVector Direction = Rotation.Vector();
		ProjectileMovementComponent->Velocity = Direction * Speed;
		ProjectileMovementComponent->Activate();
	}

	SetActorEnableCollision(true);
	SetLifeSpan(4.0f);
	//화살 효과 관련 blueprint native event
	OnActivateEffect();
}


void AGS_ArrowTrapProjectile::DeactivateProjectile()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->Deactivate();
		
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
		ProjectileMovementComponent->UpdateComponentVelocity();
		
	}
}



void AGS_ArrowTrapProjectile::OnActivateEffect_Implementation()
{

}
