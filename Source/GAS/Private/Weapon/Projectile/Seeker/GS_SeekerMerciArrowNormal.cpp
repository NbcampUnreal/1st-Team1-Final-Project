// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Skill/GS_IgnoreDefenceDamageType.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "GameFramework/DamageType.h"
#include "Components/SphereComponent.h"

AGS_SeekerMerciArrowNormal::AGS_SeekerMerciArrowNormal()
{

}

void AGS_SeekerMerciArrowNormal::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGS_SeekerMerciArrowNormal::ChangeArrowType(EArrowType Type)
{
	ArrowType = Type;
}

void AGS_SeekerMerciArrowNormal::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// 맞은 대상 구분
	ETargetType TargetType = DetermineTargetType(OtherActor);
	
	// 추가 세부 처리
	float DamageToApply = BaseDamage;
	TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

	switch (ArrowType)
	{
	case EArrowType::Normal:
		if (TargetType == ETargetType::Guardian)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Overlap - Normal - Guardian"));
			DamageToApply *= 0.5f;
			// 화살이 박히지 않음 → 바로 Destroy
			Destroy();
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Overlap - Normal - Dungeon Monster"));
			StickWithVisualOnly(SweepResult);
		}
		break;

	case EArrowType::Axe:
		//UE_LOG(LogTemp, Warning, TEXT("Overlap - Axe - Guardian, DungeonMonster"));
		DamageTypeClass = UGS_IgnoreDefenceDamageType::StaticClass();
		StickWithVisualOnly(SweepResult);
		break;

	case EArrowType::Child:
		if (TargetType == ETargetType::Guardian)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Overlap - Child - Guardian"));
			StickWithVisualOnly(SweepResult);
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			// 일반 몬스터에게는 관통 → 시각 효과 없이 그냥 지나감
			//UE_LOG(LogTemp, Warning, TEXT("Overlap - Child - Dungeon Monster"));
		}
		
		break;
	}

	if (TargetType == ETargetType::Structure)
	{
		StickWithVisualOnly(SweepResult); // 화살 박힘
	}
	else if (TargetType == ETargetType::Seeker)
	{

	}
	else if (TargetType == ETargetType::DungeonMonster || TargetType == ETargetType::Guardian)
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, DamageToApply, GetActorForwardVector(), SweepResult, GetInstigatorController(), this, DamageTypeClass);
	}
}

