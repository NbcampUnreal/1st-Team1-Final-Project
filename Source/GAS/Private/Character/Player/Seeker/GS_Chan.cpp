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
#include "Engine/DamageEvents.h"


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

void AGS_Chan::ResetCurrentStamina()
{
	CurrentStamina = MaxStamina;
}

void AGS_Chan::SetCurrentStamina(float NewValue, bool SetbyDamage)
{
	CurrentStamina = FMath::Clamp(NewValue, 0.f, MaxStamina);
	Client_UpdateChanAimingSkillBar(CurrentStamina / MaxStamina);
	// UI 반영
	/*if (SetbyDamage)
	{
		Client_UpdateChanAimingSkillBarDealy(CurrentStamina / MaxStamina);
	}
	else
	{
		Client_UpdateChanAimingSkillBar(CurrentStamina / MaxStamina);
	}*/

	// 스테미나가 다 떨어지면 스킬
	if (CurrentStamina <= 0.f && SkillComp && CurrentStamina > 0.f) // 직전 값 기준 체크
	{
		SkillComp->Server_TryDeactiveSkill(ESkillSlot::Aiming);
	}
}

// Called when the game starts or when spawned
void AGS_Chan::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
	GetMesh()->SetIsReplicated(true);

	UltimateCollision->OnComponentBeginOverlap.AddDynamic(this, &AGS_Chan::OnUltimateOverlap);

	CurrentStamina = MaxStamina;
	MaxHealth = GetStatComp()->GetMaxHealth();
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

	// 3번째 공격(Attack3)에서만 방패 콜리전 활성화
	if (HasAuthority() && CurrentComboIndex == 3)
	{
		// 방패 찾기 및 활성화
		bool bShieldFound = false;
		for (int32 i = 0; i < 5; ++i)
		{
			if (AGS_WeaponShield* Shield = Cast<AGS_WeaponShield>(GetWeaponByIndex(i)))
			{
				Shield->ServerEnableHit();
				bShieldFound = true;
				
				// 0.8초 후 비활성화 (방패 공격 지속 시간을 좀 더 길게)
				GetWorldTimerManager().ClearTimer(ShieldDisableTimer);
				GetWorldTimerManager().SetTimer(ShieldDisableTimer, [Shield]()
				{
					if (Shield && IsValid(Shield))
					{
						Shield->ServerDisableHit();
					}
				}, 0.8f, false);
				break;
			}
		}
		
		if (!bShieldFound)
		{
			UE_LOG(LogTemp, Error, TEXT("[Chan] Shield not found in any weapon slot!"));
		}
	}

	// 오디오 컴포넌트를 통해 찬 전용 콤보 공격 사운드 재생
	if (SeekerAudioComponent)
	{
		// 현재 콤보 인덱스를 가져와서 적절한 사운드 재생
		SeekerAudioComponent->PlayChanComboAttackSound(CurrentComboIndex);
	}
}

void AGS_Chan::Multicast_OnAttackHit_Implementation(int32 ComboIndex)
{
	// 4번째 공격일 때 특별한 사운드 재생
	if (ComboIndex == 4 && SeekerAudioComponent)
	{
		SeekerAudioComponent->PlayChanFinalAttackSound();
	}
	
	// 공격 성공 시 공격자에게 카메라 쉐이크 적용 (Chan 전용)
	if (HasAuthority())
	{
		if (APlayerController* AttackerPC = Cast<APlayerController>(GetController()))
		{
			// 4번째 공격(마지막 공격)은 더 강한 쉐이크 적용
			if (ComboIndex == 4)
			{
				// 강한 공격 성공 쉐이크 (마지막 콤보)
				FGS_CameraShakeInfo StrongAttackShake = AttackSuccessShake;
				StrongAttackShake.Intensity *= 1.5f; // 강도 1.5배 증가
				Client_PlayAttackSuccessShakeWithInfo(AttackerPC, StrongAttackShake);
			}
			else
			{
				// 일반 공격 성공 쉐이크
				Client_PlayAttackSuccessShake(AttackerPC);
			}
		}
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

void AGS_Chan::Client_UpdateChanAimingSkillBarDealy_Implementation(float Stamina)
{
	if (ChanAimingSkillBarWidget)
	{
		ChanAimingSkillBarWidget->SetAimingProgressByDamage(Stamina);
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
	float ActualDamage = DamageAmount;
	
	// 방어 상태일 때는 스테미나 감소 (피격 애니메이션 방지)
	if (bIsDefending)
	{
		// 방어 효과음 재생
		if (UGS_SeekerAudioComponent* SeekerAudio = GetComponentByClass<UGS_SeekerAudioComponent>())
		{
			SeekerAudio->PlayDefenseSound();
		}
		
		// 방어 VFX 재생 (나중에 구현)
		// PlayDefenseVFX();
		
		// 방어 성공 시 데미지 0으로 설정하여 피격 애니메이션 방지
		ActualDamage = 0.0f;

		// 스테미나 감소
		if (MaxHealth > 0.f)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Stamina Damage In"));
			float StaminaDamage = DamageAmount * (MaxStamina / MaxHealth);
			SetCurrentStamina(CurrentStamina - StaminaDamage, true);
		}
	}
	else
	{
		// 방어 상태가 아닐 때만 부모 클래스의 TakeDamage 호출
		//UE_LOG(LogTemp, Warning, TEXT("Normal Damage In"));
		ActualDamage = Super::TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
	}

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

void AGS_Chan::SetDefending(bool bDefending)
{
	if (HasAuthority())
	{
		bIsDefending = bDefending;
		
		// 방패의 방어용 콜리전 제어
		for (int32 i = 0; i < 5; ++i)
		{
			if (AGS_WeaponShield* Shield = Cast<AGS_WeaponShield>(GetWeaponByIndex(i)))
			{
				if (bDefending)
				{
					// 방어 시작 - 방어용 콜리전 활성화
					Shield->ServerEnableDefenseHit();
				}
				else
				{
					// 방어 해제 - 방어용 콜리전 비활성화
					Shield->ServerDisableDefenseHit();
				}
				break;
			}
		}
		
		// 방어 상태에 따른 애니메이션 변경 (나중에 구현)
		if (bDefending)
		{
			// 방어 애니메이션 재생
			// Multicast_PlayDefenseAnimation();
		}
		else
		{
			// 기본 애니메이션으로 복귀
			// Multicast_StopDefenseAnimation();
		}
	}
}

void AGS_Chan::OnRep_IsDefending()
{
	// 방어 상태 변경 시 UI 업데이트 등 (나중에 구현)
	if (bIsDefending)
	{
		// 방어 UI 표시
		// ShowDefenseUI(true);
	}
	else
	{
		// 방어 UI 숨기기
		// ShowDefenseUI(false);
	}
}

bool AGS_Chan::IsHitInShieldDefenseArea(const FVector& HitLocation) const
{
	// 방패를 찾아서 방어 영역 확인
	for (int32 i = 0; i < 5; ++i)
	{
		if (AGS_WeaponShield* Shield = Cast<AGS_WeaponShield>(GetWeaponByIndex(i)))
		{
			if (Shield && Shield->DefenseHitBox)
			{
				// 방패의 월드 위치와 방어용 콜리전 크기 가져오기
				FVector ShieldLocation = Shield->GetActorLocation();
				FVector ShieldForward = Shield->GetActorForwardVector();
				
				// 방패 방어 영역 계산 (방패 앞쪽 반구형 영역)
				const float DefenseRadius = 200.0f; // 방패 방어 반경
				const float DefenseAngle = 120.0f;  // 방패 방어 각도 (도)
				
				// 타격 지점과 방패 사이의 거리 계산
				FVector ToHit = HitLocation - ShieldLocation;
				float Distance = ToHit.Size();
				
				// 거리가 방어 반경을 벗어나면 방어 불가
				if (Distance > DefenseRadius)
				{
					return false;
				}
				
				// 타격 지점이 방패 앞쪽에 있는지 확인 (각도 체크)
				ToHit.Normalize();
				float DotProduct = FVector::DotProduct(ShieldForward, ToHit);
				float AngleInRadians = FMath::Acos(DotProduct);
				float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);
				
				// 방어 각도 내에 있으면 방어 가능
				if (AngleInDegrees <= DefenseAngle * 0.5f)
				{
					return true;
				}
			}
			break;
		}
	}
	
	// 방패를 찾지 못했거나 방어 영역 밖이면 방어 불가
	return false;
}
