// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_RTSController.generated.h"

struct FInputActionValue;
class AGS_Monster;
class UInputMappingContext;
class UInputAction;

UCLASS()
class GAS_API AGS_RTSController : public APlayerController
{
	GENERATED_BODY()

public:
	AGS_RTSController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* CameraMoveAction;

	UFUNCTION()
	void CameraMove(const FInputActionValue& value);
	
	UFUNCTION()
	void CameraMoveEnd();

	void AddUnitToSelection(AGS_Monster* Unit);
	void RemoveUnitFromSelection(AGS_Monster* Unit);
	void ClearUnitSelection();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

private:
	FVector2D KeyboardDir = FVector2D::ZeroVector;
	FVector2D MouseEdgeDir = FVector2D::ZeroVector;

	UPROPERTY()
	class AGS_RTSCamera* CameraActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraSpeed = 2000.f;
	
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EdgeScreenRatio = 0.05f;

	UPROPERTY()
	TArray<AGS_Monster*> UnitSelection;

	FVector2D GetKeyboardDirection() const;
	FVector2D GetMouseEdgeDirection() const;
	FVector2D GetFinalDirection() const;
	void MoveCamera(const FVector2D& Direction, float DeltaTime);
	void InitCameraActor();
};
