// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanUltimateSkill.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Skill/GS_SkillSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Engine/StaticMeshActor.h"
#include "Character/Debuff/EDebuffType.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/GS_TpsController.h"
#include "AIController.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Sound/GS_SeekerAudioComponent.h"



UGS_ChanUltimateSkill::UGS_ChanUltimateSkill()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}

void UGS_ChanUltimateSkill::ActiveSkill()
{
	Super::ActiveSkill();

	// 쿨타임 측정 시작
	StartCoolDown();
	
	// 구조물 충돌 확인 변수 초기화
	bInStructureCrash = false;
	
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// 궁극기 사운드 재생
		if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
		}
		
		// 입력 제한 설정
		//OwnerPlayer->SetSkillInputControl(false, false, false);
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
		OwnerPlayer->SetMoveControlValue(false, false);
	}

	// 돌진 시작 (약간 딜레이)
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, this, &UGS_ChanUltimateSkill::StartCharge, 0.5f, false);
}

void UGS_ChanUltimateSkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();
}

void UGS_ChanUltimateSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	
	OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
	OwnerPlayer->SetMoveControlValue(true, true);
	OwnerPlayer->CanChangeSeekerGait = true;

	// 스킬 상태 업데이트
	SetIsActive(false);
}

void UGS_ChanUltimateSkill::InterruptSkill()
{
	Super::InterruptSkill();
}

void UGS_ChanUltimateSkill::HandleUltimateCollision(AActor* HitActor, UPrimitiveComponent* HitComp)
{
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	
	// 데이터 테이블에서 스킬 정보 가져오기
	const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
	
	if (AGS_Guardian* Guardian = Cast<AGS_Guardian>(HitActor)) // 가디언일 경우
	{
		ApplyEffectToGuardian(Guardian);
		
		// 가디언 충돌 사운드 재생
		if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->FindComponentByClass<UGS_SeekerAudioComponent>())
		{
			AudioComp->PlaySkillCollisionSoundFromDataTable(ESkillSlot::Ultimate, 2); // 2 = 가디언 충돌
		}
		
		EndCharge();
	}
	else if (AGS_Monster* Monster = Cast<AGS_Monster>(HitActor)) // 몬스터일 경우
	{
		if (!HitActors.Contains(Monster))
		{
			HitActors.Add(Monster);
			ApplyEffectToDungeonMonster(Monster);
			
			// 몬스터 충돌 사운드 재생
			if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->FindComponentByClass<UGS_SeekerAudioComponent>())
			{
				AudioComp->PlaySkillCollisionSoundFromDataTable(ESkillSlot::Ultimate, 1); // 1 = 몬스터 충돌
			}
		}
	}

	// 벽 충돌 체크
	if (HitComp && HitComp->ComponentHasTag("Wall"))
	{
		bInStructureCrash = true;
		
		// 벽 충돌 사운드 재생
		if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->FindComponentByClass<UGS_SeekerAudioComponent>())
		{
			AudioComp->PlaySkillCollisionSoundFromDataTable(ESkillSlot::Ultimate, 0); // 0 = 벽 충돌
		}
		
		// 대시 종료
		EndCharge();
	}
}

void UGS_ChanUltimateSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	if (!Target)
	{
		return;
	}

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if (!OwnerPlayer) 
	{
		return;
	}

	// 경직 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Stun, OwnerCharacter);
	}

	// 현재 프레임의 실시간 방향 계산
	FVector CurrentForward = OwnerPlayer->GetActorForwardVector().GetSafeNormal(); // 실시간
	FVector PlayerToMonster = Target->GetActorLocation() - OwnerPlayer->GetActorLocation();
	FVector RightVector = FVector::CrossProduct(CurrentForward, FVector::UpVector).GetSafeNormal();

	// Dot 비교로 방향 판별
	float DotProduct = FVector::DotProduct(PlayerToMonster, RightVector);
	FVector KnockbackDirection = (DotProduct > 0) ? RightVector : -RightVector;

	FVector KnockbackVelocity = KnockbackDirection * KnockbackForce;
	KnockbackVelocity.Z = 300.0f;

	// 몬스터 넉백 적용
	if (Target->GetCharacterMovement())
	{
		Target->LaunchCharacter(KnockbackVelocity, true, true);
	}

	//UGameplayStatics::ApplyDamage(Target, Damage, OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());
}

void UGS_ChanUltimateSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	if (!Target) return;
    
    AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
    if (!OwnerPlayer) return;
    
    // 가디언용 넉백 (더 약한 힘)
    FVector KnockbackDirection = (Target->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();
    FVector KnockbackVelocity = KnockbackDirection * GuardianKnockbackForce;
    
    if (Target->GetCharacterMovement())
    {
        Target->LaunchCharacter(KnockbackVelocity, true, false);
    }
}

void UGS_ChanUltimateSkill::DeactiveSkill()
{
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if (OwnerPlayer)
	{
		// 넉백 Collision 설정
		OwnerPlayer->UltimateCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// 속도 조절
	OwnerCharacter->Server_SetCharacterSpeed(1.0f);

	// 자동 이동 종료
	AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->StopAutoMoveForward();

	// 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);

	// 충돌 이력 초기화
	HitActors.Empty();

	// SeekerAudioComponent를 통한 스킬 종료 사운드
	if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, false);
		}
	}

	Super::DeactiveSkill();
}

void UGS_ChanUltimateSkill::StartCharge()
{	
	// 넉백 Collision 켜기
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	OwnerPlayer->UltimateCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	GetWorld()->GetTimerManager().SetTimer(
		ChargeTimerHandle,
		this,
		&UGS_ChanUltimateSkill::EndCharge,
		1.5f, 
		false
	);

	// 속도 조절
	OwnerCharacter->Server_SetCharacterSpeed(3.0f);

	// 자동 이동 시작
	AGS_TpsController* Controller = Cast<AGS_TpsController>(OwnerCharacter->GetController());
	Controller->SetMoveControlValue(true, true);
	Controller->StartAutoMoveForward();

	if (OwnerPlayer)
	{
		// 애니메이션 설정
		OwnerPlayer->Multicast_SetIsFullBodySlot(true);
		OwnerPlayer->Multicast_SetIsUpperBodySlot(false);

		// 스킬 애니메이션 재생
		if (SkillAnimMontages[0])
		{
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
		}

		// =======================
		// VFX 재생 - 컴포넌트 RPC 사용
		// =======================
		if (OwningComp)
		{
			FVector SkillLocation = OwnerCharacter->GetActorLocation();
			//FRotator SkillRotation = OwnerCharacter->GetActorRotation();
			FRotator SkillRotation = FRotator(0.f, 0.f, 0.f);

			// 스킬 시전 VFX 재생
			OwningComp->Multicast_PlayCastVFX(CurrentSkillType, SkillLocation, SkillRotation);
		}

	}
}

void UGS_ChanUltimateSkill::EndCharge()
{
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	if(bInStructureCrash) // 구조물에 부딪혔을 때
	{
		if (OwnerPlayer && SkillAnimMontages[2])
		{
			// 애니메이션 재생
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[2]);
		}
	}
	else // 구조물이 아닌 곳에 부딪혔을 때
	{
		if (OwnerPlayer && SkillAnimMontages[1])
		{
			// 애니메이션 재생
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[1]);
		}
	}

	DeactiveSkill();
}
