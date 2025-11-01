// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"

class AGS_Player;

#include "GS_AresMovingSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_AresMovingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
	
public:
	UGS_AresMovingSkill();

	// BP_Ares에서 설정된 값을 받기 위한 함수
	void SetCameraSettings(float InZoomOutDistance, UCurveFloat* InCameraZoomCurve,
		bool bInEnableMotionBlur = false,
		float InMotionBlurPeakAmount = 0.0f,
		UCurveFloat* InMotionBlurCurve = nullptr,
		float InMotionBlurExponent = 1.0f);

	virtual void BeginDestroy() override;

	virtual void InitializeDelegate() override;
	virtual void ActiveSkill() override;
	virtual void OnSkillCanceledByDebuff() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void OnSkillCommand() override;
	virtual void InterruptSkill() override;

	// 카메라 복원 (GS_Ares에서 호출됨)
	void RestoreCameraZoom(bool bForceRestore = false);

protected:
	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	void UpdateCharging();
	void StartDash();
	void UpdateDash();
	virtual void DeactiveSkill() override;

	// OnSkillActivated 델리게이트에 바인딩할 함수
	UFUNCTION()
	void HandleSkillActivated(ESkillSlot ActivatedSkillSlot);

	// 카메라 연출 관련
	void StartCameraZoomOut();
	void UpdateCameraZoom();

	// 타이머 관리
	void SafeClearTimer(FTimerHandle& TimerHandle);
	bool IsWorldContextValid() const;

	// 카메라 줌 헬퍼 함수
	float GetCameraZoomDuration() const;

	// 모션 블러 관련
	void CacheCameraMotionBlurDefaults(class AGS_Player* Player);
	void UpdateCameraMotionBlur(float NormalizedAlpha, float ElapsedTime);
	void ResetCameraMotionBlur();

	// 카메라 애니메이션 상태
	enum class EZoomState : uint8
	{
		Idle,
		ZoomingOut,
		ZoomedOut,
		ZoomingIn
	};

	EZoomState CurrentZoomState = EZoomState::Idle;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCurveFloat* CameraZoomCurve;

	// 카메라 줌아웃 거리
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	float ZoomOutDistance = 300.0f;

	// 모션 블러 설정
	UPROPERTY(VisibleAnywhere, Category = "Camera|MotionBlur")
	bool bEnableMotionBlur = false;

	UPROPERTY(VisibleAnywhere, Category = "Camera|MotionBlur")
	float MotionBlurPeakAmount = 0.5f;

	UPROPERTY(VisibleAnywhere, Category = "Camera|MotionBlur")
	UCurveFloat* MotionBlurCurve = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Camera|MotionBlur")
	float MotionBlurExponent = 1.0f;

	// 카메라 줌 애니메이션 관련
	float OriginalArmLength = 0.0f;
	float CameraZoomElapsed = 0.0f;
	float CameraZoomDuration = 0.3f;

	bool bPendingZoomIn = false;
	bool bMotionBlurActive = false;
	bool bMotionBlurDefaultsCached = false;
	bool bOriginalOverrideMotionBlurAmount = false;
	float OriginalMotionBlurAmount = 0.0f;

	FTimerHandle ChargingTimerHandle;
	FTimerHandle DashTimerHandle;
	FTimerHandle CameraUpdateTimerHandle;

	float ChargingTime = 0.0f;
	float ChargingStartTime = 0.0f;
	float MaxChargingTime = 2.0f;

	float MinDashDistance = 200.0f;
	float MaxDashDistance = 2000.0f;
	float DashDuration = 0.3f;
	float DashInterpAlpha = 0.0f;

	FVector DashDirection; // 돌진 방향
	FVector DashStartLocation;
	FVector DashEndLocation;

	TSet<AActor*> DamagedActors;

	ECollisionResponse OriginalCapsuleResponseToPawn;
	ECollisionResponse OriginalMeshResponseToPawn;
};
