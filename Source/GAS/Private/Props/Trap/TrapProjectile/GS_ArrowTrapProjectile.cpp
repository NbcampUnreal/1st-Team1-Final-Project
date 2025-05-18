#include "Props/Trap/TrapProjectile/GS_ArrowTrapProjectile.h"
#include "Components/SphereComponent.h"
#include "Character/Player/GS_Player.h"


AGS_ArrowTrapProjectile::AGS_ArrowTrapProjectile()
{

}


void AGS_ArrowTrapProjectile::Init(AGS_TrigTrapBase* InTrap)
{
	OwningTrap = InTrap;
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_ArrowTrapProjectile::OnBeginOverlap);
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
		StickWithVisualOnly(SweepResult);
	}
}
