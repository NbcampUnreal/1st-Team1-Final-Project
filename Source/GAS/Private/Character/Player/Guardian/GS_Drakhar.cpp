#include "Character/Player/Guardian/GS_Drakhar.h"

#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = true;
	DashPower = 6000.f;
}

void AGS_Drakhar::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsDashing);
}

void AGS_Drakhar::ComboAttack()
{
	Super::ComboAttack();

	if (IsLocallyControlled())
	{
		if (IsAttacking)
		{
			if (CanNextCombo)
			{
				IsComboInputOn = true;
			}
		}
		else
		{
			AttackStartComboState();
			ServerRPCComboAttack();
			IsAttacking = true;
		}
	}
}

//WingRush
void AGS_Drakhar::Skill1()
{
	Super::Skill1();

	if (IsLocallyControlled())
	{
		//TODO
		//GetSkillComp()->Activate();
		ServerRPCDashCharacter();
	}
}

//Earthquake
void AGS_Drakhar::Skill2()
{
}

//DraconicFury
void AGS_Drakhar::UltimateSkill()
{
}


void AGS_Drakhar::ServerRPCDashCharacter_Implementation()
{
	if (bIsDashing)
	{
		return;
	}

	bIsDashing = true;

	const float DesiredDistance = 600.f;
	const float Duration = 0.6f;
	const FVector Dir = GetActorForwardVector();
	const FVector DashVelocity = Dir * (DesiredDistance / Duration);

	auto* MoveComp = GetCharacterMovement();
	MoveComp->GroundFriction = 0.f;
	MoveComp->BrakingFrictionFactor = 0.f;
	MoveComp->BrakingDecelerationWalking = 0.f;

	MoveComp->Velocity = DashVelocity;

	FTimerHandle Th;
	GetWorldTimerManager().SetTimer(Th, this, &AGS_Drakhar::EndDash, Duration, false);
}

void AGS_Drakhar::EndDash()
{
	bIsDashing = false;

	GetCharacterMovement()->StopMovementImmediately();

	GetCharacterMovement()->GroundFriction = 8.f;
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 512.f;
}

void AGS_Drakhar::OnRep_IsDashing()
{
	if (bIsDashing)
	{
		if (DashMontage)
		{
			GuardianAnim->PlayDashMontage();			
		}
	}
	else
	{
		if (DashMontage)
		{
			StopAnimMontage(DashMontage);
		}
	}
}
