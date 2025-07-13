#include "Props/Trap/TriggerTrap/GS_LandMineTrap.h"
#include "Character/Player/GS_Player.h"

AGS_LandMineTrap::AGS_LandMineTrap()
{
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForce"));
	RadialForceComp->SetupAttachment(MeshParentSceneComp);

	RadialForceComp->Radius = 300.0f;
	RadialForceComp->ImpulseStrength = 800.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->ForceStrength = 0.0f;

	DamageBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//범위 감지용 Sphere Component 추가
	DamageSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("DamageRangeSphere"));
	DamageSphereComp->SetupAttachment(MeshParentSceneComp);
	DamageSphereComp->SetSphereRadius(300.f);
	DamageSphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}


void AGS_LandMineTrap::Explode()
{
	if (HasAuthority())
	{
		DestroyTrapEffect();
		TArray<AActor*> OverlappingActors;
		DamageSphereComp->GetOverlappingActors(OverlappingActors, AGS_Player::StaticClass());
		HandleTrapAreaDamage(OverlappingActors);
		RadialForceComp->FireImpulse();
	}
}

void AGS_LandMineTrap::HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors)
{
	for (AActor* Actor : AffectedActors)
	{
		Super::HandleTrapDamage(Actor);
	}
}

void AGS_LandMineTrap::DestroyTrapEffect_Implementation()
{
}
