// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Debuff/EDebuffType.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/GS_Player.h"
#include "AkAudioEvent.h"
#include "AI/GS_AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UGS_ChanAimingSkill::UGS_ChanAimingSkill()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_ChanAimingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (OwnerPlayer->HasAuthority())
		{
			OwnerPlayer->GetMesh()->GetAnimInstance()->StopAllMontages(0);
			// Combo Control Value
			OwnerPlayer->ComboInputClose();
			OwnerPlayer->CurrentComboIndex = 0;
			
			// Control Input Key
			OwnerPlayer->SetSkillInputControl(false, false);
			OwnerPlayer->SetMoveControlValue(false, false);
			// Change Slot
			OwnerPlayer->Multicast_SetIsFullBodySlot(true);
			OwnerPlayer->Multicast_SetIsUpperBodySlot(false);

			// Play Montage
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
		}
		
		// 에이밍 스킬 시작 사운드 재생
		if (OwnerPlayer->AimingSkillStartSound)
		{
			OwnerPlayer->Multicast_PlaySkillSound(OwnerPlayer->AimingSkillStartSound);
		}

		// Skill Stat
		CurrentStamina = MaxStamina;
		// UI
		ShowProgressBar(true);
		UpdateProgressBar(CurrentStamina);
		OwnerCharacter->GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &UGS_ChanAimingSkill::TickDrainStamina, 1.0f, true);
		// Skill State
		if (OwnerCharacter->GetSkillComp())
		{
			OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, true);
		}
	}
}

void UGS_ChanAimingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (OwnerPlayer->HasAuthority())
		{
			OwnerPlayer->ComboInputOpen();
			OwnerPlayer->Multicast_SetIsFullBodySlot(false);
			OwnerPlayer->Multicast_SetIsUpperBodySlot(false);
			// Control Input Key
			OwnerPlayer->SetSkillInputControl(true, true);
			OwnerPlayer->SetMoveControlValue(true, true);
			// Change Slot
			OwnerPlayer->Multicast_SetMustTurnInPlace(false);
			OwnerPlayer->SetLookControlValue(true, true);
			OwnerPlayer->SetSkillInputControl(true, true);

			if (OwnerCharacter->GetSkillComp())
			{
				OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, false);
			}
		}
	}
}

void UGS_ChanAimingSkill::OnSkillCommand()
{
	Super::OnSkillCommand();
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (OwnerPlayer->HasAuthority())
		{
			OwnerPlayer->Multicast_StopSkillMontage(SkillAnimMontages[0]);
			OwnerPlayer->Multicast_SetMustTurnInPlace(false);
		
			// Change Slot
			OwnerPlayer->Multicast_SetIsUpperBodySlot(false);
			OwnerPlayer->Multicast_SetIsFullBodySlot(true);

			// Play Montage
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[1]);

			// Control Input Value
			OwnerPlayer->SetLookControlValue(false, false);
			OwnerPlayer->SetMoveControlValue(false, false);
			OwnerPlayer->SetSkillInputControl(false, false);
		}
		
	
		// 방패 슬램 사운드 재생
		if (OwnerPlayer->AimingSkillSlamSound)
		{
			OwnerPlayer->Multicast_PlaySkillSound(OwnerPlayer->AimingSkillSlamSound);
		}

		// End Skill
		ExecuteSkillEffect();
		if (OwnerCharacter->GetSkillComp())
		{
			OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, false);
		}
		ShowProgressBar(false);
		OwnerCharacter->GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
	}
}

void UGS_ChanAimingSkill::ExecuteSkillEffect()
{
	TArray<FHitResult> HitResults;

	const FVector Start = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	FVector SkillLocation = Start + Forward * 150.0f;
	const float Radius = 200.f;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	OwnerPlayer->Multicast_DrawSkillRange(SkillLocation, Radius, FColor::Red, 1.0f);
	if (OwnerCharacter->GetWorld()->SweepMultiByChannel(HitResults, SkillLocation, SkillLocation, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		TSet<AActor*> HitActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			UPrimitiveComponent* HitComponent = Hit.GetComponent();

			if (!HitActor || HitActors.Contains(HitActor))
			{
				continue;
			}

			if (HitComponent && HitComponent->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}

			HitActors.Add(HitActor);

			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s, Hit Component: %s"),
				*HitActor->GetName(),
				*HitComponent->GetName());

			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor))
			{
				ApplyEffectToDungeonMonster(TargetMonster);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor))
			{
				ApplyEffectToGuardian(TargetGuardian);
			}
			else if(AGS_Character* Target = Cast<AGS_Character>(HitActor))
			{
				// 넉백
				const FVector LaunchDirection = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				Target->LaunchCharacter(LaunchDirection * 1000.0f + FVector(0, 0, 500.0f), true, true);
			}
		}
	}
}

bool UGS_ChanAimingSkill::IsActive() const
{
	return bIsHoldingUp;
}

void UGS_ChanAimingSkill::OnShieldSlam()
{
	//UE_LOG(LogTemp, Warning, TEXT("Slam!!!!!!!"));
	CurrentStamina -= SlamStaminaCost;
	// UI 업데이트
	UpdateProgressBar(CurrentStamina);
	ExecuteSkillEffect();

	if (CurrentStamina <= 0.f)
	{
		EndHoldUp();
	}
}

void UGS_ChanAimingSkill::TickDrainStamina()
{
	CurrentStamina -= StaminaDrainRate;

	// UI 업데이트
	UpdateProgressBar(CurrentStamina);

	UE_LOG(LogTemp, Warning, TEXT("Stamina : %f"), CurrentStamina);
	if (CurrentStamina <= 0.f)
	{
		EndHoldUp();
	}
}

void UGS_ChanAimingSkill::StartHoldUp()
{
	bIsHoldingUp = true;
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, true);
	}
	CurrentStamina = MaxStamina;

	OwnerCharacter->Server_SetCharacterSpeed(0.3f);
	// UI 표시
	ShowProgressBar(true);
	UpdateProgressBar(CurrentStamina);

	UE_LOG(LogTemp, Warning, TEXT("Start Hold Up!!!!!!!"));
	OwnerCharacter->GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &UGS_ChanAimingSkill::TickDrainStamina, 1.0f, true);
}

void UGS_ChanAimingSkill::EndHoldUp()
{
	bIsHoldingUp = false;
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, false);
		if (AGS_Chan* Chan = Cast<AGS_Chan>(OwnerCharacter))
		{
			Chan->ToIdle();
		}
	}

	OwnerCharacter->Server_SetCharacterSpeed(1.0f);

	// UI 숨기기
	ShowProgressBar(false);
	OwnerCharacter->GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
}

void UGS_ChanAimingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	if (!Target) return;

	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Stun, OwnerCharacter);
	}

	// 넉백
	const FVector LaunchDirection = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
	Target->LaunchCharacter(LaunchDirection * 1000.0f + FVector(0, 0, 500.0f), true, true);

	// 데미지
	UGameplayStatics::ApplyDamage(Target, Damage, OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());
}

void UGS_ChanAimingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 데미지
	UGameplayStatics::ApplyDamage(Target, Damage, OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());

	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Stun, OwnerCharacter);
	}
}

void UGS_ChanAimingSkill::UpdateProgressBar(float InStamina)
{
	AGS_Chan* OwnerChan = Cast<AGS_Chan>(OwnerCharacter);
	OwnerChan->Client_UpdateChanAimingSkillBar(InStamina / MaxStamina);
}

void UGS_ChanAimingSkill::ShowProgressBar(bool bShow)
{
	AGS_Chan* OwnerChan = Cast<AGS_Chan>(OwnerCharacter);
	OwnerChan->Client_ChanAimingSkillBar(bShow);
}


float UGS_ChanAimingSkill::GetCurrentStamina()
{
	return CurrentStamina;
}
