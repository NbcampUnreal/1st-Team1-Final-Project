// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GS_AnimInstance.h"
#include "GS_CharacterAnimInstance.generated.h"

class UCharacterMovementComponent;
class AGS_Character;

UCLASS()
class GAS_API UGS_CharacterAnimInstance : public UGS_AnimInstance
{
	GENERATED_BODY()
public:	
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetOwnerCharacter(AGS_Character* Character);
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetCharacterMovement(UCharacterMovementComponent* CharacterMovement);

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<AGS_Character> OwnerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UCharacterMovementComponent> OwnerCharacterMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundSpeed;
};
