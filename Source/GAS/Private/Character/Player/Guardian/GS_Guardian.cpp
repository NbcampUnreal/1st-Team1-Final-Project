#include "Character/Player/Guardian/GS_Guardian.h"

#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"

#include "Animation/AnimInstance.h"
#include "Engine/DamageEvents.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	
}

void AGS_Guardian::ComboAttack()
{

}

void AGS_Guardian::Skill1()
{	
}

void AGS_Guardian::Skill2()
{
}

void AGS_Guardian::UltimateSkill()
{
}

void AGS_Guardian::Ctrl()
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
	DrawDebugCapsule(GetWorld(), Start, MeleeAttackRange * 0.5f, MeleeAttackRadius, FRotationMatrix::MakeFromZ(Forward).ToQuat(), FColor::Red, false, 5.f);
}