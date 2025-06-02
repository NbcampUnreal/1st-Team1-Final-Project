// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_SeekerMerciArrow.generated.h"

class AGS_ArrowVisualActor;

UENUM(BlueprintType)
enum class ETargetType : uint8
{
	Guardian		UMETA(DisplayName = "Guardian"),
	DungeonMonster	UMETA(DisplayName = "DungeonMonster"),
	Seeker			UMETA(DisplayName = "Seeker"),
	Structure		UMETA(DisplayName = "Structure"),
	Skill			UMETA(DisplayName = "Skill")
};

UCLASS()
class GAS_API AGS_SeekerMerciArrow : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_SeekerMerciArrow();
	UPROPERTY()
	TSet<AActor*> HitActors;

protected:
	virtual void BeginPlay() override;
	void StickWithVisualOnly(const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGS_ArrowVisualActor> VisualArrowClass;

	UFUNCTION()
	virtual void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	virtual ETargetType DetermineTargetType(AActor* OtherActor) const;
	virtual void HandleTargetTypeGeneric(ETargetType TargetType, const FHitResult& SweepResult);
};
