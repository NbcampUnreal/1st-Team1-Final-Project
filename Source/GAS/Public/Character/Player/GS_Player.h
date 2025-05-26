#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "Components/TimelineComponent.h"
#include "AkComponent.h"
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
	virtual void Tick(float DeltaSeconds) override;

	//component;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Components")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Components")
	TObjectPtr<UCameraComponent> CameraComp;

	// [시야방해]
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components", meta = (AllowPrivateAccess = "true"))
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* BlurMID;

	UPROPERTY(EditAnywhere, Category = "Vision")
	UMaterialInterface* PostProcessMat;

	UFUNCTION(Client, Reliable)
	void Client_StartVisionObscured();

	void StartVisionObscured();

	UFUNCTION(Client, Reliable)
	void Client_StopVisionObscured();

	void StopVisionObscured();

	FTimeline ObscureTimeline;

	UFUNCTION()
	void HandleTimelineProgress(float Value);

	UFUNCTION()
	void OnTimelineFinished();

	UPROPERTY()
	UCurveFloat* ObscureCurve; // 외부에서 세팅할 수 있음

	bool bIsObscuring = false;

	//variable
	UPROPERTY()
	float WalkSpeed;

	UPROPERTY()
	float RunSpeed;

	//Wants To Move
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	FCharacterWantsToMove WantsToMove;

	// 오디오 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAkComponent* AkComponent;

	// 오디오 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetupLocalAudioListener();
    
	UFUNCTION(BlueprintCallable, Category = "Audio")
	bool IsLocalPlayer() const;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FCharacterWantsToMove GetWantsToMove();
};
