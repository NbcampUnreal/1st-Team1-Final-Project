#include "Character/Player/Guardian/GS_Drakhar.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Component/GS_StatComp.h"
#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"

#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//combo attack variables
	CurrentComboAttackIndex = 0;
	MaxComboAttackIndex = 3;

	bIsComboAttacking = false;
	bCanDoNextComboAttack = false;

	//dash skill variables
	DashPower = 1000.f;
	bIsDashing = false;
	DashInterpAlpha = 0.f;
	DashDuration = 1.33f;

	//earthquake skill variables
	bIsEarthquaking = false;
	EarthquakePower = 3000.f;
	EarthquakeRadius = 500.f;
}

void AGS_Drakhar::BeginPlay()
{
	Super::BeginPlay();

	if (GuardianAnim)
	{
		GuardianAnim->OnMontageEnded.AddDynamic(this, &AGS_Drakhar::OnMontageEnded);
	}
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

	DOREPLIFETIME(ThisClass, bIsComboAttacking);
	DOREPLIFETIME(ThisClass, bCanDoNextComboAttack);
	DOREPLIFETIME(ThisClass, CurrentComboAttackIndex);

	DOREPLIFETIME(ThisClass, bIsDashing);

	DOREPLIFETIME(ThisClass, bIsEarthquaking);
}

void AGS_Drakhar::ComboAttack()
{
	if (IsLocallyControlled())
	{
		//play montage immediately
		if (!ClientComboAttacking)
		{
			GuardianAnim->PlayComboAttackMontage(ClientComboAttackIndex);			
		}
		//server RPC for check variables
		ServerRPCComboAttack();
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
		ServerRPCEarthquake();
	}
}

//DraconicFury
void AGS_Drakhar::UltimateSkill()
{
	Super::UltimateSkill();

	if (IsLocallyControlled())
	{
		
	}
}

void AGS_Drakhar::ServerRPCComboAttack_Implementation()
{
	//prevent attack stacking
	if (bIsComboAttacking)
	{
		bCanDoNextComboAttack = true;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		return;
	}

	MulticastRPCPlayComboAttackMontage();
	bIsComboAttacking = true;
	bCanDoNextComboAttack = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
}

void AGS_Drakhar::MulticastRPCPlayComboAttackMontage_Implementation()
{
	if (IsLocallyControlled() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	GuardianAnim->PlayComboAttackMontage(ClientComboAttackIndex);
}

void AGS_Drakhar::ServerRPCComboAttackCheck_Implementation()
{
	MeleeAttackCheck();

	if (CurrentComboAttackIndex == MaxComboAttackIndex - 1)
	{
		const FVector StartLocation = GetActorLocation() + GetActorForwardVector() * 200.f + FVector(0.f, 0.f, 50.f);
		AGS_DrakharProjectile* DrakharProjectile = GetWorld()->SpawnActor<AGS_DrakharProjectile>(Projectile, StartLocation, GetActorRotation());
		if (DrakharProjectile)
		{
			DrakharProjectile->SetOwner(this);
		}
	}
}

void AGS_Drakhar::ServerRPCComboAttackEnd_Implementation()
{
	if (bCanDoNextComboAttack)
	{
		CurrentComboAttackIndex++;
		CurrentComboAttackIndex %= MaxComboAttackIndex;
		UE_LOG(LogTemp, Warning, TEXT("combo attack input"));
	}
	else
	{
		CurrentComboAttackIndex = 0;
		//not call
		UE_LOG(LogTemp, Warning, TEXT("combo attack end %d"), CurrentComboAttackIndex);
	}
	bIsComboAttacking = false;
	bCanDoNextComboAttack = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AGS_Drakhar::OnRep_IsComboAttacking()
{
	ClientComboAttacking = bIsComboAttacking;
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("client combo attacking index %d"), ClientComboAttackIndex), true, true, FLinearColor::Green, 5.f);
}

void AGS_Drakhar::OnRep_CurrentComboAttackIndex()
{
	ClientComboAttackIndex = CurrentComboAttackIndex;

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("client combo attacking index %d"), ClientComboAttackIndex), true, true, FLinearColor::Green, 5.f);
}

void AGS_Drakhar::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		ClientComboAttacking = false;
	}
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
	}
	else
	{
		GuardianAnim->StopDashMontage();	
	}
}

void AGS_Drakhar::ServerRPCEarthquake_Implementation()
{
	if (bIsEarthquaking)
	{
		return;
	}

	bIsEarthquaking = true;
}

void AGS_Drakhar::ServerRPCEarthquakeEnd_Implementation()
{
	bIsEarthquaking = false;
}

void AGS_Drakhar::ServerRPCEarthquakeAttackCheck_Implementation()
{
	TSet<AGS_Character*> EarthquakeDamagedCharacters;
	TArray<FHitResult> OutHitResults;
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * 100.f;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(EarthquakeRadius), Params);

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
			DamagedCharacter->LaunchCharacter(GetActorForwardVector() * EarthquakePower, false, false);
		}
	}
}

void AGS_Drakhar::OnRep_IsEarthquaking()
{
	if (bIsEarthquaking)
	{
		GuardianAnim->PlayEarthquakeMontage();
	}
	else
	{
		GuardianAnim->StopEarthquakeMontage();
	}
}

