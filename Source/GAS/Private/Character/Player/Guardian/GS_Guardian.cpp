#include "Character/Player/Guardian/GS_Guardian.h"

#include "Engine/DamageEvents.h"
#include "Components/CapsuleComponent.h"

AGS_Guardian::AGS_Guardian()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGS_Guardian::TestMeleeAttack()
{
	if (IsLocallyControlled())
	{
		TArray<FHitResult> OutHitResults;
		TSet<AGS_Character*> DamagedCharacters;
		FCollisionQueryParams Params(NAME_None, false, this);

		const float MeleeAttackRange = 50.f;
		const float MeleeAttackRadius = 50.f;

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
		DrawDebugCapsule(GetWorld(), Start, MeleeAttackRange*0.5f, MeleeAttackRadius, FRotationMatrix::MakeFromZ(Forward).ToQuat(), FColor::Red, false, 5.f);
	}
}

void AGS_Guardian::BeginPlay()
{
	Super::BeginPlay();
}
