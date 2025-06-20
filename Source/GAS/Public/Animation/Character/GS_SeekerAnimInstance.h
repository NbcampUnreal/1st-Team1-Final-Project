// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_CharacterAnimInstance.h"
#include "E_SeekerAnim.h"
#include "GS_SeekerAnimInstance.generated.h"

class UChooserTable;
class UGS_ChooserInputObj;
struct UPoseSearchDatabase;

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

	UFUNCTION()
	bool GetMustTurnInPlace();

	UFUNCTION()
	void SetMustTurnInPlace(bool MustTurn);

	// Offset Root Bone
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "OffsetRootBone")
	float GetOffsetRootTranslationHalfLife();

	// Chooser
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	TObjectPtr<UGS_ChooserInputObj> ChooserInputObj;

	// Lean
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	FVector CalculateRelativeAccelerationAmount();
	
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	float Get_LeanAmount();

	// Steering
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Steering")
	bool EnableSteering();

	// Slot Change Control Value
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingUpperBodyMontage = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingFullBodyMontage = false;

	UFUNCTION(BlueprintPure, meta=(NotBlueprintThreadSafe="ture"), Category = "AimOffset")
	FVector2D Get_AOValue();

	UFUNCTION(BlueprintPure, meta=(NotBlueprintThreadSafe="ture"), Category = "AimOffset")
	bool Enable_AO();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector Acceleration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector VelocityLastFrame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector VelocityAcceleration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialValue")
	FVector LastNonZeroVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	ERotationMode LastRotationMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateValue")
	EGait LastGait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OffsetRootBone")
	bool bUseOffsetRootBone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	float PreviousDesiredController;
};
