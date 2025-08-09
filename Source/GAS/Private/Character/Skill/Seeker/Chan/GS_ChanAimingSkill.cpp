// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Sound/GS_CharacterAudioComponent.h"
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

UGS_ChanAimingSkill::UGS_ChanAimingSkill()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_ChanAimingSkill::ActiveSkill()
{
	// 스킬 상태 업데이트
	Super::ActiveSkill();
	
	// 쿨타임 측정 시작
	StartCoolDown();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// Change Slot
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::UpperBody);
		
		OwnerPlayer->Multicast_SetMustTurnInPlace(true);
		OwnerPlayer->SetSeekerGait(EGait::Walk);
		OwnerPlayer->CanChangeSeekerGait = false;

		// Play Montage
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
		OwnerPlayer->CanChangeSeekerGait = false;

		// 스킬 시작 사운드 재생
		if (UGS_CharacterAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_CharacterAudioComponent>())
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
		}

		// =======================
		// VFX 재생 - 컴포넌트 RPC 사용
		// =======================
		if (OwningComp)
		{
			FVector SkillLocation = OwnerCharacter->GetActorLocation();
			FRotator SkillRotation = OwnerCharacter->GetActorRotation();
	
			// 스킬 시전 VFX 재생
			OwningComp->Multicast_PlayCastVFX(CurrentSkillType, SkillLocation, SkillRotation);
		}

		// 방패 들기
		StartHoldUp();
	}
}

void UGS_ChanAimingSkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();
}

void UGS_ChanAimingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// Change Slot
		OwnerPlayer->Multicast_SetMustTurnInPlace(false);
		OwnerPlayer->SetSeekerGait(OwnerPlayer->GetLastSeekerGait());
		// Change Slot
		/*OwnerPlayer->Multicast_SetIsFullBodySlot(false);
		OwnerPlayer->Multicast_SetIsUpperBodySlot(false);*/
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);

		OwnerPlayer->CanChangeSeekerGait = true;
		
		OwnerPlayer->SetMoveControlValue(true, true);
		OwnerPlayer->SetLookControlValue(true, true);

		SetIsActive(false); // 이걸 할 필요가 있나?

		// =======================
		// 스킬 종료 VFX 재생
		// =======================
		
		if (OwningComp)
		{
			FVector SkillLocation = OwnerCharacter->GetActorLocation();
			FRotator SkillRotation = OwnerCharacter->GetActorRotation();

			// 스킬 종료 VFX 재생
			OwningComp->Multicast_PlayEndVFX(CurrentSkillType, SkillLocation, SkillRotation);
		}
	}
}

void UGS_ChanAimingSkill::OnSkillCommand()
{
	Super::OnSkillCommand();
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{		
		// 애니메이션 설정
		OwnerPlayer->Multicast_SetMustTurnInPlace(false);
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);

		// 입력 제한 설정
		OwnerPlayer->SetLookControlValue(false, false);
		OwnerPlayer->SetMoveControlValue(false, false);
		//OwnerPlayer->SetSkillInputControl(false, false, false);
		
		// Play Montage
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[1]);
		
		// Forward Jump
		const FVector Forward = OwnerPlayer->GetActorForwardVector();
		const FVector JumpVelocity = Forward * 600.0f + FVector(0.f, 0.f, 420.0f);
		OwnerPlayer->LaunchCharacter(JumpVelocity, true, true);

		// 방패 슬램 사운드 재생
		if (UGS_CharacterAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_CharacterAudioComponent>())
		{
			AudioComp->PlaySkillCollisionSoundFromDataTable(CurrentSkillType, 0); // 0 = Wall
		}

		// =======================
		// 스킬 범위 VFX 재생
		// =======================
		OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(
		RangeVFXSpawnHandle,
		FTimerDelegate::CreateUObject(this, &UGS_ChanAimingSkill::SpawnAimingSkillVFX),
		0.93f,
		false);

		// 내려치기
		OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(KnockbackHandle, this, &UGS_ChanAimingSkill::OnShieldSlam, 0.8f, false);
	}
}

void UGS_ChanAimingSkill::SpawnAimingSkillVFX()
{
	if (OwningComp&& OwnerCharacter)
	{
		const FVector Start = OwnerCharacter->GetActorLocation();
		const FVector Forward = OwnerCharacter->GetActorForwardVector();
		FVector SkillLocation = Start + Forward * 150.0f;
		const float Radius = 200.f;

		// 스킬 범위 표시 VFX 재생
		OwningComp->Multicast_PlayRangeVFX(CurrentSkillType, SkillLocation, Radius);
	}
}

void UGS_ChanAimingSkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);

	OwnerPlayer->SetLookControlValue(true, true);
	SetIsActive(false);

	CurrentStamina = 0;
	ShowProgressBar(false);
	OwnerPlayer->GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
}


void UGS_ChanAimingSkill::OnShieldSlam()
{
	// 방패 충돌
	TArray<FHitResult> HitResults;

	const FVector Start = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	FVector SkillLocation = Start + Forward * 150.0f;
	const float Radius = 200.f;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	// 스킬 범위 표시(테스트)
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	OwnerPlayer->Multicast_DrawSkillRange(SkillLocation, Radius, FColor::Red, 1.0f);

	if (OwnerCharacter->GetWorld()->SweepMultiByChannel(HitResults, SkillLocation, SkillLocation, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		TSet<AActor*> HitActors;

		for (const FHitResult& Hit : HitResults)
		{
			// 충돌체
			AActor* HitActor = Hit.GetActor();
			UPrimitiveComponent* HitComponent = Hit.GetComponent();

			// 중복된 충돌 액터 걸러내기
			if (!HitActor || HitActors.Contains(HitActor))
			{
				continue;
			}

			// 사운드 트리거 무시
			if (HitComponent && HitComponent->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}

			// 충돌 액터 저장
			HitActors.Add(HitActor);

			// 충돌 효과 활성화
			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor)) // 몬스터일 경우
			{
				ApplyEffectToDungeonMonster(TargetMonster);
				// Impact VFX 재생
				TargetMonster->Multicast_PlayImpactVFX(SkillImpactVFX, SkillVFXScale);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor)) // 가디언일 경우
			{
				ApplyEffectToGuardian(TargetGuardian);
				// Impact VFX 재생
				TargetGuardian->Multicast_PlayImpactVFX(SkillImpactVFX, SkillVFXScale);
			}
			else if (AGS_Character* Target = Cast<AGS_Character>(HitActor)) // 시커일 경우
			{
				// 넉백
				const FVector LaunchDirection = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
				Target->LaunchCharacter(LaunchDirection * 1000.0f + FVector(0, 0, 500.0f), true, true);
			}
		}
	}

	// 스킬 종료
	DeactiveSkill();
}

void UGS_ChanAimingSkill::TickDrainStamina()
{
	CurrentStamina -= StaminaDrainRate;

	// UI 업데이트
	UpdateProgressBar(CurrentStamina);

	// UE_LOG(LogTemp, Warning, TEXT("Stamina : %f"), CurrentStamina); 신중은
	
	// 스테미나가 0이 되었을 때
	if (CurrentStamina <= 0.f)
	{
		// 기본 포즈로 애니메이션 재생
		AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
		if (OwnerPlayer)
		{
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0], FName("LoopEnd"));
		}

		// 스킬 종료
		DeactiveSkill();
	}
}

void UGS_ChanAimingSkill::StartHoldUp()
{
	// 스테미나 초기화
	CurrentStamina = MaxStamina;

	// UI 표시
	ShowProgressBar(true);
	UpdateProgressBar(CurrentStamina);

	// 스테미나 감소 타이머
	OwnerCharacter->GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &UGS_ChanAimingSkill::TickDrainStamina, 1.0f, true);
}

void UGS_ChanAimingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	if (!Target) 
	{
		return;
	}

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

void UGS_ChanAimingSkill::DeactiveSkill()
{
	// UI 숨기기
	ShowProgressBar(false);
	OwnerCharacter->GetWorldTimerManager().ClearTimer(StaminaDrainHandle);

	// 스킬 상태 업데이트
	Super::DeactiveSkill();
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
