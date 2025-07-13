// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_SmokeFieldSkill.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Debuff/EDebuffType.h"
#include "Components/SphereComponent.h"

AGS_SmokeFieldSkill::AGS_SmokeFieldSkill()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGS_SmokeFieldSkill::BeginPlay()
{
	Super::BeginPlay();

	FHitResult Hit;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 10000); // 아래로 충분히 긴 거리

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// 레벨에 따른 연기 화살 하강 정도
	FString LevelName = GetWorld()->GetMapName();
	LevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix); // 맵 이름에서 "UEDPIE_0_" 같은 접두어 제거

	if (LevelName == TEXT("InGameTestLevel"))
	{
		GroundZ = 100.0f; // 특정 맵일 때만 다르게
	}

	TargetGroundLocation = FVector(GetActorLocation().X, GetActorLocation().Y, GroundZ);
	bShouldDescendToGround = true;
}

void AGS_SmokeFieldSkill::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldDescendToGround)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetGroundLocation, DeltaTime, DescendSpeed); // 부드럽게 내려오기

		SetActorLocation(NewLocation);

		float Dist = FVector::Dist(NewLocation, TargetGroundLocation);
		if (Dist < 5.0f) // 거의 다 내려왔으면 정지
		{
			SetActorLocation(TargetGroundLocation);
			bShouldDescendToGround = false;
		}
	}
}

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
