// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/GS_TpsController.h"
#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"
#include "AkAudioDevice.h"


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
	
	// 사운드 시스템 초기화 - 모든 클라이언트에서 사운드 정리
	Multicast_StopAttackSound();
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
	//bComboEnded = false;
	CanChangeSeekerGait = false;

	if (CanAcceptComboInput)
	{
		if (CurrentComboIndex == 0)
		{
			// 새로운 콤보 시작 시 이전 사운드 정리 (멀티캐스트)
			GetWorldTimerManager().ClearTimer(AttackSoundResetTimerHandle);
			Multicast_StopAttackSound();
			
			ServerAttackMontage();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("On Combo Attack CanAcceptComboInput : %d"), CanAcceptComboInput);
			bNextCombo = true;
			CanAcceptComboInput = false;
		}
	}
}

void AGS_Ares::ServerAttackMontage()
{
	Super::ServerAttackMontage();

	this->MulticastPlayComboSection();
}

void AGS_Ares::MulticastPlayComboSection()
{
	// 1. 기존 타이머가 있다면 클리어하고 즉시 Stop 이벤트 호출 (멀티캐스트)
	GetWorldTimerManager().ClearTimer(AttackSoundResetTimerHandle);
	Multicast_StopAttackSound();
	
	// 2. 부모 클래스의 콤보 로직 실행 (CurrentComboIndex++ 포함)
	Super::MulticastPlayComboSection();
	
	// 3. 새로운 공격 사운드 재생
	if (SwordSwingSound)
	{
		Multicast_PlaySkillSound(SwordSwingSound);
	}
	
	if (AttackVoiceSound)
	{
		Multicast_PlaySkillSound(AttackVoiceSound);
	}
	
	// 4. 공격 후 일정 시간 뒤 사운드 시퀀스 리셋을 위한 타이머 설정
	GetWorldTimerManager().SetTimer(
		AttackSoundResetTimerHandle,
		this,
		&AGS_Ares::ResetAttackSoundSequence,
		AttackSoundResetTime,  // 설정 가능한 시간 사용
		false
	);
}

void AGS_Ares::ResetAttackSoundSequence()
{
	// 멀티캐스트로 모든 클라이언트에서 Stop 이벤트 호출
	Multicast_StopAttackSound();
}

void AGS_Ares::Multicast_OnAttackHit_Implementation(int32 ComboIndex)
{
	// 4번째 공격일 때 특별한 사운드 재생
	if (ComboIndex == 4 && FinalAttackExtraSound)
	{
		Multicast_PlaySkillSound(FinalAttackExtraSound);
	}
}

void AGS_Ares::Multicast_StopAttackSound_Implementation()
{
	// 데디케이티드 서버에서는 사운드 재생하지 않음
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		return;
	}

	// 사용자가 만든 Wwise Stop 이벤트 호출
	if (SwordSwingStopEvent)
	{
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (AkComp)
		{
			AkComp->PostAkEvent(SwordSwingStopEvent);
		}
	}
}

