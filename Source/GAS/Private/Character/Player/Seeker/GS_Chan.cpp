// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Weapon/Equipable/GS_WeaponShield.h"
#include "Net/UnrealNetwork.h"
#include "UI/Character/GS_ChanAimingSkillBar.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/GS_TpsController.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"


// Sets default values
AGS_Chan::AGS_Chan()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterType = ECharacterType::Chan;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_ChanSkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));
}

void AGS_Chan::Multicast_PlaySkillSound_Implementation(UAkAudioEvent* SoundToPlay)
{
	if (SoundToPlay && AkComponent)
	{
		AkComponent->PostAkEvent(SoundToPlay);
	}
}

void AGS_Chan::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called when the game starts or when spawned
void AGS_Chan::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);
	GetMesh()->SetIsReplicated(true);
}

// Called every frame
void AGS_Chan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGS_Chan::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Chan::OnComboAttack()
{
	Super::OnComboAttack();	
}

void AGS_Chan::OnJumpAttackSkill()
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = true;
	}
}

void AGS_Chan::OffJumpAttackSkill()
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = false;
	}
	StopAnimMontage();
}

void AGS_Chan::ToIdle()
{
	Multicast_StopSkillMontage(GetCurrentMontage());
	Multicast_SetIsUpperBodySlot(false);
	Multicast_SetIsFullBodySlot(false);
	SetMoveControlValue(true, true);
	SetLookControlValue(true, true);
}

void AGS_Chan::Client_UpdateChanAimingSkillBar_Implementation(float Stamina)
{
	if(ChanAimingSkillBarWidget)
	{
		ChanAimingSkillBarWidget->SetAimingProgress(Stamina);
	}
}

void AGS_Chan::Client_ChanAimingSkillBar_Implementation(bool bShow)
{
	if (ChanAimingSkillBarWidget)
	{
		ChanAimingSkillBarWidget->ShowSkillBar(bShow);
	}
}

void AGS_Chan::Multicast_DrawSkillRange_Implementation(FVector InLocation, float InRadius, FColor InColor, float InLifetime)
{
	/*DrawDebugSphere(
		GetWorld(),
		InLocation,
		InRadius,
		16,
		InColor,
		false,
		InLifetime
	);*/
}
