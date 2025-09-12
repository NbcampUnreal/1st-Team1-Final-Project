// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Animation/Character/E_SeekerAnim.h"
#include "Character/E_Character.h"
#include "GS_ChooserInputObj.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class GAS_API UGS_ChooserInputObj : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool ShouldTurnInPlace();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"),Category = "Movement")
	bool IsMoving();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool IsStarting();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool IsPivoting();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool ShouldSpinTransition();

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector Velocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector FutureVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FTransform CharacterTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FTransform RootTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	float Speed2D;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	EMovementState MovementState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	EMovementState LastMovementState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, CAtegory = "StateValue")
	EGait Gait;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	ERotationMode RotationMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control")
	bool bMustTurnInPlace = false;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
	TArray<FName> CurrentDatabasesTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterClass")
	ECharacterType CharacterType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StateValue")
	bool IsBlock = false;
};
