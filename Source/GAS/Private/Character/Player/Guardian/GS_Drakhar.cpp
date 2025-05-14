#include "Character/Player/Guardian/GS_Drakhar.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Component/GS_StatComp.h"

#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//stat init test
	GetStatComp()->SetCurrentHealth(2000.f);
	GetStatComp()->SetAttackPower(120.f);

	DashPower = 1000.f;
	bIsDashing = false;
	DashInterpAlpha = 0.f;
	DashDuration = 1.33f;

	DashAttackCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DashCapsule"));
	FName DashSocket(TEXT("spine_02_socket"));
	DashAttackCapsule->SetCapsuleHalfHeight(200.f);
	DashAttackCapsule->SetCapsuleRadius(100.f);
	DashAttackCapsule->SetupAttachment(GetMesh(), DashSocket);
	DashAttackCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_Drakhar::BeginPlay()
{
	Super::BeginPlay();

	//DashDuration = GuardianAnim->GetDashMontageDuration();

}

void AGS_Drakhar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
	if (HasAuthority())
	{
		if (bIsDashing)
		{
			DashAttackCheck();

			DashInterpAlpha += DeltaTime / DashDuration;

			if (DashInterpAlpha >= 1.f)
			{
				SetActorLocation(DashEndLocation);
				bIsDashing = false;
				EndDash();
			}
			else
			{
				const FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, DashInterpAlpha);
				SetActorLocation(NewLocation);
				DashStartLocation = NewLocation;
			}
		}
	}	
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
	Super::Skill2();

	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill2 Drakhar"));
		ServerRPCEarthquake();
	}
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
	DashInterpAlpha = 0.f;
	DashStartLocation = GetActorLocation();
	DashEndLocation = DashStartLocation + GetActorForwardVector() * DashPower;
}

void AGS_Drakhar::DashAttackCheck()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TArray<FHitResult> OutHitResults;	
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * 100.f;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeCapsule(100.f, 200.f), Params);

	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				DamagedCharacters.Add(DamagedCharacter);

				DrawDebugPoint(
					GetWorld(),
					OutHitResult.ImpactPoint,
					15.f,
					FColor::Yellow,
					false,
					1.f
				);
			}
		}		
	}
}

void AGS_Drakhar::ServerRPCEarthquake_Implementation()
{
	if (bIsEarthquaking)
	{
		return;
	}

	TSet<AGS_Character*> EarthquakeDamagedCharacters;
	TArray<FHitResult> OutHitResults;
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * 100.f;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(200.f), Params);

	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				EarthquakeDamagedCharacters.Add(DamagedCharacter);

				DrawDebugPoint(
					GetWorld(),
					OutHitResult.ImpactPoint,
					15.f,
					FColor::Yellow,
					false,
					1.f
				);
			}
		}
		for (auto const& DamagedCharacter : EarthquakeDamagedCharacters)
		{
			float Damage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter);
			//TODO
			//Damage += SkillDamage;
			FDamageEvent DamageEvent;
			DamagedCharacter->TakeDamage(Damage, DamageEvent, GetController(), this);
		}
	}
	DrawDebugSphere(GetWorld(), Start, 100.f, 10, FColor::Red, false, 2.f);
}

void AGS_Drakhar::EndDash()
{
	bIsDashing = false;
	if (DamagedCharacters.IsEmpty())
	{
		return;
	}

	for (auto const& DamagedCharacter : DamagedCharacters)
	{
		float Damage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter);
		FDamageEvent DamageEvent;
		DamagedCharacter->TakeDamage(Damage, DamageEvent, GetController(), this);
	}

	DamagedCharacters.Empty();

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AGS_Drakhar::OnRep_IsDashing()
{
	if (bIsDashing)
	{
		GuardianAnim->PlayDashMontage();			
		/*if (DashMontage)
		{

		}*/
	}
	else
	{
		GuardianAnim->StopDashMontage();
		/*if (DashMontage)
		{
			StopAnimMontage(DashMontage);
		}*/		
	}
}

void AGS_Drakhar::OnRep_IsEarthquaking()
{

}
