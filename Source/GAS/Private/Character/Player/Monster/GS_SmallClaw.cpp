// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_SmallClaw.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"

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
	
	if (SmallClawClickSound)
	{
		ClickSoundEvent = SmallClawClickSound;
	}

	if (SmallClawMoveSound)
	{
		MoveSoundEvent = SmallClawMoveSound;
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
	
	if (!OtherActor || OtherActor == this)
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
		FDamageEvent DamageEvent;
		OtherActor->TakeDamage(Damage, DamageEvent, GetController(), this);
	
		BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
