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
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Character/Component/GS_DebuffVFXComponent.h"

AGS_Guardian::AGS_Guardian()
{
	PrimaryActorTick.bCanEverTick = false;
	
	NormalMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SpeedUpMoveSpeed = 1200.f;

	// ====================
	// 디버프 VFX 컴포넌트 생성
	// ====================
	DebuffVFXComponent = CreateDefaultSubobject<UGS_DebuffVFXComponent>("DebuffVFXComponent");
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
		TArray<FHitResult> OutHitResults;
		TSet<AGS_Character*> DamagedCharacters;
		FCollisionQueryParams Params(NAME_None, false, this);

		const float MeleeAttackRange = 200.f;
		const float MeleeAttackRadius = 150.f;

		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * MeleeAttackRange;

		bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(MeleeAttackRadius), Params);
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
					DamagedCharacters.Add(DamagedCharacter);
				}
			}

			for (auto const& DamagedCharacter : DamagedCharacters)
			{
				//ServerRPCMeleeAttack(DamagedCharacter);
				UGS_StatComp* DamagedCharacterStat = DamagedCharacter->GetStatComp();
				if (IsValid(DamagedCharacterStat))
				{
					float Damage = DamagedCharacterStat->CalculateDamage(this, DamagedCharacter);
					FDamageEvent DamageEvent;
					DamagedCharacter->TakeDamage(Damage, DamageEvent, GetController(),this);

					//server
					AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(this);
					if (!Drakhar->GetIsFeverMode())
					{
						Drakhar->MulticastRPCSetFeverGauge(10.f);
					}
				}
			}
		}
		//MulticastRPCDrawDebugLine(Start, End, MeleeAttackRange, MeleeAttackRadius, Forward, bIsHitDetected);
	}
}

void AGS_Guardian::CheckAttackRange(float AttackRange, float AttackRadius)
{
	TArray<FHitResult> OutHitResults;
	TSet<AGS_Character*> DamagedCharacters;
	FCollisionQueryParams Params(NAME_None, false, this);

	const float MeleeAttackRange = 200.f;
	const float MeleeAttackRadius = 150.f;

	const FVector Forward = GetActorForwardVector();
	const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + GetActorForwardVector() * MeleeAttackRange;

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(MeleeAttackRadius), Params);
}

void AGS_Guardian::AttackCheck()
{
	
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

	//fly end
	GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
}

void AGS_Guardian::MulticastRPCDrawDebug_Implementation(const FVector& Start,float Radius, bool bHit )
{
	DrawDebugSphere(
	   GetWorld(), Start, Radius, 16,
	   bHit ? FColor::Red : FColor::Green,
	   false, 3.f, 0, 1.f);
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.f);
}

void AGS_Guardian::MulticastRPCDrawDebugLine_Implementation(const FVector& Start, const FVector& End, float CapsuleRange, float Radius, const FVector& Forward, bool bIsHit)
{
	FColor Color = bIsHit ? FColor::Green : FColor::Red;
	const FVector Origin = Start + (End - Start) * 0.5f;
	DrawDebugCapsule(GetWorld(), Origin, CapsuleRange * 0.5f, Radius, FRotationMatrix::MakeFromZ(Forward).ToQuat(),
		Color, false, 5.f);
}

void AGS_Guardian::MulticastRPCDrawDebugCapsule_Implementation(bool bIsOverlap, const FVector& PillarLocation, float PillarHalfHeight, float PillarRadius)
{
	FColor DebugColor = bIsOverlap ? FColor::Green : FColor::Red;
	DrawDebugCapsule(
		GetWorld(),
		PillarLocation,       // 캡슐의 중심 위치
		PillarHalfHeight,     // 캡슐의 절반 높이
		PillarRadius,         // 캡슐의 반지름
		FQuat::Identity,      // 회전 (수직 캡슐이므로 기본값)
		DebugColor,           // 표시할 색상
		false,                // 영구적으로 표시할지 여부
		2.0f,                 // 표시될 시간 (초)
		0,                    // 우선순위 (보통 0)
		1.0f                  // 선 두께
	);
}