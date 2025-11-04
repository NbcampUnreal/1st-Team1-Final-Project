#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "Components/TimelineComponent.h"
#include "AkComponent.h"
#include "GS_Player.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UGS_SteamNameWidgetComp;
class FAkAudioDevice;

USTRUCT(BlueprintType)
struct FSkillInputControl
{
	GENERATED_BODY();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
	bool CanInputLC = true; // Left Click
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
	bool CanInputRC = true; // Right Click
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
	bool CanInputRoll = true; // SpaceBar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control")
	bool CanInputCtrl = true; // Ctrl
};

UCLASS()
class GAS_API AGS_Player : public AGS_Character
{
	GENERATED_BODY()

public:
	AGS_Player();

	// component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Components")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Components")
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UGS_SteamNameWidgetComp> SteamNameWidgetComp;
	
	// 시야방해
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components", meta = (AllowPrivateAccess = "true"))
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* BlurMID;

	UPROPERTY(EditAnywhere, Category = "Vision")
	UMaterialInterface* PostProcessMat;

	UPROPERTY()
	UCurveFloat* ObscureCurve; // 외부에서 세팅할 수 있음

	// variable
	UPROPERTY()
	float WalkSpeed;

	UPROPERTY()
	float RunSpeed;

	// 오디오 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAkComponent* AkComponent;

	// 머리 위치 오디오 리스너 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAkComponent* HeadAudioListenerComponent;

	// 머리 위치로 사용할 소켓/본 후보 목록 (상위에서부터 우선순위)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TArray<FName> HeadListenerCandidates;

	// 후보를 찾지 못했을 때 적용할 Z 오프셋(머리 높이 추정치)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	float HeadListenerZOffset = 180.0f;

	UFUNCTION(Client, Reliable)
	void Client_StartVisionObscured();

	void StartVisionObscured();

	UFUNCTION(Client, Reliable)
	void Client_StopVisionObscured();

	void StopVisionObscured();
	
	UFUNCTION()
	void HandleTimelineProgress(float Value);

	UFUNCTION()
	void OnTimelineFinished();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetUseControllerRotationYaw(bool UseControlRotationYaw);

	// 사운드 재생 함수들
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySound(UAkAudioEvent* SoundEvent);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySoundWithCallback(UAkAudioEvent* SoundEvent, const FOnAkPostEventCallback& Callback);

	// 오디오 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetupLocalAudioListener();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetupHeadAudioListener();
    
	UFUNCTION(BlueprintCallable, Category = "Audio")
	bool IsLocalPlayer() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySkillMontage(UAnimMontage* Montage, FName Section = NAME_None, int32 PlayRate = 1.0f);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopSkillMontage(UAnimMontage* Montage);
	
	virtual void OnDeath() override;
	
	// Skll Input Control
	void SetSkillInputControl(bool CanLeftClick, bool CanRightClick, bool CanRollClick, bool CanCtrlClick = true);
	FSkillInputControl GetSkillInputControl();
	
	FORCEINLINE UGS_SkillComp* GetSkillComp() const { return SkillComp; }
	virtual void SetCanUseSkill(bool bCanUse) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_SkillComp> SkillComp;
private:
	// Input Control Flag
	UPROPERTY(Replicated)
	FSkillInputControl SkillInputControl;
	
	FTimeline ObscureTimeline;

	bool bIsObscuring;

	void UpdateSteamNameWidgetRotation();

	// 오디오 디바이스 캐싱
	FAkAudioDevice* CachedAudioDevice = nullptr;

};
