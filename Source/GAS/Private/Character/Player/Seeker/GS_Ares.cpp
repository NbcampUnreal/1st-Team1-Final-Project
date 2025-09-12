// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Ares.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"
#include "Character/Component/GS_StatComp.h"

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

	// 사운드 배열들은 GS_SeekerAudioComponent에서 관리됨
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

/*void AGS_Ares::OnComboAttack()
{
	Super::OnComboAttack();
}*/

void AGS_Ares::ServerAttackMontage()
{
	Super::ServerAttackMontage();
}

void AGS_Ares::MulticastPlayComboSection()
{	
	Super::MulticastPlayComboSection();

	// SeekerAudioComponent를 통해 아레스 전용 콤보 공격 사운드 재생
	if (SeekerAudioComponent)
	{
		// GS_SeekerAudioComponent의 AresComboXXX 프로퍼티들을 사용하여 사운드 재생
		SeekerAudioComponent->PlayAresComboAttackSoundWithExtra(CurrentComboIndex);
	}
}

void AGS_Ares::Multicast_OnAttackHit_Implementation(int32 ComboIndex)
{
	if (!SeekerAudioComponent)
	{
		return;
	}

	// GS_SeekerAudioComponent의 PlayAresComboAttackSoundWithExtra 함수에서 
	// 추가 사운드가 자동으로 재생되므로 별도 처리 불필요
	// 필요시 여기서 추가 로직 구현 가능
}

float AGS_Ares::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// Call parent implementation
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Play hurt sound if we actually took damage and are still alive
	if (ActualDamage > 0.0f && GetStatComp() && GetStatComp()->GetCurrentHealth() > 0.0f)
	{
		if (UGS_SeekerAudioComponent* SeekerAudio = GetComponentByClass<UGS_SeekerAudioComponent>())
		{
			SeekerAudio->PlayHurtSound();
		}
	}

	return ActualDamage;
}

