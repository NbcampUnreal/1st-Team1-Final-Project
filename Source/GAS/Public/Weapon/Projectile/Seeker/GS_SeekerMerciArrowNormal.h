// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Weapon/Projectile/Seeker/GS_ArrowType.h"
#include "GS_SeekerMerciArrowNormal.generated.h"

UCLASS()
class GAS_API AGS_SeekerMerciArrowNormal : public AGS_SeekerMerciArrow
{
	GENERATED_BODY()

public:
	AGS_SeekerMerciArrowNormal();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Arrow")
	EArrowType ArrowType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	float BaseDamage = 10.0f;

	void ChangeArrowType(EArrowType Type);

protected:
	virtual void BeginPlay() override;
	virtual void ProcessDamageLogic(ETargetType TargetType, const FHitResult& SweepResult, AActor* HitActor) override;
	virtual bool HandleTargetTypeGeneric(ETargetType TargetType, const FHitResult& SweepResult) override;
private:
	TSet<AActor*> DamagedActors;
};
