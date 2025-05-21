// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_SmokeFieldSkill.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Debuff/EDebuffType.h"
#include "Components/SphereComponent.h"

void AGS_SmokeFieldSkill::ApplyFieldEffectToMonster(AGS_Monster* Target)
{
	if (!HasAuthority() || !Target) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Client or Target is null - ApplyFieldEffectToMonster"));
		return;
	}

	// DebuffConfuse 적용 (AI 타겟 상실 및 RTS 선택 불가)
	if (UGS_DebuffComp* DebuffComp = Target->GetDebuffComp())
	{
		DebuffComp->ApplyDebuff(EDebuffType::Confuse, Caster);
	}
}

void AGS_SmokeFieldSkill::RemoveFieldEffectFromMonster(AGS_Monster* Target)
{
	if (!HasAuthority() || !IsValid(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client or Target is null - RemoveFieldEffectFromMonster"));
		return;
	}

	if (UGS_DebuffComp* DebuffComp = Target->GetDebuffComp())
	{
		DebuffComp->RemoveDebuff(EDebuffType::Confuse);
	}
}

void AGS_SmokeFieldSkill::ApplyFieldEffectToGuardian(AGS_Guardian* Target)
{
	if (!HasAuthority() || !IsValid(Target)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Client or Target is null - ApplyFieldEffectToGuardian"));
		return;
	}

	// 가디언의 "머리" 본 위치가 장판 안에 있을 때만 효과 적용
	FVector HeadLocation = Target->GetMesh()->GetBoneLocation(TEXT("head"));
	float DistToCenter = FVector::Dist(HeadLocation, GetActorLocation());
	UE_LOG(LogTemp, Warning, TEXT("Dist To Centher : %f, Scaled Sphere : %f"), DistToCenter, SphereComp->GetScaledSphereRadius());
	if (DistToCenter-200.0f <= SphereComp->GetScaledSphereRadius())
	{
		if (UGS_DebuffComp* DebuffComp = Target->GetDebuffComp())
		{
			DebuffComp->ApplyDebuff(EDebuffType::Obscure, Caster);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Is Not in Head"));
	}
}

void AGS_SmokeFieldSkill::RemoveFieldEffectFromGuardian(AGS_Guardian* Target)
{
}
