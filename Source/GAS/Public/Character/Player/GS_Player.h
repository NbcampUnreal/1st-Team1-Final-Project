#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "Components/TimelineComponent.h"
#include "AkComponent.h"
#include "GS_Player.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UGS_SteamNameWidgetComp;

/*USTRUCT(BlueprintType)
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
};*/

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

/*UENUM(BlueprintType)
enum class EInputFlag : uint8
{
	None = 0,
	CanInputLC = (1 << 0),
	CanInputRC = (1 << 1),
	CanInputRoll = (1 << 2),
	CanInputLeftClick = (1 << 3)
};

ENUM_CLASS_FLAGS(EInputFlag)*/

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

	// Wants To Move
	/*UPROPERTY(BlueprintReadWrite, Category = "Movement")
	FCharacterWantsToMove WantsToMove;*/

	// 오디오 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAkComponent* AkComponent;

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
	bool IsLocalPlayer() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySkillMontage(UAnimMontage* Montage, FName Section = NAME_None);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopSkillMontage(UAnimMontage* Montage);
	
	virtual void OnDeath() override;
	
	// Skll Input Control
	void SetSkillInputControl(bool CanLeftClick, bool CanRightClick, bool CanRollClick, bool CanCtrlClick = true);
	FSkillInputControl GetSkillInputControl();
	
	FORCEINLINE UGS_SkillComp* GetSkillComp() const { return SkillComp; }
	virtual void SetCanUseSkill(bool bCanUse) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
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

};
