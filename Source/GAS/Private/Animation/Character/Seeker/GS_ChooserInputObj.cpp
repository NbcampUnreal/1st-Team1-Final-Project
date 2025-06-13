// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/GS_ChooserInputObj.h"
#include "Kismet/KismetMathLibrary.h"


bool UGS_ChooserInputObj::ShouldTurnInPlace()
{
	if ((MovementState == EMovementState::Idle && LastMovementState == EMovementState::Moving) || bMustTurnInPlace)
	{
		const FRotator CharacterRot = CharacterTransform.GetRotation().Rotator();
		const FRotator RootRot = RootTransform.GetRotation().Rotator();
		
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRot, RootRot);
		DeltaRot.Yaw = FMath::Abs(DeltaRot.Yaw);
		if (DeltaRot.Yaw >= 50)
		{
			return true;
		}
	}
	return false;
}

bool UGS_ChooserInputObj::IsMoving()
{
	bool bVelocityNotZero       = !Velocity.Equals(FVector::ZeroVector, 0.1f);
	bool bFutureVelocityNotZero = !FutureVelocity.Equals(FVector::ZeroVector, 0.1f);
	
	return bVelocityNotZero && bFutureVelocityNotZero;
}

bool UGS_ChooserInputObj::IsStarting()
{
	bool bContains = CurrentDatabasesTags.Contains(TEXT("Pivots"));
	
	if (FutureVelocity.Size2D() > Velocity.Size2D() + 100.0f)
	{
		if (!bContains && IsMoving())
		{
			return true;
		}
	}
	return false;
}

bool UGS_ChooserInputObj::IsPivoting()
{
	const FRotator CurrentRot = Velocity.Rotation();
	const FRotator FutureRot = FutureVelocity.Rotation();

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRot, FutureRot);
	DeltaRot.Yaw = FMath::Abs(DeltaRot.Yaw);

	float Threshold = 0.f;
	switch (RotationMode)
	{
	case ERotationMode::OrientToMovement :
		Threshold = 60.0f;
		break;
	case ERotationMode::Strafe :
		Threshold = 40.0f;
	}

	if (DeltaRot.Yaw > Threshold)
	{
		return true;
	}
	return false;
}

bool UGS_ChooserInputObj::ShouldSpinTransition()
{
	bool bContains = CurrentDatabasesTags.Contains(TEXT("Pivots"));

	const FRotator CharacterRot = CharacterTransform.GetRotation().Rotator();
	const FRotator RootRot = RootTransform.GetRotation().Rotator();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRot, RootRot);
	DeltaRot.Yaw = FMath::Abs(DeltaRot.Yaw);
	
	if (bContains && DeltaRot.Yaw >= 130.0f && Speed2D >= 150.0f)
	{
		return true;
	}
	
	return false;
}
