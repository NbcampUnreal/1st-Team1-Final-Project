// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSCommand.h"
#include "GS_RTSController.generated.h"

struct FInputActionInstance;
struct FInputActionValue;
class AGS_Monster;
class AGS_Character;
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, const TArray<AGS_Monster*>&, NewSelection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRTSCommandChanged, ERTSCommand, NewCommand);

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
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* StopAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* SkillAction;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TArray<UInputAction*> CameraKeyActions;

	// 선택 변경 델리게이트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Selection")
	FOnSelectionChanged OnSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category="Command")
	FOnRTSCommandChanged OnRTSCommandChanged;

	// 현재 선택된 유닛들
	const TArray<AGS_Monster*>& GetUnitSelection() const { return UnitSelection; }

	// 현재 명령 상태 반환
	ERTSCommand GetCurrentCommand() const { return CurrentCommand; }

	// 카메라 이동 입력 처리
	void CameraMove(const FInputActionValue& InputValue);
	void CameraMoveEnd();

	// 명령 모드 전환
	void OnCommandMove(const FInputActionValue& Value);
	void OnCommandAttack(const FInputActionValue& Value);
	void OnCommandStop(const FInputActionValue& Value);
	void OnCommandSkill(const FInputActionValue& Value);

	// 실제 구현 + HUD 버튼 클릭시
	UFUNCTION(BlueprintCallable, Category="RTS")
	void MoveSelectedUnits();
	
	UFUNCTION(BlueprintCallable, Category="RTS")
	void AttackSelectedUnits();
	
	UFUNCTION(BlueprintCallable, Category="RTS")
	void StopSelectedUnits();

	UFUNCTION(BlueprintCallable, Category="RTS")
	void SkillSelectedUnits();

	// 마우스 클릭 처리
	void OnLeftMousePressed();
	void OnLeftMouseReleased();
	void OnRightMousePressed(const FInputActionValue& InputValue);

	// 유닛 선택
	void AddUnitToSelection(AGS_Monster* Unit);
	void AddMultipleUnitsToSelection(const TArray<AGS_Monster*>& Units); // 다중 선택
	void RemoveUnitFromSelection(AGS_Monster* Unit);
	void ClearUnitSelection();

	// 부대 지정, 호출 
	void OnCtrlPressed(const FInputActionInstance& InputInstance);
	void OnCtrlReleased(const FInputActionInstance& InputInstance);
	void OnShiftPressed(const FInputActionInstance& InputInstance);
	void OnShiftReleased(const FInputActionInstance& InputInstance);
	void OnGroupKey(const FInputActionInstance& InputInstance, int32 GroupIdx);

	// 미니맵
	void OnCameraKey(const FInputActionInstance& InputInstance, int32 CameraIndex);
	
	UFUNCTION(BlueprintCallable)
	void MoveAIViaMinimap(const FVector& WorldLocation);
	
	UFUNCTION(BlueprintCallable)
	void MoveCameraViaMinimap(const FVector& WorldLocation);

	// Server
	UFUNCTION(Server, Reliable)
	void Server_RTSMove(const TArray<AGS_Monster*>& UnitsToCommand, const FVector& Dest);

	UFUNCTION(Server, Reliable)
	void Server_RTSAttackMove(const TArray<AGS_Monster*>& UnitsToCommand, const FVector& Dest);

	UFUNCTION(Server, Reliable)
	void Server_RTSAttack(const TArray<AGS_Monster*>& UnitsToCommand, AGS_Character* TargetActor);

	UFUNCTION(Server, Reliable)
	void Server_RTSStop(const TArray<AGS_Monster*>& UnitsToCommand);

	UFUNCTION(Server, Reliable)
	void Server_RTSSkill(const TArray<AGS_Monster*>& UnitsToCommand, const FVector& TargetLoc);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 입력 상태
	ERTSCommand CurrentCommand;

	// 카메라 
	FVector2D KeyboardDir;
	FVector2D MouseEdgeDir;

	UPROPERTY()
	class AGS_RTSCamera* CameraActor;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EdgeScreenRatio;

	UPROPERTY(Replicated)
	TArray<AGS_Monster*> UnitSelection; // 현재 선택된 유닛

	UPROPERTY(Replicated)
	TArray<FUnitGroup> UnitGroups; // 지정된 부대

	bool bCtrlDown;
	bool bShiftDown;

	// 명령이 방금 실행되었는지 추적하는 플래그
	bool bCommandJustExecuted = false;

	UPROPERTY()
	TMap<int32, FVector> SavedCameraPositions; // 카메라 저장 위치

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> RTSWidgetClass;

	// 유효성 검사 타이머 (static 제거하고 멤버 변수로 변경)
	FTimerHandle CleanupTimerHandle;
	
	UPROPERTY(EditAnywhere, Category="RTS", meta=(ClampMin="1.0", ClampMax="10.0"))
	float CleanupInterval = 5.0f; // 5초마다 검사 (1초보다 느리게)

	FVector2D GetKeyboardDirection() const;
	FVector2D GetMouseEdgeDirection() const;
	FVector2D GetFinalDirection() const;
	void MoveCamera(const FVector2D& Direction, float DeltaTime);
	void InitCameraActor();
	
	void SelectOnCtrlClick();
	void ToggleOnShiftClick();
	
	// 명령 가능한 유닛들
	void GatherCommandableUnits(TArray<AGS_Monster*>& Out) const;
	bool IsSelectable(AGS_Monster* Monster) const;

	UFUNCTION()
	void OnSelectedUnitDead(AGS_Monster* Monster);
	
	// 유효하지 않은 유닛들 정리
	void CleanupInvalidUnits();
};

