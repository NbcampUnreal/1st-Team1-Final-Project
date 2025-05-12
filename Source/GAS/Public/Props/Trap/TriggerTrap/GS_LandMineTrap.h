#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/SphereComponent.h"
#include "GS_LandMineTrap.generated.h"

class RadialForceComponent;
UCLASS()
class GAS_API AGS_LandMineTrap : public AGS_TrigTrapBase
{
	GENERATED_BODY()
	
	
public:
	AGS_LandMineTrap();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Trap")
	URadialForceComponent* RadialForceComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trap")
	USphereComponent* DamageSphereComp;


	UFUNCTION()
	void Explode();


	void HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors) override;
};
