#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "GS_Player.generated.h"

class USpringArmComponent;
class UCameraComponent;

USTRUCT(BlueprintType)
struct FCharacterWantsToMove
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move")
	bool WantsToSprint = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move")
	bool WantsToWalk = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move")
	bool WantsToAim = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move")
	bool WantsToStrafe = false;
};

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

	//Wants To Move
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	FCharacterWantsToMove WantsToMove;
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FCharacterWantsToMove GetWantsToMove();
};
