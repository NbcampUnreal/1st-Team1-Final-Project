// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_Monster.h"
#include "AI/GS_AIController.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AkComponent.h"
#include "Animation/Character/GS_MonsterAnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Sound/GS_AudioManager.h"
#include "Sound/GS_CharacterAudioSystem.h"
#include "EngineUtils.h"
#include "Character/Player/Seeker/GS_Seeker.h"

// 정적 멤버 변수 초기화 - 멀티플레이 문제로 제거
// TArray<AGS_Monster*> AGS_Monster::ActiveCombatMonsters;
// AkPlayingID AGS_Monster::CurrentCombatMusicID = AK_INVALID_PLAYING_ID;

AGS_Monster::AGS_Monster()
{
	AIControllerClass = AGS_AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	Weapon = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponComp"));
	Weapon->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));

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

	CombatTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("CombatTrigger"));
	CombatTrigger->SetupAttachment(RootComponent);
	CombatTrigger->SetSphereRadius(1200.0f);

	// *** SoundTrigger 프리셋 사용 ***
	CombatTrigger->SetCollisionProfileName(TEXT("SoundTrigger"));

	CombatTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGS_Monster::OnCombatTriggerBeginOverlap);
	CombatTrigger->OnComponentEndOverlap.AddDynamic(this, &AGS_Monster::OnCombatTriggerEndOverlap);
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
	
	// 주변의 모든 Seeker에게 이 몬스터 제거 알림
	if (GetWorld())
	{
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			APawn* Pawn = *It;
			if (Pawn && Pawn->IsA(AGS_Seeker::StaticClass()))
			{
				if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(Pawn))
				{
					Seeker->RemoveCombatMonster(this);
				}
			}
		}
	}
	
	DetachFromControllerPendingDestroy();
	SetLifeSpan(4.f);
	
	Multicast_OnDeath();
}

void AGS_Monster::Multicast_OnDeath_Implementation()
{
	OnMonsterDead.Broadcast(this);
}

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

void AGS_Monster::OnCombatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AGS_Seeker::StaticClass()))
	{
		if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor))
		{
			Seeker->AddCombatMonster(this);
		}
	}
}

void AGS_Monster::OnCombatTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(AGS_Seeker::StaticClass()))
	{
		if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor))
		{
			Seeker->RemoveCombatMonster(this);
		}
	}
}