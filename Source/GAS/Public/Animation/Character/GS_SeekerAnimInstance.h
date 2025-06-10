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

	// Anim Update
	UFUNCTION(BlueprintNativeEvent, Category = "Update")
	void UpdateEssentialValue();
	UFUNCTION(BlueprintImplementableEvent, Category = "Update")
	void UpdateTrajectory();
	UFUNCTION(BlueprintNativeEvent, Category = "Update")
	void UpdateState();

	// Movement control Flags 
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"),Category = "Movement")
	bool IsMoving();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool ShouldTurnInPlace();

	UFUNCTION()
	bool GetMustTurnInPlace();

	UFUNCTION()
	void SetMustTurnInPlace(bool MustTurn);

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool ShouldSpinTransition();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool IsPivoting();

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Movement")
	bool IsStarting();

	// Offset Root Bone
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "OffsetRootBone")
	float GetOffsetRootTranslationHalfLife();

	// Lean
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	FVector CalculateRelativeAccelerationAmount();
	
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	float Get_LeanAmount();

	// Slot Change Control Value
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingUpperBodyMontage = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingFullBodyMontage = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control")
	bool bMustTurnInPlace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OffsetRootBone")
	bool bUseOffsetRootBone = false;
};
