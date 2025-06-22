// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Character/GS_Character.h"
#include "UI/Character/GS_CrossHairImage.h"
#include "GS_TpsController.generated.h"

class UGS_GameInstance;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FControlValue
{
	GENERATED_BODY()
public:
	FControlValue()
	{
		bCanLookUp = true;
		bCanLookRight = true;
		bCanMoveForward = true;
		bCanMoveRight = true;
	}

	bool CanMove() const { return bCanMoveForward || bCanMoveRight; }
	bool CanLook() const { return bCanLookUp || bCanLookRight; }
	
	UPROPERTY(EditAnywhere)
	bool bCanLookUp;

	UPROPERTY(EditAnywhere)
	bool bCanLookRight;

	UPROPERTY(EditAnywhere)
	bool bCanMoveForward;

	UPROPERTY(EditAnywhere)
	bool bCanMoveRight;
};

UCLASS()
class GAS_API AGS_TpsController : public APlayerController
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
	UInputAction* RClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PageUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PageDownAction;
	
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
	void SetLookControlValue(bool CanLookRight, bool CanLookUp);
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Control")
	FControlValue ControlValues;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FRotator LastRotatorInMoving;

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
	

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;
	virtual void BeginPlayingState() override;

	//게임 인스턴스 참조
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	UGS_GameInstance* GameInstance;

	//메르시 크로스헤어 위젯
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UGS_CrossHairImage* CrosshairWidget;

private:
	// Auto Moving (KCY)
	FTimerHandle AutoMoveTickHandle;
	UPROPERTY(Replicated)
	bool bIsAutoMoving = false;

	void AutoMoveTick();
	
	UFUNCTION(Client, Reliable)
	void Client_StartAutoMoveForward();

	UFUNCTION(Client, Reliable)
	void Client_StopAutoMoveForward();

};
