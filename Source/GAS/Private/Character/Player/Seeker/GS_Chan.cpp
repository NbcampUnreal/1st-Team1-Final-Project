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
#include "AkGameplayStatics.h"
#include "AkAudioDevice.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"
#include "Components/CapsuleComponent.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Skill/Seeker/Chan/GS_ChanUltimateSkill.h"


// Sets default values
AGS_Chan::AGS_Chan()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterType = ECharacterType::Chan;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_ChanSkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));

	UltimateCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("UltimateCollision"));
	UltimateCollision->SetupAttachment(GetRootComponent());
	UltimateCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UltimateCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	UltimateCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	UltimateCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	UltimateCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	UltimateCollision->SetGenerateOverlapEvents(true);
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

	UltimateCollision->OnComponentBeginOverlap.AddDynamic(this, &AGS_Chan::OnUltimateOverlap);

}

void AGS_Chan::OnUltimateOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (UGS_ChanUltimateSkill* Skill = Cast<UGS_ChanUltimateSkill>(
		SkillComp->GetSkillFromSkillMap(ESkillSlot::Ultimate)))
	{
		Skill->HandleUltimateCollision(OtherActor, OtherComp);
	
	}
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
	// 멀티캐스트로 모든 클라이언트에서 Stop 이벤트 호출
	Multicast_StopAttackSound();
}

void AGS_Chan::Multicast_StopAttackSound_Implementation()
{
	// 데디케이티드 서버에서는 사운드 재생하지 않음
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		return;
	}

	// 사용자가 만든 Wwise Stop 이벤트 호출
	if (AxeSwingStopEvent)
	{
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (AkComp)
		{
			AkComp->PostAkEvent(AxeSwingStopEvent);
		}
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
