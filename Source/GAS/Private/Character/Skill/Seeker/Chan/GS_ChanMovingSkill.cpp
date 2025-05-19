// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanMovingSkill.h"
#include "Character/GS_Character.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Debuff/EDebuffType.h"

void UGS_ChanMovingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	ExecuteSkillEffect();
}

void UGS_ChanMovingSkill::ExecuteSkillEffect()
{
	TArray<FHitResult> HitResults;

	const FVector Center = OwnerCharacter->GetActorLocation(); // 중심은 캐릭터
	const float Radius = 4000.f;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	// 디버그: 캐릭터를 중심으로 원을 그림
	DrawDebugSphere(
		GetWorld(),
		Center,
		Radius,
		16,
		FColor::Red,
		false,
		1.f
	);

	// 캐릭터를 중심으로 한 지점에 고정된 SphereOverlap
	if (GetWorld()->SweepMultiByChannel(HitResults, Center, Center, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor) continue;

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
}
