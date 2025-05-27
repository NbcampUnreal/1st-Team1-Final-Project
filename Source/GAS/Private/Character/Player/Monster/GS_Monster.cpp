// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_Monster.h"
#include "AI/GS_AIController.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AkComponent.h"
#include "Animation/Character/GS_MonsterAnimInstance.h"
#include "Net/UnrealNetwork.h"

AGS_Monster::AGS_Monster()
{
	AIControllerClass = AGS_AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	SelectionDecal = CreateDefaultSubobject<UDecalComponent>("SelectionDecal");
	SelectionDecal->SetupAttachment(RootComponent);
	SelectionDecal->SetVisibility(false);

	AkComponent = CreateDefaultSubobject<UAkComponent>("AkComponent");
	AkComponent->SetupAttachment(RootComponent);
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->bUseRVOAvoidance = true;
		MovementComponent->AvoidanceConsiderationRadius = AvoidanceRadius;
		MovementComponent->AvoidanceWeight = 0.5f;
	}

	TeamId = FGenericTeamId(2);

	Tags.Add("Monster");
}

void AGS_Monster::BeginPlay()
{
	Super::BeginPlay();
}

void AGS_Monster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MonsterAnim = Cast<UGS_MonsterAnimInstance>(GetMesh()->GetAnimInstance());
}

void AGS_Monster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_Monster, bCommandLocked);
	DOREPLIFETIME(AGS_Monster, bSelectionLocked);
}

void AGS_Monster::OnDeath()
{
	Super::OnDeath();
	
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
	
	DetachFromControllerPendingDestroy();
	SetLifeSpan(4.f);
	
	Multicast_OnDeath();
}

void AGS_Monster::Multicast_OnDeath_Implementation()
{
	OnMonsterDead.Broadcast(this);
}

//void AGS_Monster::SetSelected(bool bIsSelected)
//{
//	if (SelectionDecal)
//	{
//		SelectionDecal->SetVisibility(bIsSelected);
//	}
//
//	if (bIsSelected && ClickSoundEvent)
//	{
//		UAkGameplayStatics::PostEvent(ClickSoundEvent, this, 0, FOnAkPostEventCallback());
//	}
//}

void AGS_Monster::SetSelected(bool bIsSelected, bool bPlaySound)
{
	if (SelectionDecal)
	{
		SelectionDecal->SetVisibility(bIsSelected);
	}

	// 선택되었고, 소리 재생이 허용된 경우에만 소리 재생
	if (bIsSelected && bPlaySound && ClickSoundEvent)
	{
		UAkGameplayStatics::PostEvent(ClickSoundEvent, this, 0, FOnAkPostEventCallback());
	}
}

void AGS_Monster::Attack()
{
	if (HasAuthority())
	{
		Multicast_PlayAttackMontage();
	}
}

void AGS_Monster::Multicast_PlayAttackMontage_Implementation()
{
	MonsterAnim->Montage_Play(AttackMontage);
}