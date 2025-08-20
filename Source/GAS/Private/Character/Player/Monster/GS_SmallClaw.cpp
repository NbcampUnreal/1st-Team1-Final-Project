// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_SmallClaw.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Character/F_GS_DamageEvent.h"

AGS_SmallClaw::AGS_SmallClaw()
{
	BiteCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BiteCollision"));
	BiteCollision->SetupAttachment(GetMesh(), TEXT("head"));
	BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteCollision->OnComponentBeginOverlap.AddDynamic(this, &AGS_SmallClaw::OnAttackBiteboxOverlap);
}

void AGS_SmallClaw::BeginPlay()
{
	Super::BeginPlay();
	
	// SmallClaw 전용 몬스터 오디오 설정 (컴포넌트 사용)
	if (MonsterAudioComponent)
	{
		MonsterAudioComponent->MonsterSoundVariant = 1; // SmallClaw = 1

		// 작은 몬스터 특성: 가까운 거리에서 경계, 짧은 최대 거리
		MonsterAudioComponent->AudioConfig.AlertDistance = 600.0f;
		MonsterAudioComponent->AudioConfig.MaxAudioDistance = 2000.0f;

		// 사운드 재생 간격 (작은 몬스터이므로 자주 울음)
		MonsterAudioComponent->IdleSoundInterval = 4.0f;
		MonsterAudioComponent->CombatSoundInterval = 2.5f;
	}
}

void AGS_SmallClaw::SetBiteCollision(bool bEnable)
{
	if (BiteCollision)
	{
		BiteCollision->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void AGS_SmallClaw::OnAttackBiteboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)

{
	if (!HasAuthority())
	{
		return;
	}
	
	AGS_Character* Damaged = Cast<AGS_Character>(OtherActor);
	AGS_Character* Attacker = this;
	if (!Damaged || !Attacker || !Damaged->IsEnemy(Attacker))
	{
		return;
	}

	if (AGS_Character* DamagedCharacter = Cast<AGS_Character>(OtherActor))
	{
		if (!DamagedCharacter->IsEnemy(Cast<AGS_Character>(this)))
		{
			return; 
		}
		
		float Damage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter);
		FGS_DamageEvent DamageEvent;
		DamageEvent.HitReactType = EHitReactType::Interrupt;
		OtherActor->TakeDamage(Damage, DamageEvent, GetController(), this);
	
		BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
