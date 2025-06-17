// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanMovingSkill.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Debuff/EDebuffType.h"
#include "Character/Player/GS_Player.h"
#include "AkAudioEvent.h"

UGS_ChanMovingSkill::UGS_ChanMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_ChanMovingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (OwnerPlayer->HasAuthority())
		{
			OwnerPlayer->GetMesh()->GetAnimInstance()->StopAllMontages(0);
			//OwnerPlayer->CanAcceptComboInput = false;
			OwnerPlayer->ComboInputClose();
			OwnerPlayer->CurrentComboIndex = 0;
			OwnerPlayer->Multicast_SetIsFullBodySlot(true);
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
			OwnerPlayer->SetMoveControlValue(false, false);
			OwnerPlayer->SetSkillInputControl(false, false);

		}
		// 무빙 스킬 사운드 재생
		if (OwnerPlayer->MovingSkillSound)
		{
			OwnerPlayer->Multicast_PlaySkillSound(OwnerPlayer->MovingSkillSound);
		}
	}
	
	ExecuteSkillEffect();
}

void UGS_ChanMovingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);

	if (OwnerPlayer->HasAuthority())
	{
		OwnerPlayer->ComboInputOpen();
		OwnerPlayer->Multicast_SetIsFullBodySlot(false);
		OwnerPlayer->SetMoveControlValue(true, true);
		OwnerPlayer->Multicast_StopSkillMontage(SkillAnimMontages[0]);
		OwnerPlayer->SetSkillInputControl(true, true);
		
		OwnerPlayer->CanChangeSeekerGait = true;
	}
}

void UGS_ChanMovingSkill::ExecuteSkillEffect()
{
	TArray<FHitResult> HitResults;

	const FVector Center = OwnerCharacter->GetActorLocation(); // 중심은 캐릭터
	const float Radius = 800.0f;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	OwnerPlayer->Multicast_DrawSkillRange(Center, Radius, FColor::Red, 2.0f);
	// 캐릭터를 중심으로 한 지점에 고정된 SphereOverlap
	if (GetWorld()->SweepMultiByChannel(HitResults, Center, Center, FQuat::Identity, ECC_Pawn, Shape, Params))
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

			// SoundTrigger 콜리전 프로파일을 가진 컴포넌트 제외
			if (HitComponent && HitComponent->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}

			HitActors.Add(HitActor);

			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor))
			{
				
				ApplyEffectToDungeonMonster(TargetMonster);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor))
			{
				ApplyEffectToGuardian(TargetGuardian);
			}
		}
	}
}

void UGS_ChanMovingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	// 어그로 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Aggro, OwnerCharacter);
	}
}

void UGS_ChanMovingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 뮤트 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Mute, OwnerCharacter);
	}
}