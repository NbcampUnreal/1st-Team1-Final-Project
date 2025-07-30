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

	// 콤보별 사운드 배열 기본 크기 설정 (4개 콤보)
	ComboSwingSounds.SetNum(4);
	ComboVoiceSounds.SetNum(4);
	ComboExtraSounds.SetNum(4);
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
		// 새로운 콤보별 사운드 시스템 사용 (추가 사운드 포함)
		if (ComboSwingSounds.Num() > 0 || ComboVoiceSounds.Num() > 0 || ComboExtraSounds.Num() > 0)
		{
			CharacterAudioComponent->PlayComboAttackSoundByIndexWithExtra(CurrentComboIndex, ComboSwingSounds, ComboVoiceSounds, ComboExtraSounds, SwordSwingStopEvent, AttackSoundResetTime);
		}
		// 레거시 시스템으로 폴백 (기존 설정이 있는 경우)
		else if (SwordSwingSound || AttackVoiceSound)
		{
			CharacterAudioComponent->PlayComboAttackSound(SwordSwingSound, AttackVoiceSound, SwordSwingStopEvent, AttackSoundResetTime);
		}
	}
}

void AGS_Ares::Multicast_OnAttackHit_Implementation(int32 ComboIndex)
{
	if (!CharacterAudioComponent)
	{
		return;
	}

	// 새로운 콤보별 추가 사운드 시스템 사용
	int32 ArrayIndex = ComboIndex - 1; // 1-based에서 0-based로 변환
	if (ComboExtraSounds.IsValidIndex(ArrayIndex) && ComboExtraSounds[ArrayIndex])
	{
		CharacterAudioComponent->PlayFinalAttackSound(ComboExtraSounds[ArrayIndex]);
	}
	// 레거시 시스템으로 폴백 (4번째 공격에 대해서만)
	else if (ComboIndex == 4 && FinalAttackExtraSound)
	{
		CharacterAudioComponent->PlayFinalAttackSound(FinalAttackExtraSound);
	}
}

