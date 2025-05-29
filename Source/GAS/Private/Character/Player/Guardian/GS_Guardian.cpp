#include "Character/Player/Guardian/GS_Guardian.h"

#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"

#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AGS_Guardian::AGS_Guardian()
{
	PrimaryActorTick.bCanEverTick = false;

	FeverTime = 0.f;
	FeverGage = 0.f;
}

void AGS_Guardian::BeginPlay()
{
	Super::BeginPlay();
}

void AGS_Guardian::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	GuardianAnim = Cast<UGS_DrakharAnimInstance>(GetMesh()->GetAnimInstance());
}

void AGS_Guardian::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GuardianState);
}

void AGS_Guardian::LeftMouse()
{
}

void AGS_Guardian::Ctrl()
{
}

void AGS_Guardian::CtrlStop()
{
}

void AGS_Guardian::RightMouse()
{
}

void AGS_Guardian::MeleeAttackCheck()
{
	TArray<FHitResult> OutHitResults;
	TSet<AGS_Character*> DamagedCharacters;
	FCollisionQueryParams Params(NAME_None, false, this);

	const float MeleeAttackRange = 100.f;
	const float MeleeAttackRadius = 80.f;

	const FVector Forward = GetActorForwardVector();
	const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + GetActorForwardVector() * MeleeAttackRange;

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(MeleeAttackRadius), Params);
	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				DamagedCharacters.Add(DamagedCharacter);
			}
		}

		for (auto const& DamagedCharacter : DamagedCharacters)
		{
			ServerRPCMeleeAttack(DamagedCharacter);
		}
	}

	MulticastRPCDrawDebugLine(Start, End, MeleeAttackRange, MeleeAttackRadius, Forward, bIsHitDetected);
}

void AGS_Guardian::ServerRPCStartCtrl_Implementation()
{
	GuardianState = EGuardianState::CtrlUp;
}

void AGS_Guardian::ServerRPCStopCtrl_Implementation()
{
	GuardianState = EGuardianState::CtrlSkillEnd;
}

void AGS_Guardian::OnRep_GuardianState()
{
	ClientGuardianState = GuardianState;
}

void AGS_Guardian::MulticastRPCDrawDebugLine_Implementation(const FVector& Start, const FVector& End, float CapsuleRange, float Radius, const FVector& Forward, bool bIsHit)
{
	FColor Color = bIsHit ? FColor::Green : FColor::Red;
	const FVector Origin = Start + (End - Start) * 0.5f;
	DrawDebugCapsule(GetWorld(), Origin, CapsuleRange * 0.5f, Radius, FRotationMatrix::MakeFromZ(Forward).ToQuat(),
		Color, false, 5.f);
}
