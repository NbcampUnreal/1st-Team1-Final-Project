#include "Character/Player/Guardian/GS_Guardian.h"

#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"

#include "Animation/AnimInstance.h"
#include "Engine/DamageEvents.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

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


	//AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &AGS_Guardian::ComboAttackMontageNotifyBegin);
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
					GuardianAnim->JumpToAttackMontageSection(CurrentCombo);
				}
			});
	}
	
}

void AGS_Guardian::MulticastRPCComboAttack_Implementation()
{
	/*if (IsLocallyControlled())
	{		
	}*/
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
		GuardianAnim->PlayAttackMontage();
		GuardianAnim->JumpToAttackMontageSection(CurrentCombo);
		IsAttacking = true;
	}
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

		FDamageEvent DamageEvent;
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
