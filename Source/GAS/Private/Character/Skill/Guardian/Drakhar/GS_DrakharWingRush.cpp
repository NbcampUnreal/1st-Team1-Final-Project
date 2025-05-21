#include "Character/Skill/Guardian/Drakhar/GS_DrakharWingRush.h"

#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Templates/SharedPointer.h"

UGS_DrakharWingRush::UGS_DrakharWingRush()
{
	Cooltime = 8.f;

	DashRemainTime = 1.2f; //not yet
	DashPower = 1000.f;
	//DashInterpAlpha = 0.f;
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> WingRushMontage(TEXT("/Game/Player/Guardian/Drakhar/Animations/Blueprint/AM_Dash.AM_Dash"));
	if (WingRushMontage.Succeeded())
	{
		SkillAnimMontages.Add(WingRushMontage.Object);
	}
}

void UGS_DrakharWingRush::ActiveSkill()
{
	Super::ActiveSkill();

	ExecuteSkillEffect();
	
	DashStartLocation = OwnerCharacter->GetActorLocation();
	DashEndLocation = DashStartLocation + OwnerCharacter->GetActorForwardVector() * DashPower;
}

void UGS_DrakharWingRush::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();

	OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);

	
	//OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	//DoDash();
	//const FVector CharacterForwardVector = OwnerCharacter->GetActorForwardVector();
	//OwnerCharacter->LaunchCharacter(CharacterForwardVector * 2000.f,false,false);
	//OwnerCharacter->GetWorldTimerManager().SetTimer(DashTimer,this, &UGS_DrakharWingRush::DashTimerEnd, DashRemainTime, false);
}

// void UGS_DrakharWingRush::ServerRPCDoDash_Implementation(float DeltaTime)
// {
// 	DashInterpAlpha += DeltaTime / DashRemainTime;
//
// 	DashAttackCheck();
// 	if (DashInterpAlpha >= 1.f)
// 	{
// 		OwnerCharacter->SetActorLocation(DashEndLocation);
// 		//bIsDashing = false;
// 		//EndDash();
// 	}
// 	else
// 	{
// 		const FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, DashInterpAlpha);
// 		OwnerCharacter->SetActorLocation(NewLocation);
// 		DashStartLocation = NewLocation;
// 	}
// }

// void UGS_DrakharWingRush::ServerRPCEndDash_Implementation()
// {
// 	if (DamagedCharacters.IsEmpty())
// 	{
// 		return;
// 	}
// 	
// 	for (auto const& DamagedCharacter : DamagedCharacters)
// 	{
// 		float Damage = DamagedCharacter->GetStatComp()->CalculateDamage(OwnerCharacter, DamagedCharacter);
// 		FDamageEvent DamageEvent;
// 		DamagedCharacter->TakeDamage(Damage, DamageEvent, OwnerCharacter->GetController(), OwnerCharacter);
// 	}
//
// 	DamagedCharacters.Empty();
// }

// void UGS_DrakharWingRush::DashTimerEnd()
// {
// 	OwnerCharacter->MulicastRPCStopCurrentSkillMontage(SkillAnimMontages[0]);
// 	OwnerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
// }
//
// int32 UGS_DrakharWingRush::DashMoveSetting( )
// {
// 	TSharedPtr<FRootMotionSource_MoveToForce> MoveSource = MakeShared<FRootMotionSource_MoveToForce>();
// 	MoveSource->InstanceName        = TEXT("PreciseMoveToForce");
// 	MoveSource->AccumulateMode      = ERootMotionAccumulateMode::Override; // 기존 속도 차단
// 	MoveSource->Priority            = 7;                                   // 걷기/달리기보다 높게
// 	MoveSource->Duration            = DashRemainTime;
// 	MoveSource->StartLocation       = DashStartLocation;
// 	MoveSource->TargetLocation      = DashEndLocation;
// 	MoveSource->bRestrictSpeedToExpected = true;
//
// 	DashAttackCheck();
// 	return OwnerCharacter->GetCharacterMovement()->ApplyRootMotionSource(MoveSource);
// }
//
// void UGS_DrakharWingRush::DoDash()
// {
// 	Activeid = DashMoveSetting();
// }

// void UGS_DrakharWingRush::DashAttackCheck()
// {
// 	OwnerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//
// 	TArray<FHitResult> OutHitResults;	
// 	const FVector Start = OwnerCharacter->GetActorLocation();
// 	const FVector End = Start + OwnerCharacter->GetActorForwardVector() * 100.f;
// 	FCollisionQueryParams Params(NAME_None, false, OwnerCharacter);
//
// 	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeCapsule(100.f, 200.f), Params);
//
// 	if (bIsHitDetected)
// 	{
// 		for (auto const& OutHitResult : OutHitResults)
// 		{
// 			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
// 			if (IsValid(DamagedCharacter))
// 			{
// 				DamagedCharacters.Add(DamagedCharacter);
//
// 				DrawDebugPoint(
// 					GetWorld(),
// 					OutHitResult.ImpactPoint,
// 					15.f,
// 					FColor::Yellow,
// 					false,
// 					1.f
// 				);
// 			}
// 		}		
// 	}
// }