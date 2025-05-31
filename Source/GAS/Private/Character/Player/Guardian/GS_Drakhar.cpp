#include "Character/Player/Guardian/GS_Drakhar.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Components/CapsuleComponent.h"
#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"

#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = false;
	
	//combo attack variables
	CurrentComboAttackIndex = 0;
	MaxComboAttackIndex = 3;

	bIsComboAttacking = false;
	bCanDoNextComboAttack = false;
	ClientNextComboAttack = false;

	//dash skill variables
	DashPower = 1500.f;
	DashInterpAlpha = 0.f;
	DashDuration = 1.f;

	//earthquake skill variables
	EarthquakePower = 3000.f;
	EarthquakeRadius = 500.f;

	//Guardian State Setting
	ClientGuardianState = EGuardianState::None;
}

void AGS_Drakhar::BeginPlay()
{
	Super::BeginPlay();

	if (GuardianAnim)
	{
		GuardianAnim->OnMontageEnded.AddDynamic(this, &AGS_Drakhar::OnMontageEnded);
	}
	GuardianState = EGuardianState::CtrlSkillEnd;
}

void AGS_Drakhar::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsComboAttacking);
	DOREPLIFETIME(ThisClass, bCanDoNextComboAttack);
	DOREPLIFETIME(ThisClass, CurrentComboAttackIndex);
}

void AGS_Drakhar::Ctrl()
{
	if (IsLocallyControlled())
	{
		if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			GetSkillComp()->TryActivateSkill(ESkillSlot::Ready);
			ServerRPCStartCtrl();
		}
	}
}

void AGS_Drakhar::CtrlStop()
{
	if (IsLocallyControlled())
	{		
		GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
		ServerRPCStopCtrl();
	}
}

void AGS_Drakhar::LeftMouse()
{
	Super::LeftMouse();

	if (IsLocallyControlled())
	{
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready))
		{
			//earthquake
			GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}

		//not flying
		else if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			if (!ClientComboAttacking)
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("melee attack")), true, true, FLinearColor::Blue, 5.f);
				GuardianAnim->PlayComboAttackMontage(ClientComboAttackIndex);
			}
			ServerRPCComboAttack();
		}
	}
}

void AGS_Drakhar::RightMouse()
{
	if (IsLocallyControlled())
	{
		//ultimate skill
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready))
		{	
			GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
		}
		//dash skill
		else if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
		}
		
	}
}

void AGS_Drakhar::ServerRPCComboAttack_Implementation()
{
	//다음 공격 되는 것이 확정인 경우
	if (bCanDoNextComboAttack)
	{
		//UE_LOG(LogTemp, Warning, TEXT("can combo attack ????? %d"), bIsComboAttacking);				
	}

	//prevent attack stacking
	if (bIsComboAttacking)
	{
		//이미 공격하고 있는 중에 클릭을 입력받으면 CanDoNext를 true로
		bCanDoNextComboAttack = true;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		return;
	}
	
	//이미 공격하고 있는 중이 아니라면,
	//몽타주 재생시켜주고
	MulticastRPCPlayComboAttackMontage();
	
	//공격 몽타주 못하게 막기
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
	//만약 다음 공격을 할 수 있는 상황이라면,
	if (bCanDoNextComboAttack)
	{
		//인덱스 증가
		CurrentComboAttackIndex++;
		CurrentComboAttackIndex %= MaxComboAttackIndex;
	}
	//아니라면
	else
	{
		//초기화
		CurrentComboAttackIndex = 0;
	}

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("attack server end")), true, true, FLinearColor::Blue, 5.f);

	//다음 몽타주 실행할 수 있게
	ResetComboAttackVariables();
}

void AGS_Drakhar::OnRep_IsComboAttacking()
{
	ClientComboAttacking = bIsComboAttacking;
}

void AGS_Drakhar::OnRep_CurrentComboAttackIndex()
{
	ClientComboAttackIndex = CurrentComboAttackIndex;
}

void AGS_Drakhar::OnRep_CanDoNextComboAttack()
{
	ClientNextComboAttack = bCanDoNextComboAttack;
}

void AGS_Drakhar::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
}

void AGS_Drakhar::ResetComboAttackVariables()
{
	bIsComboAttacking = false;
	bCanDoNextComboAttack = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AGS_Drakhar::ServerRPCDoDash_Implementation(float DeltaTime)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	DashInterpAlpha += DeltaTime / DashDuration;

	DashAttackCheck();
	
	if (DashInterpAlpha >= 1.f)
	{
		SetActorLocation(DashEndLocation);
	}
	else
	{
		const FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, DashInterpAlpha);
		SetActorLocation(NewLocation, true);
		DashStartLocation = NewLocation;
	}
}

void AGS_Drakhar::ServerRPCEndDash_Implementation()
{
	if (DamagedCharacters.IsEmpty())
	{
		return;
	}
	
	for (auto const& DamagedCharacter : DamagedCharacters)
	{
		float SkillDamage = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Moving)->Damage;
		FDamageEvent DamageEvent;
		DamagedCharacter->TakeDamage(SkillDamage, DamageEvent, GetController(), this);
	}

	DamagedCharacters.Empty();
	GuardianState = EGuardianState::None;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AGS_Drakhar::ServerRPCCalculateDashLocation_Implementation()
{
	DashInterpAlpha = 0.f;
	DashStartLocation = GetActorLocation();
	DashEndLocation = DashStartLocation + GetActorForwardVector() * DashPower;
}

void AGS_Drakhar::DashAttackCheck()
{
	TArray<FHitResult> OutHitResults;	
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * 100.f;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity,
		ECC_Camera, FCollisionShape::MakeCapsule(100.f, 200.f), Params);
	
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
	}
}

void AGS_Drakhar::ServerRPCEarthquakeAttackCheck_Implementation()
{
	TSet<AGS_Character*> EarthquakeDamagedCharacters;
	TArray<FHitResult> OutHitResults;
	const FVector Start = GetActorLocation() + 200.f;
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
			}
		}
		for (auto const& DamagedCharacter : EarthquakeDamagedCharacters)
		{
			float SkillDamage = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Aiming)->Damage;
			
			FDamageEvent DamageEvent;
			DamagedCharacter->TakeDamage(SkillDamage, DamageEvent, GetController(), this);
			DamagedCharacter->LaunchCharacter(GetActorForwardVector() * EarthquakePower, false, false);
		}
	}
	MulticastRPCDrawDebugLine(Start, End, 100.f, EarthquakeRadius, GetActorForwardVector(),bIsHitDetected);
}

void AGS_Drakhar::ServerRPCSpawnDraconicFury_Implementation()
{
	GetRandomDraconicFuryTarget();

	int32 Index = FMath::RandRange(0, DraconicFuryTargetArray.Num() - 1);

	UE_LOG(LogTemp, Warning, TEXT("Spawn"));

	//spawn meteor
	AGS_DrakharProjectile* DrakharProjectile = GetWorld()->SpawnActor<AGS_DrakharProjectile>(DraconicProjectile, DraconicFuryTargetArray[Index].GetLocation(), DraconicFuryTargetArray[Index].Rotator());
	if (DrakharProjectile)
	{
		DrakharProjectile->SetOwner(this);
	}
}

void AGS_Drakhar::GetRandomDraconicFuryTarget()
{
	DraconicFuryTargetArray.Empty();

	for (int i = 0; i < 5; ++i)
	{
		FVector StartLocation = GetActorLocation();
		FVector Offset = GetActorForwardVector() * 200.f + FVector(FMath::FRandRange(-300.f, 300.f), FMath::FRandRange(-300.f, 300.f), FMath::FRandRange(500.f, 600.f));

		StartLocation += Offset;

		FRotator StartRotation = GetActorRotation();
		float RandomPitch = FMath::FRandRange(-35.f, -30.f);
		StartRotation.Pitch += RandomPitch;

		FTransform StartTransform = FTransform(StartRotation, StartLocation);
		DraconicFuryTargetArray.Add(StartTransform);
	}
}