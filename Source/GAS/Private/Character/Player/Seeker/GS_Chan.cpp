// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Weapon/Equipable/GS_WeaponShield.h"
#include "Net/UnrealNetwork.h"
#include "UI/Character/GS_ChanAimingSkillBar.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/GS_TpsController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Skill/GS_SkillComp.h"


// Sets default values
AGS_Chan::AGS_Chan()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterType = ECharacterType::Chan;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_ChanSkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));
	bReplicates = true;
}

void AGS_Chan::Multicast_SetMustTurnInPlace_Implementation(bool MustTurn)
{
	Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance())->SetMustTurnInPlace(MustTurn);
}

void AGS_Chan::Multicast_PlaySkillSound_Implementation(UAkAudioEvent* SoundToPlay)
{
	if (SoundToPlay && AkComponent)
	{
		AkComponent->PostAkEvent(SoundToPlay);
	}
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

void AGS_Chan::ComboInputOpen()
{
	CanAcceptComboInput = true;
}

void AGS_Chan::ComboInputClose()
{
	CanAcceptComboInput = false;
	if (bNextCombo)
	{
		ServerAttackMontage();
		CanAcceptComboInput = false;
		bNextCombo = false;
	}
}

void AGS_Chan::ComboEnd()
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		StopAnimMontage();
		//AnimInstance->Montage_Stop(0.1f);
		AnimInstance->IsPlayingUpperBodyMontage = false;
		CurrentComboIndex = 0;
		CanAcceptComboInput = true;

		AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController());
		if (IsValid(TPSController))
		{
			//TPSController->GetControlValue().bCanLookRight = true; SJE
			TPSController->SetLookControlValue(true, true);
		}
	}
}

/*void AGS_Chan::OnMoveSkill()
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = true;
	}
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		TPSController->SetMoveControlValue(false,false);
	}
	bUseControllerRotationYaw = false;
}

void AGS_Chan::OffMoveSkill()
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = false;
	}
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		/*TpsController->GetControlValue().bCanMoveRight = true;
		TpsController->GetControlValue().bCanMoveForward = true;#1# // SJE
		TPSController->SetMoveControlValue(true, true);
	}
	StopAnimMontage();
	bUseControllerRotationYaw = true;
}*/

void AGS_Chan::OnReadyAimSkill()
{
	
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = false;
		AnimInstance->IsPlayingUpperBodyMontage = true;
	}
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
	UE_LOG(LogTemp, Warning, TEXT("ToIdle"));
	Multicast_StopSkillMontage(GetCurrentMontage());
	Multicast_SetIsUpperBodySlot(false);
	Multicast_SetIsFullBodySlot(false);
	Multicast_SetMoveControlValue(true, true);
	Multicast_SetLookControlValue(true, true);
}

void AGS_Chan::Multicast_SetLookControlValue_Implementation(bool bLookUp, bool bLookRight)
{
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		TPSController->SetLookControlValue(bLookRight, bLookUp);
	}
	
}

void AGS_Chan::Multicast_SetMoveControlValue_Implementation(bool bMoveForward, bool bMoveRight)
{
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		TPSController->SetMoveControlValue(bMoveRight, bMoveForward);
	}
}

void AGS_Chan::Multicast_SetIsFullBodySlot_Implementation(bool bFullBodySlot)
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = bFullBodySlot;
	}
}

void AGS_Chan::Multicast_SetIsUpperBodySlot_Implementation(bool bUpperBodySlot)
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingUpperBodyMontage = bUpperBodySlot;
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
	AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController());
	if (IsValid(TPSController))
	{
		TPSController->SetLookControlValue(false, true);
	}
	
	FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), CurrentComboIndex + 1));
	UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance());
	AnimInstance->SetMotionMatchingPlayRate(2.0f, 2.0f);
	
	CurrentComboIndex++;
	if (AnimInstance && ComboAnimMontage)
	{
		AnimInstance->Montage_Play(ComboAnimMontage);
		AnimInstance->IsPlayingUpperBodyMontage = true;
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
	MulticastPlayComboSection();
}

void AGS_Chan::Multicast_DrawSkillRange_Implementation(FVector InLocation, float InRadius, FColor InColor, float InLifetime)
{
	DrawDebugSphere(
		GetWorld(),
		InLocation,
		InRadius,
		16,
		InColor,
		false,
		InLifetime
	);
}
