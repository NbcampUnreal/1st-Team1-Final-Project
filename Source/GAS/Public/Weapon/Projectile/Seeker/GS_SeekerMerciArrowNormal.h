// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "GS_SeekerMerciArrowNormal.generated.h"

UENUM(BlueprintType)
enum class EArrowType : uint8
{
	Normal	UMETA(DisplayName="Normal"),
	Axe		UMETA(DisplayName = "Axe"),
	Child	UMETA(DisplayName = "Child")
};

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

	UPROPERTY()
	TSet<AActor*> HitActors;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

protected:
	virtual void BeginPlay() override;
	
};
