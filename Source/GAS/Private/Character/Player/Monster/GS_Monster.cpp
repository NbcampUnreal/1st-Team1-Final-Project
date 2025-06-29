// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_Monster.h"
#include "AI/GS_AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AkComponent.h"
#include "Animation/Character/GS_MonsterAnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Sound/GS_AudioManager.h"
#include "Sound/GS_CharacterAudioSystem.h"
#include "EngineUtils.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Character/Skill/Monster/GS_MonsterSkillComp.h"
#include "Sound/GS_MonsterAudioComponent.h"
#include "Character/Component/GS_DebuffVFXComponent.h"
#include "Components/DecalComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


AGS_Monster::AGS_Monster()
{
	AIControllerClass = AGS_AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	MonsterSkillComp = CreateDefaultSubobject<UGS_MonsterSkillComp>(TEXT("MonsterSkillComp"));

	SkillCooldownWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("SkillCooldownWidgetComp"));
	SkillCooldownWidgetComp->SetupAttachment(RootComponent);
	SkillCooldownWidgetComp->SetVisibility(false);

	AkComponent = CreateDefaultSubobject<UAkComponent>("AkComponent");
	AkComponent->SetupAttachment(RootComponent);
	
	// 몬스터 오디오 컴포넌트 생성
	MonsterAudioComponent = CreateDefaultSubobject<UGS_MonsterAudioComponent>("MonsterAudioComponent");
	
	// 디버프 VFX 컴포넌트 생성
	DebuffVFXComponent = CreateDefaultSubobject<UGS_DebuffVFXComponent>("DebuffVFXComponent");

	TeamId = FGenericTeamId(2);
	Tags.Add("Monster");

	// RTS 선택을 위한 콜리전 설정 (모든 몬스터에 적용)
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block); // Interactable
	}
	
	bCommandLocked = false;
	bSelectionLocked = false;
	bIsSelected = false;
}

void AGS_Monster::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(MonsterSkillComp))
	{
		MonsterSkillComp->OnMonsterSkillCooldownChanged.AddDynamic(this, &AGS_Monster::HandleSkillCooldownChanged);
	}
	
	// AkComponent Occlusion 비활성화
	if (AkComponent)
	{
		AkComponent->OcclusionRefreshInterval = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("AGS_Monster: AkComponent occlusion DISABLED."));
	}
	
	if (IsValid(SkillCooldownWidgetComp) && !HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			SkillCooldownWidgetTimer,
			this,
			&AGS_Monster::UpdateSkillCooldownWidget,
			0.1f, 
			true 
		);
	}
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

void AGS_Monster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(SkillCooldownWidgetTimer);
} 

void AGS_Monster::OnDeath()
{
	Super::OnDeath();
	
	if (MonsterAudioComponent)
	{
		MonsterAudioComponent->PlayDeathSound();
	}
	
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
	Multicast_OnDeath();

	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AGS_Monster::HandleDelayedDestroy,
		2.f,
		false
	);
}

void AGS_Monster::HandleDelayedDestroy()
{
	Destroy();
}

void AGS_Monster::Multicast_OnDeath_Implementation()
{
	OnMonsterDead.Broadcast(this);
}


void AGS_Monster::UseSkill()
{	
}

void AGS_Monster::SetCanUseSkill(bool bCanUse)
{
	if (MonsterSkillComp)
	{
		MonsterSkillComp->SetCanUseSkill(bCanUse);
	}
}

void AGS_Monster::HandleSkillCooldownChanged(float InCurrentCoolTime, float InMaxCoolTime)
{
	if (SkillCooldownWidgetComp)
	{
		if (InCurrentCoolTime > 0.0f)
		{
			SkillCooldownWidgetComp->SetVisibility(true);
		}
		else
		{
			SkillCooldownWidgetComp->SetVisibility(false);
		}
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


void AGS_Monster::SetSelected(bool bSelected, bool bPlaySound)
{
	bIsSelected = bSelected;
	UpdateDecal();

	// 선택되었고, 소리 재생이 허용된 경우에만 소리 재생
	if (bSelected && bPlaySound && ClickSoundEvent)
	{
		UAkGameplayStatics::PostEvent(ClickSoundEvent, this, 0, FOnAkPostEventCallback());
	}
}

FLinearColor AGS_Monster::GetCurrentDecalColor()
{
	if (bIsSelected)
	{
		return FLinearColor::Green;
	}
	else if (bIsHovered)
	{
		return FLinearColor::Yellow;
	}
	else
	{
		return FLinearColor::Yellow;
	}
}

void AGS_Monster::UpdateDecal()
{
	if (!SelectionDecal || !ShowDecal())
	{
		SelectionDecal->SetVisibility(false);
		return;
	}

	if (bIsSelected || bIsHovered)
	{
		ShowDecalWithColor(GetCurrentDecalColor());
	}
	else
	{
		SelectionDecal->SetVisibility(false);
	}
}

bool AGS_Monster::ShowDecal()
{
	return true;
}

void AGS_Monster::UpdateSkillCooldownWidget()
{
	if (!IsValid(SkillCooldownWidgetComp))
	{
		GetWorldTimerManager().ClearTimer(SkillCooldownWidgetTimer);
		return;
	}
	
	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		FVector CameraForward = CameraManager->GetCameraRotation().Vector();
		FVector CameraRight = FVector::CrossProduct(CameraForward, FVector::UpVector).GetSafeNormal();
		FVector CameraUp = FVector::CrossProduct(CameraRight, CameraForward).GetSafeNormal();
		FRotator WidgetRotation = UKismetMathLibrary::MakeRotFromXZ(-CameraForward, CameraUp);

		SkillCooldownWidgetComp->SetWorldRotation(WidgetRotation);
	}
	
}
