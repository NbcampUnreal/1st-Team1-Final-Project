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

	void ChangeArrowType(EArrowType Type);
	virtual void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;

protected:
	virtual void BeginPlay() override;

private:
	TSet<AActor*> DamagedActors;
	
};
