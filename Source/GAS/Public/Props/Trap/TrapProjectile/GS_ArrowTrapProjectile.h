#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "GS_ArrowTrapProjectile.generated.h"


UCLASS()
class GAS_API AGS_ArrowTrapProjectile : public AGS_SeekerMerciArrow
{
	GENERATED_BODY()

public:
	AGS_ArrowTrapProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	AGS_TrigTrapBase* OwningTrap;

	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Trap")
	void Init(AGS_TrigTrapBase* InTrap);
};