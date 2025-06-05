// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/GS_SeekerAnimInstance.h"

#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Elements/Framework/TypedElementQueryBuilder.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Net/UnrealNetwork.h"

UGS_SeekerAnimInstance::UGS_SeekerAnimInstance()
{
	MovementState = EMovementState::Idle;
	RotationMode = ERotationMode::OrientToMovement;
	Gait = EGait::Run;

	MotionMatchingPlayRate.Min = 1.0f;
	MotionMatchingPlayRate.Max = 1.0f;
}

void UGS_SeekerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	AGS_Seeker* OwnerPawn = Cast<AGS_Seeker>(TryGetPawnOwner());
	if (OwnerPawn)
	{
		OwnerCharacter = OwnerPawn;
		OwnerCharacterMovement =  OwnerCharacter->GetCharacterMovement();
	}
}

void UGS_SeekerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (OwnerCharacter)
	{
		UpdateEssentialValue();
		UpdateTrajectory();
		UpdateState();
		/*AGS_Player* playerdd = Cast<AGS_Player>(OwnerCharacter);
		if (playerdd->HasAuthority())
		{
			playerdd->GetSkillInputControl();
			UE_LOG(LogTemp, Warning, TEXT("Left Can : %s / Right Can : %s"), playerdd->GetSkillInputControl().CanInputLC ? TEXT("True") : TEXT("False"), playerdd->GetSkillInputControl().CanInputRC ? TEXT("True") : TEXT("False")); // SJE
		}*/
	}
}

void UGS_SeekerAnimInstance::UpdateEssentialValue_Implementation()
{	
	// Set Character Transform
	CharacterTransform = OwnerCharacter->GetActorTransform();

	// Set Character Acceleration
	Acceleration = OwnerCharacterMovement->GetCurrentAcceleration();

	// Set Character Velocity
	VelocityLastFrame = Velocity;
	Velocity = OwnerCharacterMovement->Velocity;
	Speed2D = UKismetMathLibrary::VSizeXY(Velocity);

	const float WorldDT = UGameplayStatics::GetWorldDeltaSeconds(OwnerCharacter->GetWorld());
	const float SafeDT = FMath::Max(WorldDT, 0.001f);

	VelocityAcceleration = (Velocity - VelocityLastFrame) / SafeDT;

	if (Speed2D > 5)
	{
		LastNonZeroVelocity = Velocity;
	}
}

void UGS_SeekerAnimInstance::UpdateState_Implementation()
{	
	// Set Rotation Mode
	LastRotationMode = RotationMode;
	if (OwnerCharacterMovement->bOrientRotationToMovement)
	{
		RotationMode = ERotationMode::OrientToMovement;
	}
	else
	{
		RotationMode = ERotationMode::Strafe;
	}

	// Set Movement State
	LastMovementState = MovementState;
	if (IsMoving())
	{
		MovementState = EMovementState::Moving;
	}
	else
	{
		MovementState = EMovementState::Idle;
	}

	// Set Gait State
	LastGait = Gait;
}

bool UGS_SeekerAnimInstance::IsMoving()
{
	bool bVelocityNotZero       = !Velocity.Equals(FVector::ZeroVector, 0.1f);
	bool bFutureVelocityNotZero = !FutureVelocity.Equals(FVector::ZeroVector, 0.1f);
	
	return bVelocityNotZero && bFutureVelocityNotZero;
}

bool UGS_SeekerAnimInstance::ShouldTurnInPlace()
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

bool UGS_SeekerAnimInstance::GetMustTurnInPlace()
{
	return bMustTurnInPlace;
}

void UGS_SeekerAnimInstance::SetMustTurnInPlace(bool MustTurn)
{
	bMustTurnInPlace = MustTurn;
}

bool UGS_SeekerAnimInstance::ShouldSpinTransition()
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

bool UGS_SeekerAnimInstance::IsPivoting()
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

bool UGS_SeekerAnimInstance::IsStarting()
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

float UGS_SeekerAnimInstance::GetOffsetRootTranslationHalfLife()
{
	switch (MovementState)
	{
		case EMovementState::Idle :
			return 0.1;
		case EMovementState::Moving:
			return 0.2;
	}
	return 0;
}

FVector UGS_SeekerAnimInstance::CalculateRelativeAccelerationAmount()
{
	if (!IsValid(OwnerCharacterMovement))
	{
		return FVector::ZeroVector;
	};
	
	float MaxAcceration = OwnerCharacterMovement->GetMaxAcceleration();
	float MaxBrakingDeceleration = OwnerCharacterMovement->GetMaxBrakingDeceleration();
	if (MaxAcceration > 0 && MaxBrakingDeceleration > 0)
	{
		if (FVector::DotProduct(Acceleration, Velocity) > 0)
		{
			FVector ClampedMaxVector = VelocityAcceleration.GetClampedToMaxSize(MaxAcceration);
			return CharacterTransform.GetRotation().UnrotateVector(ClampedMaxVector/MaxAcceration);
		}
		else
		{
			FVector ClampedMaxVector = VelocityAcceleration.GetClampedToMaxSize(MaxBrakingDeceleration);
			return CharacterTransform.GetRotation().UnrotateVector(ClampedMaxVector/MaxBrakingDeceleration);
		}
	}
	
	return FVector::ZeroVector;
}

float UGS_SeekerAnimInstance::Get_LeanAmount()
{
	float ClampedLeanAmount = FMath::GetMappedRangeValueClamped(FVector2D(200.0, 500.0), FVector2D(0.5, 1.0), Speed2D);
	return CalculateRelativeAccelerationAmount().Y * ClampedLeanAmount;
}

void UGS_SeekerAnimInstance::AnimNotify_ComboInput()
{
	if (APawn* OwnerPawn = TryGetPawnOwner())
	{
		if (AGS_Chan* Chan = Cast<AGS_Chan>(OwnerPawn))
		{
			Chan->ComboInputOpen();
		}
	}
}

void UGS_SeekerAnimInstance::AnimNotify_CanProceed()
{
	if (APawn* OwnerPawn = TryGetPawnOwner())
	{
		if (AGS_Chan* Chan = Cast<AGS_Chan>(OwnerPawn))
		{
			//Chan->CheckToNext();
		}
	}
}

void UGS_SeekerAnimInstance::AnimNotify_ComboEnd()
{
	if (APawn* OwnerPawn = TryGetPawnOwner())
	{
		if (AGS_Chan* Chan = Cast<AGS_Chan>(OwnerPawn))
		{
			//Chan->ComboEnd();
		}
	}
}

void UGS_SeekerAnimInstance::SetMotionMatchingPlayRate(float Min, float Max)
{
	MotionMatchingPlayRate.Min = Min;
	MotionMatchingPlayRate.Max = Max;
}

void UGS_SeekerAnimInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGS_SeekerAnimInstance, IsPlayingUpperBodyMontage);
	DOREPLIFETIME(UGS_SeekerAnimInstance, IsPlayingFullBodyMontage);
}
