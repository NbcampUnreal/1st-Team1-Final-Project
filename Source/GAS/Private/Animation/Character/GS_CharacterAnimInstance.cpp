// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/GS_CharacterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGS_CharacterAnimInstance::SetOwnerCharacter(AGS_Character* Character)
{
	OwnerCharacter = Character;
}

void UGS_CharacterAnimInstance::SetCharacterMovement(UCharacterMovementComponent* CharacterMovement)
{
	OwnerCharacterMovement = CharacterMovement;
}

void UGS_CharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UGS_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter && OwnerCharacterMovement)
	{
		const FVector Velocity = OwnerCharacterMovement->Velocity;
		GroundSpeed = Velocity.Size2D();
	}
}
