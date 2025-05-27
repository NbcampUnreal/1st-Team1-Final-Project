// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "GS_Seeker.generated.h"

class UGS_SkillInputHandlerComp;
class UPostProcessComponent;
class UMaterialInterface;
class UGS_StatComp;

USTRUCT(BlueprintType) // Current Action
struct FSeekerState
{
	GENERATED_BODY()

	FSeekerState()
	{
		IsAim = false;
		IsDraw = false;
		IsEquip = false;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsAim;
	bool IsDraw;
	bool IsEquip;
};

UCLASS()
class GAS_API AGS_Seeker : public AGS_Player
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Seeker();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SetAimState(bool IsAim);

	UFUNCTION(BlueprintPure, Category = "State")
	bool GetAimState();

	UFUNCTION(BlueprintCallable)
	void SetDrawState(bool IsDraw);

	UFUNCTION(BlueprintPure, Category = "State")
	bool GetDrawState();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	UChildActorComponent* Weapon;
	
	// ================
	// LowHP 스크린 효과
	// ================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
	UPostProcessComponent* LowHealthPostProcessComp;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	UMaterialInterface* LowHealthEffectMaterial;
	
	UFUNCTION()
	void HandleLowHealthEffect(UGS_StatComp* InStatComp);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	UGS_SkillInputHandlerComp* SkillInputHandlerComponent;

private :
	UPROPERTY(VisibleAnywhere, Category="State")
	FSeekerState SeekerState;

	// ================
	// LowHP 스크린 효과
	// ================
	UPROPERTY(EditDefaultsOnly, Category="Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LowHealthThresholdRatio = 0.3f;

	bool bIsLowHealthEffectActive = false;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	class UMaterialParameterCollection* MPC_LowHPEffectAsset;
};
