// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Ares.h"
#include "Sound/GS_CharacterAudioComponent.h"
#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/GS_TpsController.h"
#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"


// Sets default values
AGS_Ares::AGS_Ares()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterType = ECharacterType::Ares;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_AresSkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));
}

// Called when the game starts or when spawned
void AGS_Ares::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
	GetMesh()->SetIsReplicated(true);
}

// Called every frame
void AGS_Ares::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGS_Ares::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGS_Ares::OnComboAttack()
{
	Super::OnComboAttack();
}

void AGS_Ares::ServerAttackMontage()
{
	Super::ServerAttackMontage();
}

void AGS_Ares::MulticastPlayComboSection()
{	
	Super::MulticastPlayComboSection();

	// 오디오 컴포넌트를 통해 콤보 공격 사운드 재생
	if (CharacterAudioComponent)
	{
		CharacterAudioComponent->PlayComboAttackSound(SwordSwingSound, AttackVoiceSound, SwordSwingStopEvent, AttackSoundResetTime);
	}
}

void AGS_Ares::Multicast_OnAttackHit_Implementation(int32 ComboIndex)
{
	// 4번째 공격일 때 특별한 사운드 재생
	if (ComboIndex == 4 && CharacterAudioComponent)
	{
		CharacterAudioComponent->PlayFinalAttackSound(FinalAttackExtraSound);
	}
}

