#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GS_DEPawn.generated.h"

struct FInputActionValue;
class UFloatingPawnMovement;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class GAS_API AGS_DEPawn : public APawn
{
	GENERATED_BODY()

public:
	AGS_DEPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USceneComponent> SceneComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> MovementComp;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Zoom(const FInputActionValue& Value);
	UFUNCTION()
	void PropRotation(const FInputActionValue& Value);
	UFUNCTION()
	void ClickLMB(const FInputActionValue& Value);
	UFUNCTION()
	void ReleasedLMB(const FInputActionValue& Value);
	UFUNCTION()
	void ClickRMB(const FInputActionValue& Value);
	UFUNCTION()
	void ReleasedRMB(const FInputActionValue& Value);
};
