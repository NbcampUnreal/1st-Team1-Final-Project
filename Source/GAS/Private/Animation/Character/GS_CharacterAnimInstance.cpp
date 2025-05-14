// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/GS_CharacterAnimInstance.h"

#include "Character/Player/Seeker/GS_Seeker.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGS_CharacterAnimInstance::SetOwnerCharacter(AGS_Character* Character)
{
	OwnerCharacter = Character;
}

void UGS_CharacterAnimInstance::SetCharacterMovement(UCharacterMovementComponent* CharacterMovement)
{
	OwnerCharacterMovement = CharacterMovement;
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

bool UGS_CharacterAnimInstance::IsCharacterAimming()
{
	if (OwnerCharacter)
	{
		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter);
		if (Seeker)
		{
			return Seeker->GetAimState();
		}
	}
	return false;
}
