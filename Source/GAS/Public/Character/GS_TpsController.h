// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_TpsController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

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
	UInputAction* LAttackAction; // Left Click

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RAttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LCAttackAction; // Left + Ctrl Click

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RCAttackAction;

	void Move(const FInputActionValue& InputValue);
	void Look(const FInputActionValue& InputValue);
	void WalkToggle(const FInputActionValue& InputValue);
	void LAttackPressed(const FInputActionValue& InputValue);
	void LAttackRelease(const FInputActionValue& InputValue);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
