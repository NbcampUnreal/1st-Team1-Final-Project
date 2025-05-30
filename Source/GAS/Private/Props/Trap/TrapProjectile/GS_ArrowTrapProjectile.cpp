#include "Props/Trap/TrapProjectile/GS_ArrowTrapProjectile.h"
#include "Components/SphereComponent.h"
#include "Character/Player/GS_Player.h"
#include "Character/GS_Character.h"
#include "Props/Trap/TrapProjectile/GS_TrapVisualProjectile.h"
#include "Net/UnrealNetwork.h"


AGS_ArrowTrapProjectile::AGS_ArrowTrapProjectile()
{
}

void AGS_ArrowTrapProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(4.0f);
}


void AGS_ArrowTrapProjectile::Init(AGS_TrigTrapBase* InTrap)
{
	OwningTrap = InTrap;
	if (HasAuthority())
	{
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

	AGS_Player* Player = Cast<AGS_Player>(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("OtherComp: %s, Type: %d"), *OtherComp->GetName(), OtherComp->GetCollisionObjectType());

	if (Player && OtherComp == Player->GetMesh() && OwningTrap)
	{
		//수정
		OwningTrap->HandleTrapDamage(OtherActor);
		StickWithVisualOnly(SweepResult);
		Destroy();
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

	// 본 화살 제거
	Destroy();
}