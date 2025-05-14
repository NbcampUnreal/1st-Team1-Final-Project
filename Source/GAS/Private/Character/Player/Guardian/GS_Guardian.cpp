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

	MaxCombo = 3;
	AttackEndComboState();
}

void AGS_Guardian::BeginPlay()
{
	Super::BeginPlay();

}

void AGS_Guardian::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	GuardianAnim = Cast<UGS_DrakharAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(GuardianAnim))
	{
		GuardianAnim->OnMontageEnded.AddDynamic(this, &AGS_Guardian::OnAttackMontageEnded);
		GuardianAnim->OnNextAttackCheck.AddLambda([this]()->void
			{
				CanNextCombo = false;

				if (IsComboInputOn)
				{
					AttackStartComboState();

					if (HasAuthority())
					{
						MulticastRPCJumpToAttackMontageSection(CurrentCombo);
					}
					else
					{
						ServerRPCJumpToAttackMontageSection(CurrentCombo);
					}
				}
			});
	}
}

void AGS_Guardian::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, IsAttacking);
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

void AGS_Guardian::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	IsAttacking = false;
	AttackEndComboState();

}

void AGS_Guardian::AttackStartComboState()
{
	CanNextCombo = true;
	IsComboInputOn = false;
	CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, MaxCombo);
}

void AGS_Guardian::AttackEndComboState()
{
	IsComboInputOn = false;
	CanNextCombo = false;
	CurrentCombo = 0;
}

void AGS_Guardian::OnRep_Attacking()
{
	if (IsAttacking)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void AGS_Guardian::ServerRPCComboAttack_Implementation()
{	
	MeleeAttackCheck();
	MulticastRPCComboAttack();
}

bool AGS_Guardian::ServerRPCComboAttack_Validate()
{
	return true;
}

void AGS_Guardian::MulticastRPCComboAttack_Implementation()
{	
	GuardianAnim->PlayAttackMontage();
}

void AGS_Guardian::MulticastRPCJumpToAttackMontageSection_Implementation(int32 ComboIndex)
{
	GuardianAnim->JumpToAttackMontageSection(ComboIndex);
}

void AGS_Guardian::ServerRPCJumpToAttackMontageSection_Implementation(int32 ComboIndex)
{
	MulticastRPCJumpToAttackMontageSection(ComboIndex);
}

bool AGS_Guardian::ServerRPCJumpToAttackMontageSection_Validate(int32 ComboIndex)
{		
	return ComboIndex > 0 && ComboIndex <= MaxCombo;
}