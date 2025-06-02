#include "Character/Player/Guardian/GS_Drakhar.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Components/CapsuleComponent.h"
#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"

#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = false;

	//combo attack
	DefaultComboAttackSectionName = FName("Combo1");
	ComboAttackSectionName = DefaultComboAttackSectionName;
	bCanCombo = true;
	bClientCanCombo = true;
	
	//dash skill variables
	DashPower = 1500.f;
	DashInterpAlpha = 0.f;
	DashDuration = 1.f;

	//earthquake skill variables
	EarthquakePower = 3000.f;
	EarthquakeRadius = 500.f;

	//Guardian State Setting
	ClientGuardianState = EGuardianState::CtrlSkillEnd;
}

void AGS_Drakhar::BeginPlay()
{
	Super::BeginPlay();
	
	GuardianState = EGuardianState::CtrlSkillEnd;

	UGS_DrakharAnimInstance* Anim = Cast<UGS_DrakharAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(Anim))
	{
		Anim->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::OnMontageNotifyBegin);
	}
}

void AGS_Drakhar::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanCombo);
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

	if (!HasAuthority() && IsLocallyControlled())
	{
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready))
		{
			//earthquake
			GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}

		//not flying
		else if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			if (bClientCanCombo)
			{
				PlayComboAttackMontage();
				ServerRPCNewComboAttack();
				bClientCanCombo = false;
			}
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

void AGS_Drakhar::SetNextComboAttackSection(FName InSectionName)
{
	ComboAttackSectionName = InSectionName;
}

void AGS_Drakhar::ResetComboAttackSection()
{
	ComboAttackSectionName = DefaultComboAttackSectionName;
}

void AGS_Drakhar::PlayComboAttackMontage()
{
	PlayAnimMontage(ComboAttackMontage,1.f, ComboAttackSectionName);
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s  %s"),*ComboAttackMontage.GetName(), *ComboAttackSectionName.ToString()), true, true, FLinearColor::Blue, 5.f);
}

void AGS_Drakhar::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& PayLoad)
{
	if (NotifyName == "None")
	{
		bClientCanCombo = true;
		ServerRPCResetValue();
	}
}

void AGS_Drakhar::OnRep_CanCombo()
{
	bClientCanCombo = bCanCombo;
}

void AGS_Drakhar::ServerRPCResetValue_Implementation()
{
	bCanCombo = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AGS_Drakhar::ServerRPCNewComboAttack_Implementation()
{
	MulticastRPCComboAttack();	
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	bCanCombo = false;
}

void AGS_Drakhar::MulticastRPCComboAttack_Implementation()
{
	if (!IsLocallyControlled())
	{
		PlayComboAttackMontage();
	}
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
	GuardianState = EGuardianState::CtrlSkillEnd;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	bCanCombo = true;
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
	const FVector End = Start + GetActorForwardVector() * 10.f;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeCapsule(100.f, 100.f), Params);
	
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
	//MulticastRPCDrawDebugLine(Start,End, 100.f, 100.f, GetActorForwardVector(), bIsHitDetected);
}

void AGS_Drakhar::ServerRPCEarthquakeAttackCheck_Implementation()
{
	TSet<AGS_Character*> EarthquakeDamagedCharacters;
	TArray<FHitResult> OutHitResults;
	const FVector Start = GetActorLocation() + 200.f;
	const FVector End = Start + GetActorForwardVector() * 100.f;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(EarthquakeRadius), Params);

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