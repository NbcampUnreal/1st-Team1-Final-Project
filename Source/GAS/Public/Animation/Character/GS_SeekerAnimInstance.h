// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_CharacterAnimInstance.h"
#include "E_SeekerAnim.h"
#include "GS_SeekerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_SeekerAnimInstance : public UGS_CharacterAnimInstance
{
	GENERATED_BODY()
public:
	UGS_SeekerAnimInstance();
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// Update
	UFUNCTION(BlueprintNativeEvent, Category = "Update")
	void UpdateEssentialValue();
	UFUNCTION(BlueprintImplementableEvent, Category = "Update")
	void UpdateTrajectory();
	UFUNCTION(BlueprintNativeEvent, Category = "Update")
	void UpdateState();

	// Movement 
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"),Category = "Movement")
	bool IsMoving();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool ShouldTurnInPlace();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool ShouldSpinTransition();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool IsPivoting();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool IsStarting();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "OffsetRootBone")
	float GetOffsetRootTranslationHalfLife();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	FVector CalculateRelativeAccelerationAmount();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	float Get_LeanAmount();

	// Chan Combo Attack
	UFUNCTION()
	void AnimNotify_ComboInputOpen();
	UFUNCTION()
	void AnimNotify_ComboInputClose();
	UFUNCTION()
	void AnimNotify_EndMontage();
	
	UPROPERTY(BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingComboMontage = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FTransform CharacterTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FTransform RootTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector Acceleration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector Velocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector VelocityLastFrame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	float Speed2D;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector VelocityAcceleration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector LastNonZeroVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector FutureVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	ERotationMode RotationMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	EMovementState MovementState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	EGait Gait;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	ERotationMode LastRotationMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	EMovementState LastMovementState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	EGait LastGait;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
	TArray<FName> CurrentDatabasesTags;
};
