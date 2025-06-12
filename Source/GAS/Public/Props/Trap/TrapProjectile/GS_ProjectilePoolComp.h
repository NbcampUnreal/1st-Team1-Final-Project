#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_ArrowTrapProjectile.h"
#include "GS_ProjectilePoolComp.generated.h"


UCLASS( ClassGroup=(Trap), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_ProjectilePoolComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_ProjectilePoolComp();

protected:

	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable)
	void Initialize(TSubclassOf<AGS_ArrowTrapProjectile> InProjectileClass, int32 PoolSize);
	
	UFUNCTION(BlueprintCallable)
	AGS_ArrowTrapProjectile* GetProjectile();
	
	UFUNCTION(BlueprintCallable)
	void ReturnProjectile(AGS_ArrowTrapProjectile* Projectile);

private:
	TSet<AGS_ArrowTrapProjectile*> ProjectilePool;
	TSubclassOf<AGS_ArrowTrapProjectile> ProjectileClass;
};
