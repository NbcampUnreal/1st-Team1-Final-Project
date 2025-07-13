#include "Props/Trap/TrapProjectile/GS_ProjectilePoolComp.h"


UGS_ProjectilePoolComp::UGS_ProjectilePoolComp()
{
}


void UGS_ProjectilePoolComp::BeginPlay()
{
	Super::BeginPlay();
}

//PoolSize만큼 projectile 미리 생성하여 pull에 저장
void UGS_ProjectilePoolComp::Initialize(TSubclassOf<AGS_ArrowTrapProjectile> InProjectileClass, int32 PoolSize)
{
	ProjectileClass = InProjectileClass;

	if (!ProjectileClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}


	for (int32 i = 0; i < PoolSize; ++i)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;
		
		AGS_ArrowTrapProjectile* Projectile = World->SpawnActor<AGS_ArrowTrapProjectile>(ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (Projectile)
		{
			//여기가 pool 에 저장되어 있는 상태
			Projectile->SetActorEnableCollision(false);
			Projectile->SetActorHiddenInGame(true);
			Projectile->SetLifeSpan(0); //풀에서 관리하므로 lifeSpan 제거
			ProjectilePool.Add(Projectile);
		}
	}
}

//풀에 있던 Projectile 꺼냄
AGS_ArrowTrapProjectile* UGS_ProjectilePoolComp::GetProjectile()
{

	AGS_ArrowTrapProjectile* SelectedProjectile = nullptr;

	for (AGS_ArrowTrapProjectile* Projectile : ProjectilePool)
	{
		if (Projectile && Projectile->IsReady())
		{
			SelectedProjectile = Projectile;
			break;
		}
	}

	if (SelectedProjectile)
	{
		ProjectilePool.Remove(SelectedProjectile);
		SelectedProjectile->OwningPool = this;
	}

	return SelectedProjectile;
}

//다시 풀에 저장
void UGS_ProjectilePoolComp::ReturnProjectile(AGS_ArrowTrapProjectile* Projectile)
{
	if (!Projectile)
	{
		return;
	}
	Projectile->DeactivateProjectile();

	if (ProjectilePool.Contains(Projectile))
	{
		return;
	}

	ProjectilePool.Add(Projectile);

}
