// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"


UGS_SeekerAnimInstance::UGS_SeekerAnimInstance()
{
	ChooserInputObj = nullptr;
}

void UGS_SeekerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	AGS_Seeker* OwnerPawn = Cast<AGS_Seeker>(TryGetPawnOwner());
	if (OwnerPawn)
	{
		OwnerCharacter = OwnerPawn;
		OwnerCharacterMovement =  OwnerCharacter->GetCharacterMovement();

		if (OwnerPawn->HasAuthority())
		{
			bUseOffsetRootBone = true;
		}
	}
	
	ChooserInputObj = NewObject<UGS_ChooserInputObj>(this);
	ChooserInputObj->MovementState = EMovementState::Idle;
	ChooserInputObj->RotationMode = ERotationMode::OrientToMovement;
	ChooserInputObj->Gait = EGait::Run;
}

void UGS_SeekerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (OwnerCharacter)
	{
		UpdateEssentialValue();
		UpdateTrajectory();
		UpdateState();
	}
}

void UGS_SeekerAnimInstance::UpdateEssentialValue_Implementation()
{	
	// Set Character Transform
	ChooserInputObj->CharacterTransform = OwnerCharacter->GetActorTransform();
	
	// Set Character Acceleration
	Acceleration = OwnerCharacterMovement->GetCurrentAcceleration();

	// Set Character Velocity
	VelocityLastFrame = ChooserInputObj->Velocity;
	ChooserInputObj->Velocity = OwnerCharacterMovement->Velocity;
	ChooserInputObj->Speed2D = UKismetMathLibrary::VSizeXY(ChooserInputObj->Velocity);

	const float WorldDT = UGameplayStatics::GetWorldDeltaSeconds(OwnerCharacter->GetWorld());
	const float SafeDT = FMath::Max(WorldDT, 0.001f);

	VelocityAcceleration = (ChooserInputObj->Velocity - VelocityLastFrame) / SafeDT;

	if (ChooserInputObj->Speed2D > 5)
	{
		LastNonZeroVelocity = ChooserInputObj->Velocity;
	}

	bIsMoving = ChooserInputObj->IsMoving();
}

void UGS_SeekerAnimInstance::UpdateState_Implementation()
{	
	// Set Rotation Mode
	LastRotationMode = ChooserInputObj->RotationMode;
	if (OwnerCharacterMovement->bOrientRotationToMovement)
	{
		ChooserInputObj->RotationMode = ERotationMode::OrientToMovement;
	}
	else
	{
		ChooserInputObj->RotationMode = ERotationMode::Strafe;
	}

	// Set Movement State
	ChooserInputObj->LastMovementState = ChooserInputObj->MovementState;
	if (ChooserInputObj->IsMoving())
	{
		OwnerCharacter->bUseControllerRotationYaw = true;
		ChooserInputObj->MovementState = EMovementState::Moving;
		if (ChooserInputObj)
		{
			ChooserInputObj->MovementState = EMovementState::Moving;
		}
	}
	else
	{
		OwnerCharacter->bUseControllerRotationYaw = false;
		ChooserInputObj->MovementState = EMovementState::Idle;
		if (ChooserInputObj)
		{
			ChooserInputObj->MovementState = EMovementState::Idle;
		}
	}

	// Set Gait State
	LastGait = ChooserInputObj->Gait;
}

bool UGS_SeekerAnimInstance::GetMustTurnInPlace()
{
	return ChooserInputObj->bMustTurnInPlace;
}

void UGS_SeekerAnimInstance::SetMustTurnInPlace(bool MustTurn)
{
	ChooserInputObj->bMustTurnInPlace = MustTurn;
}

float UGS_SeekerAnimInstance::GetOffsetRootTranslationHalfLife()
{
	switch (ChooserInputObj->MovementState)
	{
		case EMovementState::Idle :
			return 0.15;
		case EMovementState::Moving:
			return 0.4;
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
		if (FVector::DotProduct(Acceleration, ChooserInputObj->Velocity) > 0)
		{
			FVector ClampedMaxVector = VelocityAcceleration.GetClampedToMaxSize(MaxAcceration);
			return ChooserInputObj->CharacterTransform.GetRotation().UnrotateVector(ClampedMaxVector/MaxAcceration);
		}
		else
		{
			FVector ClampedMaxVector = VelocityAcceleration.GetClampedToMaxSize(MaxBrakingDeceleration);
			return ChooserInputObj->CharacterTransform.GetRotation().UnrotateVector(ClampedMaxVector/MaxBrakingDeceleration);
		}
	}
	
	return FVector::ZeroVector;
}

float UGS_SeekerAnimInstance::Get_LeanAmount()
{
	if (ChooserInputObj)
	{
		float ClampedLeanAmount = FMath::GetMappedRangeValueClamped(FVector2D(200.0, 500.0), FVector2D(0.5, 1.0), ChooserInputObj->Speed2D);
		return CalculateRelativeAccelerationAmount().Y * ClampedLeanAmount;
	}
	return 0.0f;
}

bool UGS_SeekerAnimInstance::EnableSteering()
{
	return ChooserInputObj->MovementState == EMovementState::Moving;
}

FVector2D UGS_SeekerAnimInstance::Get_AOValue()
{
	FVector2D AO = FVector2D::ZeroVector;
	
	if (OwnerCharacter && OwnerCharacter->GetController())
	{	const FRotator ControllerRot = OwnerCharacter->GetController()->GetControlRotation();
		const FRotator RootRot = ChooserInputObj->RootTransform.Rotator();
		
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControllerRot, RootRot);

		const float PitchMin = -80.f;
		const float PitchMax = 60.f;

		AO.X = FMath::GetMappedRangeValueClamped(FVector2D(PitchMin, PitchMax), FVector2D(-100.f, 100.f), DeltaRot.Pitch);
		AO.Y = DeltaRot.Yaw;

		/*AO.X = DeltaRot.Pitch;
		AO.Y = DeltaRot.Yaw;*/
	}
	return AO;
}

bool UGS_SeekerAnimInstance::Enable_AO()
{
	return FMath::Abs(Get_AOValue().X) < 90.0f && ChooserInputObj->RotationMode == ERotationMode::Strafe;
}

void UGS_SeekerAnimInstance::SetCurMontageSlot(ESeekerMontageSlot InputMontageSlot)
{
	uint8 BitFlag = 0;
	BitFlag |= (1 << static_cast<int32>(InputMontageSlot));
	CurMontageSlot = BitFlag;
}

bool UGS_SeekerAnimInstance::IsMontageSlotActive(ESeekerMontageSlot InputMontageSlot)
{
	uint8 BitFlag = 0;
	BitFlag |= (1 << static_cast<int32>(InputMontageSlot));
	return CurMontageSlot & BitFlag;
}


void UGS_SeekerAnimInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGS_SeekerAnimInstance, CurMontageSlot);
	/*DOREPLIFETIME(UGS_SeekerAnimInstance, IsPlayingUpperBodyMontage);
	DOREPLIFETIME(UGS_SeekerAnimInstance, IsPlayingFullBodyMontage);*/
}

