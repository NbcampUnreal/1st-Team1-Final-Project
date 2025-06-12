#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "GS_ArrowTrapProjectile.generated.h"

class UGS_ProjectilePoolComp;
UCLASS()
class GAS_API AGS_ArrowTrapProjectile : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_ArrowTrapProjectile();

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ArrowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	AGS_NonTrigTrapBase* OwningTrap;

	UPROPERTY()
	TObjectPtr<UGS_ProjectilePoolComp> OwningPool;


	UFUNCTION(BlueprintCallable, Category = "Trap")
	void Init(AGS_NonTrigTrapBase* InTrap);

	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void ActivateProjectile(const FVector& SpawnLocation, const FRotator& Rotation, float Speed);
	UFUNCTION(BlueprintCallable)
	void DeactivateProjectile();
	UFUNCTION(BlueprintNativeEvent)
	void OnActivateEffect();
	void OnActivateEffect_Implementation();


	bool IsReady() const;


	void StickWithVisualOnly(const FHitResult& Hit);
	

protected:
	virtual void BeginPlay() override;
};