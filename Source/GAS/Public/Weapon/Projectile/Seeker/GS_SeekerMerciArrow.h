// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "Engine/Engine.h" 
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "AkAudioEvent.h"
#include "NiagaraSystem.h"
#include "Weapon/Projectile/GS_TargetType.h"
#include "GS_SeekerMerciArrow.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArrowHitEnemy, AActor*, HitActor);

class AGS_ArrowVisualActor;
class UGS_ArrowFXComponent;

UCLASS()
class GAS_API AGS_SeekerMerciArrow : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_SeekerMerciArrow();
	UPROPERTY()
	TSet<AActor*> HitActors;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InitHomingTarget(AActor* Target);
	//AGS_Player* OwnerPlayer;

	//Crosshair
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnArrowHitEnemy OnArrowHitEnemy;

	UFUNCTION()
	void OnTargetDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnTargetDied();

protected:
	virtual void BeginPlay() override;
	void StickWithVisualOnly(const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGS_ArrowVisualActor> VisualArrowClass;

	// 화살 FX 컴포넌트 (Trail VFX + Hit VFX + Sound 모두 관리)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FX")
	class UGS_ArrowFXComponent* ArrowFXComponent;

	// 유도 화살
	UPROPERTY()
	AActor* HomingTarget = nullptr;

	

	UFUNCTION()
	virtual void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	virtual ETargetType DetermineTargetType(AActor* OtherActor) const;
	virtual void HandleTargetTypeGeneric(ETargetType TargetType, const FHitResult& SweepResult);
};
