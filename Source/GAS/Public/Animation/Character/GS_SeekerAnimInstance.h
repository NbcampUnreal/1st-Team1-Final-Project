// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_CharacterAnimInstance.h"
#include "E_SeekerAnim.h"
#include "GS_SeekerAnimInstance.generated.h"

class UChooserTable;
class UGS_ChooserInputObj;
struct UPoseSearchDatabase;

UENUM(BlueprintType)
enum class ESeekerMontageSlot : uint8
{
	None UMETA(DisplayName = "None"),
	FullBody UMETA(DisplayName = "Full Body"),
	UpperBody UMETA(DisplayName = "Upper Body"),
	End UMETA(DisplayName = "End"),
};

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
	
	// Lean
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	FVector CalculateRelativeAccelerationAmount();
	
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Lean")
	float Get_LeanAmount();

	// Steering
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe="true"), Category = "Steering")
	bool EnableSteering();

	UFUNCTION(BlueprintPure, meta=(NotBlueprintThreadSafe="ture"), Category = "AimOffset")
	FVector2D Get_AOValue();

	UFUNCTION(BlueprintPure, meta=(NotBlueprintThreadSafe="ture"), Category = "AimOffset")
	bool Enable_AO();

	UFUNCTION(BlueprintCallable, Category = "Montage")
	void SetCurMontageSlot(ESeekerMontageSlot InputMontageSlot);

	// Slot Change Control Value
	/*UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingUpperBodyMontage = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingFullBodyMontage = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingLeftArmMontage = false;*/

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	uint8 CurMontageSlot = 0;
	
	// Chooser
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	TObjectPtr<UGS_ChooserInputObj> ChooserInputObj;

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

	UFUNCTION(BlueprintPure, Category = "Montage")
	bool IsMontageSlotActive(ESeekerMontageSlot InputMontageSlot);
};
