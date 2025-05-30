// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Weapon/Equipable/GS_WeaponShield.h"
#include "Net/UnrealNetwork.h"
#include "UI/Character/GS_ChanAimingSkillBar.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"


// Sets default values
AGS_Chan::AGS_Chan()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterType = ECharacterType::Chan;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_ChanSkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));
	bReplicates = true;
}



// Called when the game starts or when spawned
void AGS_Chan::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnAndAttachWeapon(WeaponShieldClass, WeaponShieldName, WeaponShield);
		SpawnAndAttachWeapon(WeaponAxeClass, WeaponAxeName, WeaponAxe);
	}
	
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

void AGS_Chan::LeftClickPressed_Implementation()
{
	IGS_AttackInterface::LeftClickPressed_Implementation();
	if (CanAcceptComboInput)
	{
		if (CurrentComboIndex == 0)
		{
			ServerAttackMontage();
		}
		else
		{
			bNextCombo = true;
			CanAcceptComboInput = false;
		}
	}
}

void AGS_Chan::LeftClickRelease_Implementation()
{
	IGS_AttackInterface::LeftClickRelease_Implementation();
}

void AGS_Chan::ComboInputOpen()
{
	CanAcceptComboInput = true;
}

void AGS_Chan::ComboInputClose()
{
	CanAcceptComboInput = false;
}

void AGS_Chan::EndMontage()
{
	if (bNextCombo)
	{
		ServerAttackMontage();
		CanAcceptComboInput = false;
		bNextCombo = false;
	}
	else
	{
		if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			AnimInstance->Montage_Stop(0.1f);
			AnimInstance->IsPlayingComboMontage = false;
			CurrentComboIndex = 0;
			CanAcceptComboInput = true;
		}
	}
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


void AGS_Chan::MulticastPlayComboSection_Implementation()
{
	FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), CurrentComboIndex + 1));
	UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance());
	CurrentComboIndex++;
	if (AnimInstance && ComboAnimMontage)
	{
		AnimInstance->Montage_Play(ComboAnimMontage);
		AnimInstance->IsPlayingComboMontage = true;
		AnimInstance->Montage_JumpToSection(SectionName, ComboAnimMontage);

		// 콤보 공격 사운드와 공격 목소리 재생
		if (AxeSwingSound)
		{
			PlaySound(AxeSwingSound);
		}
		if (AttackVoiceSound)
		{
			PlaySound(AttackVoiceSound);
		}
	}
	CanAcceptComboInput = false;
	bNextCombo = false;
}

void AGS_Chan::ServerAttackMontage_Implementation()
{
	if (HasAuthority())
	{
		MulticastPlayComboSection();
	}
}

void AGS_Chan::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_Chan, WeaponAxe);
	DOREPLIFETIME(AGS_Chan, WeaponShield);
}
