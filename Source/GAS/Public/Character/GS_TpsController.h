// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_TpsController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FControlValue
{
	GENERATED_BODY()
public:
	FControlValue()
	{
		bCanLookUp = true;
		bCanLookRight = true;
		bCanMoveForward = true;
		bCanMoveRight = true;
	}

	bool CanMove() const { return bCanMoveForward || bCanMoveRight; }
	bool CanLook() const { return bCanLookUp || bCanLookRight; }
	
	UPROPERTY(EditAnywhere)
	bool bCanLookUp;

	UPROPERTY(EditAnywhere)
	bool bCanLookRight;

	UPROPERTY(EditAnywhere)
	bool bCanMoveForward;

	UPROPERTY(EditAnywhere)
	bool bCanMoveRight;
};

UCLASS()
class GAS_API AGS_TpsController : public APlayerController
{
	GENERATED_BODY()

public:
	AGS_TpsController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* WalkToggleAction;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PageUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PageDownAction;
	
	UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetupPlayerAudioListener();

	void Move(const FInputActionValue& InputValue);
	void Look(const FInputActionValue& InputValue);
	void WalkToggle(const FInputActionValue& InputValue);
	void PageUp(const FInputActionValue& InputValue);
	void PageDown(const FInputActionValue& InputValue);

	UFUNCTION(BlueprintImplementableEvent)
	void AddWidget();
	void InitControllerPerWorld();

	UFUNCTION()
	FControlValue GetControlValue() const;

	UFUNCTION()
	void SetMoveControlValue(bool CanMoveRight, bool CanMoveForward);

	UFUNCTION()
	void SetLookControlValue(bool CanLookRight, bool CanLookUp);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FControlValue ControlValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FRotator LastRotatorInMoving;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;
	virtual void BeginPlayingState() override;
};
