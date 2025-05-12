#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "GS_Player.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class GAS_API AGS_Player : public AGS_Character
{
	GENERATED_BODY()

public:
	AGS_Player();

	virtual void BeginPlay() override;

	//component;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Components")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Components")
	TObjectPtr<UCameraComponent> CameraComp;

	//variable
	UPROPERTY()
	float WalkSpeed;

	UPROPERTY()
	float RunSpeed;


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
