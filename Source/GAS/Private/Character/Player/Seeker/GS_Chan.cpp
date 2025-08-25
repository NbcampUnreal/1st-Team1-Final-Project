// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Character/Component/GS_StatComp.h"
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

/*void AGS_Chan::OnComboAttack()
{
	Super::OnComboAttack();	
}*/

void AGS_Chan::MulticastPlayComboSection()
{
	Super::MulticastPlayComboSection();

	// 오디오 컴포넌트를 통해 찬 전용 콤보 공격 사운드 재생
	if (SeekerAudioComponent)
	{
		// 현재 콤보 인덱스를 가져와서 적절한 사운드 재생
		// TODO: 현재 콤보 인덱스를 추적하는 로직 필요
		SeekerAudioComponent->PlayChanComboAttackSound(0); // 기본값 0, 실제로는 현재 콤보 인덱스 사용
	}
}

void AGS_Chan::Multicast_OnAttackHit_Implementation(int32 ComboIndex)
{
	// 4번째 공격일 때 특별한 사운드 재생
	if (ComboIndex == 4 && SeekerAudioComponent)
	{
		SeekerAudioComponent->PlayChanFinalAttackSound();
	}
}

void AGS_Chan::OnJumpAttackSkill()
{
	/*if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = true;
	}*/
	Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
}

void AGS_Chan::OffJumpAttackSkill()
{
	/*if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = false;
		
	}*/
	Multicast_SetMontageSlot(ESeekerMontageSlot::None);
	StopAnimMontage();
}

void AGS_Chan::ToIdle()
{
	Multicast_StopSkillMontage(GetCurrentMontage());
	Multicast_SetMontageSlot(ESeekerMontageSlot::None);
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

float AGS_Chan::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
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
