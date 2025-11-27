// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GS_BasePlayerController.h"
#include "Character/GS_Character.h"
#include "UI/Character/GS_CrossHairImage.h"
#include "Character/Skill/ESkill.h"
#include "GS_TpsController.generated.h"

class UGS_GameInstance;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AGS_Seeker;
class UGS_ReviveIndicatorWidget;

UCLASS()
class GAS_API AGS_TpsController : public AGS_BasePlayerController
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
	UInputAction* PlaceMarkerAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PageUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PageDownAction;

	// ==========================================
	// 빈사 플레이어 구조 시스템
	// ==========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Revive")
	UInputAction* ReviveAction;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> PlayerWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TMap<ECharacterType, TSubclassOf<UUserWidget>> PlayerWidgetClasses;
	
	UFUNCTION(BlueprintCallable)
	UUserWidget* GetPlayerWidget();
	
	UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetupPlayerAudioListener();

	void Move(const FInputActionValue& InputValue);
	void Look(const FInputActionValue& InputValue);
	void WalkToggle(const FInputActionValue& InputValue);
	void PlaceMarker(const FInputActionValue& InputValue);
	void PageUp(const FInputActionValue& InputValue);
	void PageDown(const FInputActionValue& InputValue);

	void InitControllerPerWorld();
	
	//[Spectate Other Player]
	UFUNCTION(Server, Unreliable)
	void ServerRPCSpectatePlayer();
	
	UFUNCTION()
	FControlValue GetControlValue() const;

	UFUNCTION()
	void SetMoveControlValue(bool CanMoveRight, bool CanMoveForward);

	UFUNCTION()
	FControlValue GetMoveControlValue();

	UFUNCTION()
	void SetLookControlValue(bool CanLookRight, bool CanLookUp);
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Control")
	FControlValue ControlValues;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FRotator LastRotatorInMoving;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Control")
	FVector2D MoveInputValue;

	UFUNCTION(Server, Reliable)
	void Server_CacheMoveInputValue(FVector2D InputValue);

	UFUNCTION(BlueprintCallable)
	void TestFunction();
	
	//마우스 민감도 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Settings")
	float GetCurrentMouseSensitivity() const;

	//메르시 크로스헤어 위젯
	UFUNCTION(BlueprintCallable, Category = "UI")
	UGS_CrossHairImage* GetCrosshairWidget() const { return CrosshairWidget; }

	// Auto Moving (KCY)
	void StartAutoMoveForward();
	void StopAutoMoveForward();

	// Pawn 유효성 검사를 위한 타이머 및 함수 추가
	void TryCreatingPlayerWidget();
	FTimerHandle WaitForPawnTimerHandle;

	// Debug
	UFUNCTION(Client, Unreliable)
	void Client_DrawAimAssistDebug(const FVector& Start, const FVector& End, const FVector& TargetLocation, float Duration); // SJE
	
	void SetIsAutoMoving(bool InIsAutoMoving);

	// ==========================================
	// 빈사 플레이어 구조 시스템
	// ==========================================
	
	/** E키 누름 - 구조 시작 시도 */
	void TryStartRevive(const FInputActionValue& InputValue);
	
	/** E키 떼기 - 구조 취소 */
	void StopRevive(const FInputActionValue& InputValue);
	
	/** 현재 구조 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Revive")
	bool IsReviving() const { return bIsReviving; }

	/** E키를 누르고 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Revive")
	bool IsHoldingReviveKey() const { return bIsHoldingReviveKey; }

	/** 현재 구조 대상 확인 */
	UFUNCTION(BlueprintPure, Category = "Revive")
	AGS_Seeker* GetReviveTarget() const { return ReviveTarget.Get(); }

	/** 구조 시작 서버 RPC */
	UFUNCTION(Server, Reliable, Category = "Revive")
	void Server_RequestRevive(AGS_Seeker* Target);
	
	/** 구조 취소 서버 RPC */
	UFUNCTION(Server, Reliable, Category = "Revive")
	void Server_CancelRevive();

	/** E키 홀드 상태 서버로 전달 */
	UFUNCTION(Server, Unreliable, Category = "Revive")
	void Server_SetHoldingReviveKey(bool bIsHolding);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;
	virtual void BeginPlayingState() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//게임 인스턴스 참조
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	UGS_GameInstance* GameInstance;

	//메르시 크로스헤어 위젯
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UGS_CrossHairImage* CrosshairWidget;

	void SnapCameraToCharacterYaw();

	/** 근처 빈사 시커 감지 및 위젯 업데이트 (Tick에서 호출) */
	void UpdateReviveIndicatorVisibility();

private:
	// Auto Moving (KCY)
	FTimerHandle AutoMoveTickHandle;
	
	UPROPERTY(Replicated)
	bool bIsAutoMoving = false;

	// ==========================================
	// 빈사 플레이어 구조 시스템 변수들
	// ==========================================
	
	/** 현재 구조 중인지 여부 */
	bool bIsReviving = false;

	/** E키를 누르고 있는지 여부 (서버로 복제됨) */
	UPROPERTY(Replicated)
	bool bIsHoldingReviveKey = false;

	/** 현재 구조 대상 */
	TWeakObjectPtr<AGS_Seeker> ReviveTarget;

	/** 근처에 빈사 시커가 있는지 여부 (위젯 표시 제어) */
	bool bNearbyDyingSeekerDetected = false;

	/** 마지막으로 감지된 빈사 시커 */
	TWeakObjectPtr<AGS_Seeker> LastDetectedDyingSeeker;

	/** 근처 빈사 상태 시커 찾기 */
	AGS_Seeker* FindNearbyDyingSeeker() const;
	
	/** 구조 가능 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Revive", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float ReviveDistance = 200.0f;

	/** 구조 표시 위젯 클래스 (BP에서 할당) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Revive", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGS_ReviveIndicatorWidget> ReviveIndicatorWidgetClass;

	/** 현재 생성된 구조 표시 위젯 */
	UPROPERTY()
	UGS_ReviveIndicatorWidget* ReviveIndicatorWidget;

	void AutoMoveTick();
	void ApplyChargeCameraSettings(bool bCharging);
	void SaveOriginalCameraSettings();
	void RestoreOriginalCameraSettings();

	UFUNCTION(Client, Reliable)
	void Client_StartAutoMoveForward();

	UFUNCTION(Client, Reliable)
	void Client_StopAutoMoveForward();

	// 돌진 시 원래 카메라 설정 저장용 (KCY)
	bool bOriginalUseControllerRotationYaw = true;
	bool bOriginalOrientRotationToMovement = false;
	bool bOriginalUsePawnControlRotation = true;
	bool bOriginalEnableCameraLag = true;
	bool bOriginalEnableCameraRotationLag = true;
	bool bOriginalInheritYaw = true;
};
