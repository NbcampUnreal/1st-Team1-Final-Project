// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_RTSController.generated.h"

struct FInputActionInstance;
struct FInputActionValue;
class AGS_Monster;
class UInputMappingContext;
class UInputAction;

// 지정된 부대 
USTRUCT(BlueprintType)
struct FUnitGroup
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<AGS_Monster*> Units;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* LeftClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* RightClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* CtrlAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* ShiftAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TArray<UInputAction*> GroupKeyActions;

	// 카메라 이동 입력 처리
	void CameraMove(const FInputActionValue& InputValue);
	void CameraMoveEnd();

	// 마우스 클릭 처리
	void OnLeftMousePressed();
	void OnLeftMouseReleased();
	void OnRightMousePressed(const FInputActionValue& InputValue);

	// 유닛 선택
	void AddUnitToSelection(AGS_Monster* Unit);
	void RemoveUnitFromSelection(AGS_Monster* Unit);
	void ClearUnitSelection();

	// 부대 지정, 호출 
	void OnCtrlPressed(const FInputActionInstance& InputInstance);
	void OnCtrlReleased(const FInputActionInstance& InputInstance);
	void OnShiftPressed(const FInputActionInstance& InputInstance);
	void OnShiftReleased(const FInputActionInstance& InputInstance);
	void OnGroupKey(const FInputActionInstance& InputInstance, int32 GroupIdx);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

private:
	FVector2D KeyboardDir;
	FVector2D MouseEdgeDir;

	UPROPERTY()
	class AGS_RTSCamera* CameraActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EdgeScreenRatio;

	UPROPERTY()
	TArray<AGS_Monster*> UnitSelection; // 현재 선택된 유닛

	UPROPERTY()
	TArray<FUnitGroup> UnitGroups; // 지정된 부대

	bool bCtrlDown = false;
	bool bShiftDown = false;

	FVector2D GetKeyboardDirection() const;
	FVector2D GetMouseEdgeDirection() const;
	FVector2D GetFinalDirection() const;
	void MoveCamera(const FVector2D& Direction, float DeltaTime);
	void InitCameraActor();

	void DoShiftClickToggle();
};

