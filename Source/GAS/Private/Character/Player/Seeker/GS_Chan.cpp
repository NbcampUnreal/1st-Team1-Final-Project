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
	
	/*float ControlYaw = GetControlRotation().Yaw;
	if (AGS_TpsController* TpsController = Cast<AGS_TpsController>(GetController()))
	{
		float YawDiff = FMath::Abs(FMath::FindDeltaAngleDegrees(ControlYaw, TpsController->LastRotatorInMoving.Yaw));
	
		if (YawDiff > 50)
		{
			return;
		}
	}*/
	
	Server_ComboEnd(false);
	CanChangeSeekerGait = false;
	
	if (CanAcceptComboInput)
	{
		if (CurrentComboIndex == 0)
		{
			ServerAttackMontage();
			//Server_SetSeekerGait(EGait::Walk);
		}
		else
		{
			bNextCombo = true;
			CanAcceptComboInput = false;
		}
	}
}

void AGS_Chan::MulticastPlayComboSection()
{
	// 기존 타이머가 있다면 클리어 (Stop 이벤트는 호출하지 않음)
	GetWorldTimerManager().ClearTimer(AttackSoundResetTimerHandle);
	
	// 부모 클래스의 콤보 로직 실행 (CurrentComboIndex++ 포함)
	Super::MulticastPlayComboSection();
	
	// 공격 사운드 재생
	if (AxeSwingSound)
	{
		Multicast_PlaySkillSound(AxeSwingSound);
	}
	
	if (AttackVoiceSound)
	{
		Multicast_PlaySkillSound(AttackVoiceSound);
	}
	
	// 공격 후 일정 시간 뒤 사운드 시퀀스 리셋을 위한 타이머 설정
	GetWorldTimerManager().SetTimer(
		AttackSoundResetTimerHandle,
		this,
		&AGS_Chan::ResetAttackSoundSequence,
		1.0f,  // 1초로 고정
		false
	);
}

void AGS_Chan::ResetAttackSoundSequence()
{
	// 사용자가 만든 Wwise Stop 이벤트 호출
	if (AxeSwingStopEvent && AkComponent)
	{
		AkComponent->PostAkEvent(AxeSwingStopEvent);
	}
}

void AGS_Chan::Multicast_OnAttackHit_Implementation(int32 ComboIndex)
{
	// 4번째 공격일 때 특별한 사운드 재생
	if (ComboIndex == 4 && FinalAttackExtraSound)
	{
		Multicast_PlaySkillSound(FinalAttackExtraSound);
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
	Multicast_StopSkillMontage(GetCurrentMontage());
	Multicast_SetIsUpperBodySlot(false);
	Multicast_SetIsFullBodySlot(false);
	SetMoveControlValue(true, true);
	SetLookControlValue(true, true);
}

void AGS_Chan::SetMoveControlValue(bool bMoveForward, bool bMoveRight)
{
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		TPSController->SetMoveControlValue(bMoveRight, bMoveForward);
	}
}
void AGS_Chan::SetLookControlValue(bool bLookUp, bool bLookRight)
{
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		TPSController->SetLookControlValue(bLookRight, bLookUp);
	}
}

void AGS_Chan::Multicast_SetMustTurnInPlace_Implementation(bool MustTurn)
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->SetMustTurnInPlace(MustTurn);
	}
}

void AGS_Chan::Multicast_SetIsFullBodySlot_Implementation(bool bFullBodySlot)
{
	if (!IsValid(this) || !GetWorld() || GetWorld()->bIsTearingDown || GetWorld()->IsInSeamlessTravel())
	{
		return;
	}

	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = bFullBodySlot;
	}
}

void AGS_Chan::Multicast_SetIsUpperBodySlot_Implementation(bool bUpperBodySlot)
{
	if (!IsValid(this) || !GetWorld() || GetWorld()->bIsTearingDown || GetWorld()->IsInSeamlessTravel())
	{
		return;
	}

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
