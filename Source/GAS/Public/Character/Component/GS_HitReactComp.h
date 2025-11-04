// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "E_HitReact.h"
#include "GS_HitReactComp.generated.h"

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

	/*UFUNCTION()
	void CheckAxeState(UAnimMontage* Montage, bool bInterrupted);*/ // SJE

	FOnMontageEnded HitReactEndDelegate;

	UFUNCTION()
	void OnEndDelegate(UAnimMontage* Montage, bool bInterrupted);

protected:
	virtual void BeginPlay() override;
};