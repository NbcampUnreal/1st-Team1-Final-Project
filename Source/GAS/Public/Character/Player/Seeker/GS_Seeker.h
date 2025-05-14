// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "GS_Seeker.generated.h"

class UGS_SkillInputHandlerComp;

UENUM(BlueprintType)
enum class ESeekerAttackMode : uint8
{
	NonAttackMode,
	AimAttackMode
};

USTRUCT(BlueprintType)
struct FSeekerState
{
	GENERATED_BODY()

	FSeekerState()
		: IsAim(false)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsAim;
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
	
	// Change Mode or State
	UFUNCTION(BlueprintCallable)
	void SetAttackMode(ESeekerAttackMode Mode);

	UFUNCTION(BlueprintCallable, Category="Mode")
	ESeekerAttackMode GetAttackMode();

	UFUNCTION(BlueprintCallable)
	void SetAimState(bool IsAim);

	UFUNCTION(BlueprintCallable, Category = "State")
	bool GetAimState();

	UFUNCTION(BlueprintCallable, Category="State", meta=(BlueprintThreadSafe))
	bool GetAimState_ThreadSafe(){return SeekerState.IsAim;};

	//UFUNCTION(BlueprintCallable, Category = "State", meta=(BlueprintThreadSafe))
	//bool GetAimState_hreadSafe();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Action Function
	UFUNCTION()
	virtual void LeftClickPressed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	UGS_SkillInputHandlerComp* SkillInputHandlerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon", meta=(AllowPrivateAccess="true"))
	UChildActorComponent* Weapon;
	
private:
	UPROPERTY(VisibleAnywhere, Category="Mode")
	ESeekerAttackMode AttackMode;

	UPROPERTY(VisibleAnywhere, Category="State")
	FSeekerState SeekerState;
};
