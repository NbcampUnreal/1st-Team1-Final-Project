#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AGS_Guardian::AGS_Guardian()
{
	PrimaryActorTick.bCanEverTick = false;
	
	NormalMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SpeedUpMoveSpeed = 1200.f;
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
	DOREPLIFETIME(ThisClass, MoveSpeed);
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

void AGS_Guardian::OnRep_MoveSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AGS_Guardian::MeleeAttackCheck()
{
	if (HasAuthority())
	{
		GuardianState = EGuardianState::CtrlSkillEnd;

		const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float MeleeAttackRange = 200.f;
		const float MeleeAttackRadius = 200.f;
		
		TSet<AGS_Character*> DamagedCharacters = DetectPlayerInRange(Start, MeleeAttackRange, MeleeAttackRadius);
		ApplyDamageToDetectedPlayer(DamagedCharacters, 0.f);
	}
}

TSet<AGS_Character*> AGS_Guardian::DetectPlayerInRange(const FVector& Start, float SkillRange, float Radius)
{
	TArray<FHitResult> OutHitResults;
	TSet<AGS_Character*> DamagedPlayers;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);

	FVector End = Start + GetActorForwardVector() * SkillRange;
	
	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, End, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);

	//FOR DEBUGGING
	//MulticastRPCDrawDebugSphere(bIsHitDetected, End, Radius);
	
	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			if (OutHitResult.GetComponent() && OutHitResult.GetComponent()->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}

			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				DamagedPlayers.Add(DamagedCharacter);
			}
		}
	}

	return DamagedPlayers;
}

void AGS_Guardian::ApplyDamageToDetectedPlayer(const TSet<AGS_Character*>& DamagedCharacters, float PlusDamge)
{
	for (auto const& DamagedCharacter : DamagedCharacters)
	{
		//ServerRPCMeleeAttack(DamagedCharacter);
		UGS_StatComp* DamagedCharacterStat = DamagedCharacter->GetStatComp();
		if (IsValid(DamagedCharacterStat))
		{
			float Damage = DamagedCharacterStat->CalculateDamage(this, DamagedCharacter);
			FDamageEvent DamageEvent;
			DamagedCharacter->TakeDamage(Damage + PlusDamge, DamageEvent, GetController(),this);

			//server
			AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(this);
			if (!Drakhar->GetIsFeverMode())
			{
				Drakhar->MulticastRPCSetFeverGauge(10.f);
			}
		}
	}
}


void AGS_Guardian::OnRep_GuardianState()
{
	ClientGuardianState = GuardianState;
}

void AGS_Guardian::QuitGuardianSkill()
{
	UE_LOG(LogTemp, Warning, TEXT("quit skill"));

	//reset skill state
	GuardianState = EGuardianState::CtrlSkillEnd;

	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(this);
	if (Drakhar)
	{
		Drakhar->ServerRPCResetValue();
	}
	//fly end
	GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
}

void AGS_Guardian::MulticastRPCDrawDebugSphere_Implementation(bool bIsOverlap, const FVector& Location, float CapsuleRadius)
{
	FColor DebugColor = bIsOverlap ? FColor::Green : FColor::Red;
	DrawDebugSphere(GetWorld(), Location,  CapsuleRadius, 16,DebugColor, false, 2.0f, 0, 1.0f);
}