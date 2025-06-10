// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_HitReactComp.generated.h"


UENUM(BlueprintType)
enum EHitReactType : uint8
{
	Interrupt,
	Additive,
	DamageOnly,
	TypeNum,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))

class GAS_API UGS_HitReactComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_HitReactComp();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UAnimMontage*> AM_HitReacts;

	UFUNCTION()
	void PlayHitReact(EHitReactType ReactType, FVector HitDirection);

	UFUNCTION()
	void StopHitReact(UAnimMontage* TargetMontage);
	
	UFUNCTION()
	FName CalculateHitDirection(FVector HitDirection);

protected:
	virtual void BeginPlay() override;
};