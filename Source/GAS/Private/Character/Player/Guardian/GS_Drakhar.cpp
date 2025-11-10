#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Components/CapsuleComponent.h"
#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "AkAudioDevice.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ArrowComponent.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Component/GS_FootManagerComponent.h"
#include "Character/Skill/Guardian/Drakhar/GS_EarthquakeEffect.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Character/GS_DrakharFeverGauge.h"
#include "Character/Component/GS_DrakharVFXComponent.h"
#include "Character/Component/GS_DrakharAudioComponent.h"
#include "Character/F_GS_DamageEvent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UI/Character/GS_DrakharStaminaGauge.h"
#include "VFX/GS_VFX_FunctionLibrary.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Guardian의 VFXComponent를 제거하고 Drakhar 전용 컴포넌트로 교체
	if (VFXComponent)
	{
		VFXComponent->DestroyComponent();
		VFXComponent = nullptr;
	}
	
	DrakharVFXComponent = CreateDefaultSubobject<UGS_DrakharVFXComponent>(TEXT("DrakharVFXComponent"));
	AudioComponent = CreateDefaultSubobject<UGS_DrakharAudioComponent>(TEXT("AudioComponent"));
	FootManagerComponent = CreateDefaultSubobject<UGS_FootManagerComponent>(TEXT("FootManagerComponent"));
	
	// === 어스퀘이크 카메라 쉐이크 기본값 설정 ===
	EarthquakeShakeInfo.Intensity = 8.0f;
	EarthquakeShakeInfo.MaxDistance = 2500.0f;
	EarthquakeShakeInfo.MinDistance = 300.0f;
	EarthquakeShakeInfo.PropagationSpeed = 300000.0f; // 지진파 속도
	EarthquakeShakeInfo.bUseFalloff = true;

	//combo attack
	DefaultComboAttackSectionName = FName("Combo1");
	ComboAttackSectionName = DefaultComboAttackSectionName;
	bCanCombo = true;

	//dash skill variables
	DashPower = 1500.f;
	DashInterpAlpha = 0.f;
	DashDuration = 1.f;

	//earthquake skill variables
	EarthquakePower = 150.f;
	EarthquakeRadius = 500.f;

	//DraconicFury
	DraconicAttackPersistenceTime = 5.f;

	//Guardian State Setting
	GuardianState = EGuardianCtrlState::CtrlEnd;
	GuardianDoSkillState = EGuardianDoSkill::None;
	
	//fever mode
	MaxFeverGauge = 100.f;
	CurrentFeverGauge = 0.f;

	//team id
	TeamId = FGenericTeamId(0);

	//spring arm data for flying
	DefaultSpringArmLength = SpringArmComp->TargetArmLength;
	TargetSpringArmLength = 800.f;

	// === Wwise 사운드 이벤트 초기화 -> 이제 Drakhar 블루프린트에서 직접 설정.
	ComboAttackSoundEvent = nullptr;
	DashSkillSoundEvent = nullptr;
	EarthquakeSkillSoundEvent = nullptr;
	DraconicFurySkillSoundEvent = nullptr;
	DraconicProjectileSoundEvent = nullptr;
	DraconicProjectileImpactSoundEvent = nullptr;
	DraconicProjectileExplosionSoundEvent = nullptr;
	AttackHitSoundEvent = nullptr;
	ComboFinisherSoundEvent = nullptr;
	FeverModeStartSoundEvent = nullptr;
	FeverModeEndSoundEvent = nullptr;
	HurtSoundEvent = nullptr;

	// AkComponent 추가
	if (!FindComponentByClass<UAkComponent>())
	{
		UAkComponent* AkComp = CreateDefaultSubobject<UAkComponent>(TEXT("AkAudioComponent"));
		if (IsValid(AkComp))
		{
			AkComp->SetupAttachment(RootComponent);
		}
	}

	// === VFX 위치 제어용 화살표 컴포넌트 생성 ===
	WingRushVFXSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("WingRushVFXSpawnPoint"));
	if (WingRushVFXSpawnPoint)
	{
		WingRushVFXSpawnPoint->SetupAttachment(GetMesh(), FName("foot_l"));
		WingRushVFXSpawnPoint->SetRelativeLocation(FVector(-20.f, 0.f, 0.f));
		WingRushVFXSpawnPoint->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
		WingRushVFXSpawnPoint->SetArrowSize(2.0f);
		WingRushVFXSpawnPoint->SetArrowColor(FLinearColor::Blue);

#if WITH_EDITOR
		WingRushVFXSpawnPoint->bIsEditorOnly = false;
#endif
	}

	// === 어스퀘이크 VFX 위치 제어용 화살표 컴포넌트 생성 ===
	EarthquakeVFXSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("EarthquakeVFXSpawnPoint"));
	if (EarthquakeVFXSpawnPoint)
	{
		EarthquakeVFXSpawnPoint->SetupAttachment(GetMesh(), FName("hand_r")); // 오른손 본에 부착
		EarthquakeVFXSpawnPoint->SetRelativeLocation(FVector(100.f, 0.f, -150.f));
		EarthquakeVFXSpawnPoint->SetRelativeRotation(FRotator(0.f, 0.f, -90.f)); // 아래쪽 향하게
		EarthquakeVFXSpawnPoint->SetArrowSize(5.0f);
		EarthquakeVFXSpawnPoint->SetArrowColor(FLinearColor::Red);

#if WITH_EDITOR
		EarthquakeVFXSpawnPoint->bIsEditorOnly = false;
#endif
	}

	FlyingDustTraceDistance = 2000.f;

	// KeyManual에서 쓰일 캐릭터 타입 저장
	ManualRowName = FName("Drakhar");
	FlyingStaminaCoolTime = MaxFlyingStaminaCoolTime;
}

void AGS_Drakhar::BeginPlay()
{
	Super::BeginPlay();
	
	UGS_DrakharAnimInstance* Anim = Cast<UGS_DrakharAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(Anim))
	{
		Anim->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::OnMontageNotifyBegin);
	}
}

void AGS_Drakhar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (SpringArmComp && bIsFlying)
	{
		if (FMath::IsNearlyEqual(SpringArmComp->TargetArmLength, TargetSpringArmLength, 1.0f))
		{
			SpringArmComp->TargetArmLength = TargetSpringArmLength;
			bIsFlying = false;
		}
		else
		{
			SpringArmComp->TargetArmLength = FMath::FInterpTo(SpringArmComp->TargetArmLength, TargetSpringArmLength, DeltaTime, 5.0f);
		}
	}
}

void AGS_Drakhar::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanCombo);
	DOREPLIFETIME(ThisClass, CurrentFeverGauge);
	DOREPLIFETIME(ThisClass, IsFeverMode);
	DOREPLIFETIME(ThisClass, FlyingStaminaCoolTime);
}

void AGS_Drakhar::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 타이머 정리 (레벨 전환 시 크래시 방지)
	SafeClearTimer(FeverTimer);
	SafeClearTimer(FeverStateSoundDelayTimer);
	SafeClearTimer(ResetAttackTimer);
	SafeClearTimer(HealthRegenTimer);
	SafeClearTimer(HealthDelayTimer);
	SafeClearTimer(DraconicAttackTimer);  // 궁극기 타이머
	SafeClearTimer(CameraZoomTimer);      // 카메라 효과 타이머 (통합됨)
}

void AGS_Drakhar::OnDamageStart()
{
	bIsDamaged = true;

	StopHealRegeneration();

	//timer start (타이머 설정)
	UWorld* World = GetWorld();
	if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
	{
		World->GetTimerManager().SetTimer(HealthDelayTimer, this, &AGS_Drakhar::BeginHealRegeneration, 5.f, false);
	}
}

void AGS_Drakhar::Ctrl()
{
	Super::Ctrl();
	
	if (!HasAuthority() && IsLocallyControlled())
	{
		if (FMath::IsNearlyZero(FlyingStaminaCoolTime))
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("!!!!!!!STOP!!!!!!!!!!")));
			return;
		}

		//not flying
		if (GuardianState == EGuardianCtrlState::CtrlEnd && FlyingStaminaCoolTime >= ValidFlyingStaminaCoolTime)
		{
			//if execute flying skill, prevent change state
			if (GuardianDoSkillState != EGuardianDoSkill::None)
			{
				return;
			}

			GuardianState = EGuardianCtrlState::CtrlUp;
			StartCtrl();
		}
	}
}

void AGS_Drakhar::CtrlStop()
{
	Super::CtrlStop();
	
	if (!HasAuthority() && IsLocallyControlled())
	{
		//stop flying
		if (GuardianDoSkillState == EGuardianDoSkill::None)
		{
			GuardianState = EGuardianCtrlState::CtrlEnd;
			StopCtrl();
		}
	}
}

void AGS_Drakhar::LeftMouse()
{
	Super::LeftMouse();

	if (!HasAuthority() && IsLocallyControlled())
	{
		//flying & not using flying skill
		if (GuardianState == EGuardianCtrlState::CtrlUp && GuardianDoSkillState == EGuardianDoSkill::None)
		{
			//check earthquake skill
			GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Aiming);
		}
		
		//not flying & not using skills
		else if (GuardianDoSkillState == EGuardianDoSkill::None)
		{
			if (bCanCombo)
			{
				bCanCombo = false;
				PlayComboAttackMontage();
				ServerRPCNewComboAttack();
			}
		}
	}
}

void AGS_Drakhar::RightMouse()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		//flying & not using flying skill
		if (GuardianState == EGuardianCtrlState::CtrlUp && GuardianDoSkillState == EGuardianDoSkill::None)
		{
			//ultimate skill check
			GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Ultimate);
		}
		//not flying & not using flying skills
		else if (GuardianState == EGuardianCtrlState::CtrlEnd && GuardianDoSkillState == EGuardianDoSkill::None)
		{
			//dash skill check
			GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Moving);
		}
	}
}

void AGS_Drakhar::SetNextComboAttackSection(FName InSectionName)
{
	ComboAttackSectionName = InSectionName;
}

void AGS_Drakhar::ResetComboAttackSection()
{
	ComboAttackSectionName = DefaultComboAttackSectionName;
}

void AGS_Drakhar::PlayComboAttackMontage()
{
	PlayAnimMontage(ComboAttackMontage, 1.f, ComboAttackSectionName);
}

void AGS_Drakhar::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& PayLoad)
{
	if (NotifyName == "None")
	{
		bCanCombo = true;
		ServerRPCResetValue();
	}
}

void AGS_Drakhar::MeleeAttackCheck()
{
	if (HasAuthority())
	{
		GuardianState = EGuardianCtrlState::CtrlEnd;

		const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float MeleeAttackRange = 200.f;
		const float MeleeAttackRadius = 200.f;
		
		TSet<AGS_Character*> DamagedCharacters = DetectPlayerInRange(Start, MeleeAttackRange, MeleeAttackRadius);
		
		// 각 플레이어에게 개별적으로 데미지 적용 및 혈흔 이펙트 처리
		for (AGS_Character* DamagedCharacter : DamagedCharacters)
		{
			if (IsValid(DamagedCharacter))
			{
				UGS_StatComp* DamagedCharacterStat = DamagedCharacter->GetStatComp();
				if (IsValid(DamagedCharacterStat))
				{
					float Damage = DamagedCharacterStat->CalculateDamage(this, DamagedCharacter);
					FGS_DamageEvent DamageEvent;
					DamageEvent.HitReactType = EHitReactType::DamageOnly;
					
					float ActualDamage = DamagedCharacter->TakeDamage(Damage, DamageEvent, GetController(), this);
					
					// 실제로 데미지가 적용된 경우에만 혈흔 이펙트 재생 (기본 콤보는 1.0 스케일)
					if (ActualDamage > 0.0f)
					{
						FVector HitLocation = DamagedCharacter->GetActorLocation();
						FVector HitNormal = (HitLocation - GetActorLocation()).GetSafeNormal();
						Multicast_PlayBloodEffect(HitLocation, HitNormal, 1.0f);
					}
					
					// 히트 스톱 효과
					MulticastRPCApplyHitStop(DamagedCharacter);
					
					// 공격 성공 시 공격자에게 카메라 쉐이크 적용
					if (APlayerController* AttackerPC = Cast<APlayerController>(GetController()))
					{
						Client_PlayAttackSuccessShake(AttackerPC);
					}
					
					// 서버에서 피버 게이지 증가
					if (!GetIsFeverMode())
					{
						SetFeverGauge(10.f);
					}
					else if (GetIsFeverMode())
					{
						bIsAttckingDuringFever = true;
						ResetIsAttackingDuringFeverMode();
					}
					
					// 히트 사운드 재생
					if (AudioComponent) AudioComponent->PlayAttackHitSound();
				}
			}
		}
	}
}

void AGS_Drakhar::ComboLastAttack()
{
	if (HasAuthority())
	{
		const FVector Start = GetActorLocation();
		const float Radius = 300.f;
		const float PlusDamage = 20.f;

		TSet<AGS_Character*> DamagedPlayers = DetectPlayerInRange(Start, 200.f, Radius);
		
		// 각 플레이어에게 개별적으로 데미지 적용 및 혈흔 이펙트 처리
		for (AGS_Character* DamagedPlayer : DamagedPlayers)
		{
			if (IsValid(DamagedPlayer))
			{
				UGS_StatComp* DamagedCharacterStat = DamagedPlayer->GetStatComp();
				if (IsValid(DamagedCharacterStat))
				{
					float Damage = DamagedCharacterStat->CalculateDamage(this, DamagedPlayer);
					FGS_DamageEvent DamageEvent;
					DamageEvent.HitReactType = EHitReactType::DamageOnly;
					
					float ActualDamage = DamagedPlayer->TakeDamage(Damage + PlusDamage, DamageEvent, GetController(), this);
					
					// 실제로 데미지가 적용된 경우에만 혈흔 이펙트 재생 (마지막 공격은 더 큰 스케일)
					if (ActualDamage > 0.0f)
					{
						FVector HitLocation = DamagedPlayer->GetActorLocation();
						FVector HitNormal = (HitLocation - GetActorLocation()).GetSafeNormal();
						Multicast_PlayBloodEffect(HitLocation, HitNormal, 1.5f); // 마지막 공격은 1.5배 스케일
					}
					
					MulticastRPC_PlayAttackHitVFX(DamagedPlayer->GetActorLocation());
					if (AudioComponent) AudioComponent->PlayAttackHitSound();
					
					// 공격 성공 시 공격자에게 강한 카메라 쉐이크 적용
					if (APlayerController* AttackerPC = Cast<APlayerController>(GetController()))
					{
						FGS_CameraShakeInfo StrongAttackShake = AttackSuccessShake;
						StrongAttackShake.Intensity *= 1.8f; // 마지막 콤보는 더 강한 쉐이크
						Client_PlayAttackSuccessShakeWithInfo(AttackerPC, StrongAttackShake);
					}
				}
			}
		}
		
		if (IsFeverMode)
		{
			FeverComoLastAttack();
		}
	}	
}

void AGS_Drakhar::ServerRPCResetValue_Implementation()
{
	bCanCombo = true;
}

void AGS_Drakhar::ServerRPCNewComboAttack_Implementation()
{
	bCanCombo = false;
	MulticastRPCComboAttack();
	if (AudioComponent) AudioComponent->PlayComboAttackSound();
}

void AGS_Drakhar::MulticastRPCComboAttack_Implementation()
{
	if (!IsLocallyControlled())
	{
		PlayComboAttackMontage();
	}
}

void AGS_Drakhar::ServerRPCDoDash_Implementation(float DeltaTime)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	DashInterpAlpha += DeltaTime / DashDuration;

	DashAttackCheck();

	if (DashInterpAlpha >= 1.f)
	{
		SetActorLocation(DashEndLocation);
	}
	else
	{
		const FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, DashInterpAlpha);
		SetActorLocation(NewLocation, true);
		DashStartLocation = NewLocation;
	}

	DashDirection = GetActorForwardVector().GetSafeNormal();
}

void AGS_Drakhar::ServerRPCEndDash_Implementation()
{
	//collision setting first
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	//skill state reset
	GuardianDoSkillState = EGuardianDoSkill::None;

	// Skill Input Reset
	GetSkillComp()->ResetAllowedSkillsMask();
	
	if (DamagedCharactersFromDash.IsEmpty())
	{
		return;
	}

	for (auto const& DamagedCharacter : DamagedCharactersFromDash)
	{
		float SkillCoefficient = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Moving)->Damage;
		float RealDamage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter, SkillCoefficient);

		FGS_DamageEvent DamageEvent;
		DamageEvent.HitReactType = EHitReactType::DamageOnly; // 가드 풀리지 않도록 변경
		
		float ActualDamage = DamagedCharacter->TakeDamage(RealDamage, DamageEvent, GetController(), this);

		// 실제로 데미지가 적용된 경우에만 혈흔 이펙트 재생
		if (ActualDamage > 0.0f)
		{
			FVector HitLocation = DamagedCharacter->GetActorLocation();
			FVector HitNormal = (HitLocation - GetActorLocation()).GetSafeNormal();
			Multicast_PlayBloodEffect(HitLocation, HitNormal, 1.2f); // 대시 공격
		}

		if (IsFeverMode)
		{
			DamagedCharacter->GetDebuffComp()->ApplyDebuff(EDebuffType::Bleed, this);
		}

		MulticastRPC_PlayAttackHitVFX(DamagedCharacter->GetActorLocation());
		if (AudioComponent) AudioComponent->PlayAttackHitSound();
		
		// 공격 성공 시 공격자에게 카메라 쉐이크 적용
		if (APlayerController* AttackerPC = Cast<APlayerController>(GetController()))
		{
			FGS_CameraShakeInfo DashAttackShake = AttackSuccessShake;
			DashAttackShake.Intensity *= 1.2f; // 대시 공격은 약간 강한 쉐이크
			Client_PlayAttackSuccessShakeWithInfo(AttackerPC, DashAttackShake);
		}

		FVector DrakharPos = GetActorLocation();
		FVector DamagedPos = DamagedCharacter->GetActorLocation();
		FVector OutVector = (DamagedPos - DrakharPos);
		FVector TempVector = -OutVector.Dot(DashDirection) * DashDirection;
		FVector ResultVector = TempVector + OutVector;

		DamagedCharacter->LaunchCharacter(ResultVector.GetSafeNormal() * 10000.f, true, true);
	}
	
	DamagedCharactersFromDash.Empty();
	bCanCombo = true;

	MulticastStopWingRushVFX();
	MulticastStopDustVFX();
}

void AGS_Drakhar::ServerRPCCalculateDashLocation_Implementation()
{
	DashInterpAlpha = 0.f;
	DashStartLocation = GetActorLocation();
	DashEndLocation = DashStartLocation + GetActorForwardVector() * DashPower;

	if (AudioComponent) AudioComponent->PlayDashSkillSound();
	MulticastStartWingRushVFX();
	MulticastStartDustVFX();
}

void AGS_Drakhar::DashAttackCheck()
{
	//server
	if (HasAuthority())
	{
		const FVector Start = GetActorLocation();
		DamagedCharactersFromDash.Append(DetectPlayerInRange(Start, 10.f, 100.f));
	}
}

void AGS_Drakhar::ServerRPCEarthquakeAttackCheck_Implementation()
{
	MulticastRPC_OnEarthquakeStart();
	if (AudioComponent) AudioComponent->PlayEarthquakeSkillSound();

	const FVector Start = GetActorLocation() + 100.f;
	TSet<AGS_Character*> EarthquakeDamagedCharacters = DetectPlayerInRange(Start, 200.f, EarthquakeRadius);

	//Spawn Skill Effect
	FVector SpawnLocation = Start + GetActorForwardVector() * 300.f;
	AGS_EarthquakeEffect* GC_Earthquake = GetWorld()->SpawnActor<AGS_EarthquakeEffect>(GC_EarthquakeEffect, SpawnLocation + FVector(0.f,0.f,-200.f), GetActorRotation());
	GC_Earthquake->SetOwner(this);
	GC_Earthquake->MulticastTriggerDestruction(SpawnLocation, EarthquakeRadius, 3000.f);
	
	for (const auto& DamagedCharacter : EarthquakeDamagedCharacters)
	{
		float SkillCoefficient = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Aiming)->Damage;
		float RealDamage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter, SkillCoefficient);

		FGS_DamageEvent DamageEvent;
		if (IsValid(DamagedCharacter))
		{
			DamageEvent.HitReactType = EHitReactType::DamageOnly; // 가드 풀리지 않도록 변경
			
			// 실제 데미지 적용
			float ActualDamage = DamagedCharacter->TakeDamage(RealDamage, DamageEvent, GetController(), this);
			
			// 실제로 데미지가 적용된 경우에만 혈흔 이펙트 재생
			if (ActualDamage > 0.0f)
			{
				FVector HitLocation = DamagedCharacter->GetActorLocation();
				FVector HitNormal = FVector::UpVector; // 어스퀘이크는 위쪽에서 아래로
				Multicast_PlayBloodEffect(HitLocation, HitNormal, 1.3f);
			}
			
			if (IsFeverMode)
			{
				DamagedCharacter->GetDebuffComp()->ApplyDebuff(EDebuffType::Bleed, this);
				MulticastPlayFeverEarthquakeImpactVFX(DamagedCharacter->GetActorLocation());
			}
			else
			{
				MulticastPlayEarthquakeImpactVFX(DamagedCharacter->GetActorLocation());
			}
			
			// === 어스퀘이크 스킬 히트 사운드 재생 ===
			MulticastRPC_PlayAttackHitVFX(DamagedCharacter->GetActorLocation());
			if (AudioComponent) AudioComponent->PlayAttackHitSound();

			FVector DrakharLocation = GetActorLocation();
			FVector DamagedLocation = DamagedCharacter->GetActorLocation();
			FVector LaunchVector = (DamagedLocation - DrakharLocation).GetSafeNormal();
			//Guardian 쪽으로 당겨오기
			DamagedCharacter->LaunchCharacter(-LaunchVector * EarthquakePower + FVector(0.f, 0.f, 500.f), false, false);
		}
	}
}

void AGS_Drakhar::ServerRPCStartCtrl_Implementation()
{
	GuardianState = EGuardianCtrlState::CtrlUp;
	MoveSpeed = SpeedUpMoveSpeed;

	if (isStartCoolTime)
	{
		SafeClearTimer(FlyingEndStaminaCoolTimeHandler);
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(FlyingStartStaminaCoolTimeHandler, this, &AGS_Drakhar::StartFlyingStaminaTimer, 1.f, true);
		}
		isStartCoolTime = false;
	}
	
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AGS_Drakhar::ServerRPCStopCtrl_Implementation()
{
	GuardianState = EGuardianCtrlState::CtrlEnd;
	GuardianDoSkillState = EGuardianDoSkill::None;
	
	MoveSpeed = NormalMoveSpeed;

	isStartCoolTime = true;
	SafeClearTimer(FlyingStartStaminaCoolTimeHandler);
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(FlyingEndStaminaCoolTimeHandler, this, &AGS_Drakhar::EndFlyingStaminaTimer, 1.f, true);
	}
	
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	GetSkillComp()->ResetAllowedSkillsMask();
}

void AGS_Drakhar::StartCtrl()
{
	if (!HasAuthority())
	{
		ServerRPCStartCtrl();
		GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Ready);
		
		TargetSpringArmLength = 800.f;
		bIsFlying = true;
	}
}

void AGS_Drakhar::StopCtrl()
{
	//[Client] reset flying skill values
	if (!HasAuthority())
	{
		ServerRPCStopCtrl();
		GetSkillComp()->Server_TrySkillCanceledByDebuff(ESkillSlot::Ready);
		
		TargetSpringArmLength = 500.f;
		bIsFlying = true;
		bCanCombo = true;
	}
}

void AGS_Drakhar::ServerRPCSpawnDraconicFury_Implementation()
{
	// 월드 검증 및 생명주기 체크
	if (!IsWorldContextValid() || !IsValid(this))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 사운드 재생 (월드 검증 후)
	if (AudioComponent) AudioComponent->PlayDraconicFurySkillSound();

	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (IsFeverMode)
	{
		// 피버 모드: 드라카의 현재 위치 기준으로 앞쪽에 투사체 소환
		FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 200.f + FVector(0.f, 0.f, 600.f);
		FRotator SpawnRotation = GetActorRotation();
		float RandomPitch = FMath::FRandRange(-35.f, -30.f);
		SpawnRotation.Pitch += RandomPitch;

		AGS_DrakharProjectile* DrakharProjectile = World->SpawnActor<AGS_DrakharProjectile>(FeverDraconicProjectile, SpawnLocation, SpawnRotation, Params);

		if (DrakharProjectile && FeverDraconicFuryIndicatorVFX)
		{
			// 피버 모드 인디케이터 VFX 설정 (더 큰 반경)
			float FeverIndicatorRadius = 250.0f * 1.5f; // 피버 모드는 1.5배 반경
			DrakharProjectile->SetIndicatorVFX(FeverDraconicFuryIndicatorVFX, FeverIndicatorRadius);
		}
	}
	else
	{
		// 일반 모드: 드라카의 현재 위치 기준으로 랜덤 위치에 투사체 소환
		FVector BaseLocation = GetActorLocation();
		FVector RandomOffset = GetActorForwardVector() * 200.f + FVector(
			FMath::FRandRange(-300.f, 300.f),
			FMath::FRandRange(-300.f, 300.f),
			FMath::FRandRange(500.f, 600.f)
		);

		FVector SpawnLocation = BaseLocation + RandomOffset;
		FRotator SpawnRotation = GetActorRotation();
		float RandomPitch = FMath::FRandRange(-35.f, -30.f);
		SpawnRotation.Pitch += RandomPitch;

		AGS_DrakharProjectile* DrakharProjectile = World->SpawnActor<AGS_DrakharProjectile>(
			DraconicProjectile,
			SpawnLocation,
			SpawnRotation,
			Params
		);

		if (DrakharProjectile)
		{
			if (DraconicFuryIndicatorVFX)
			{
				float NormalIndicatorRadius = 250.0f; // 일반 모드 반경
				DrakharProjectile->SetIndicatorVFX(DraconicFuryIndicatorVFX, NormalIndicatorRadius);
			}

			if (AudioComponent) AudioComponent->PlayDraconicProjectileSound(DrakharProjectile->GetActorLocation());
		}
	}
}

void AGS_Drakhar::ServerRPC_BeginDraconicFury_Implementation()
{
	// 월드 검증 및 생명주기 체크
	if (!IsWorldContextValid() || !IsValid(this))
	{
		return;
	}

	if (GetSkillComp()->IsSkillActive(ESkillSlot::Ultimate))
	{
		return;
	}

	GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Ultimate);
	MulticastRPC_OnUltimateStart();

	// 타이머 설정 (레벨 전환 시 크래시 방지) - 멤버 변수 사용
	UWorld* World = GetWorld();
	if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
	{
		// 기존 타이머가 있다면 먼저 정리
		SafeClearTimer(DraconicAttackTimer);

		World->GetTimerManager().SetTimer(
			DraconicAttackTimer,  // 멤버 변수 사용!
			this,
			&AGS_Drakhar::EndDraconicFury,
			DraconicAttackPersistenceTime,
			false);
	}
}

void AGS_Drakhar::EndDraconicFury()
{
	// 월드 검증 및 생명주기 체크
	if (!IsWorldContextValid() || !IsValid(this))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Draconic Fury Skill End"));
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT] Draconic Fury Skill End")));

	// 컴포넌트 안전성 체크
	if (UGS_SkillComp* Skill = GetSkillComp())
	{
		Skill->Server_TrySkillCanceledByDebuff(ESkillSlot::Ready);
	}

	GuardianState = EGuardianCtrlState::CtrlEnd;

	MoveSpeed = NormalMoveSpeed;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeed;
	}
}

void AGS_Drakhar::SetFeverGaugeWidget(UGS_DrakharFeverGauge* InDrakharFeverGaugeWidget)
{
	UGS_DrakharFeverGauge* DrakharFeverGaugeWidget = Cast<UGS_DrakharFeverGauge>(InDrakharFeverGaugeWidget);
	if (IsValid(DrakharFeverGaugeWidget))
	{
		//client
		DrakharFeverGaugeWidget->InitializeGauge(GetCurrentFeverGauge());
		OnCurrentFeverGaugeChanged.AddUObject(DrakharFeverGaugeWidget ,&UGS_DrakharFeverGauge::OnCurrentFeverGaugeChanged);
	}
}

void AGS_Drakhar::SetFeverGauge(float InValue)
{	
	//server
	if (HasAuthority())
	{
		CurrentFeverGauge += InValue;

		//Stop Fever Mode
		if (CurrentFeverGauge < KINDA_SMALL_NUMBER)
		{
			CurrentFeverGauge = 0.f;

			SafeClearTimer(FeverTimer);
			if (IsFeverMode)
			{
				FGS_StatRow Stat;
				Stat.ATK = 50.f;
				GetStatComp()->ResetStat(Stat);
				MulticastRPC_OnFeverModeEnd();
				MulticastPlayFeverModeEndEffects();  // 피버 모드 종료 사운드 & 카메라 효과
			}

			IsFeverMode = false;
		}

		//Start Fever Mode
		if (CurrentFeverGauge >= MaxFeverGauge)
		{
			CurrentFeverGauge = MaxFeverGauge;
			IsFeverMode = true;
			StartFeverMode();
		}
				
		if (CurrentFeverGauge > 0.f)
		{
			DecreaseFeverGauge();
		}
		
		OnRep_FeverGauge();
	}
}

void AGS_Drakhar::ResetIsAttackingDuringFeverMode()
{
	SafeClearTimer(ResetAttackTimer);
	UWorld* TimerWorld = GetWorld();
	if (TimerWorld && TimerWorld->IsValidLowLevel() && !TimerWorld->bIsTearingDown)
	{
		TimerWorld->GetTimerManager().SetTimer(ResetAttackTimer, this, &AGS_Drakhar::StartIsAttackingTimer, 3.f, false);
	}
}

void AGS_Drakhar::StartIsAttackingTimer()
{
	bIsAttckingDuringFever = false;
}

void AGS_Drakhar::MulticastRPCFeverMontagePlay_Implementation()
{
	PlayAnimMontage(FeverOnMontage, 1.f);
}

void AGS_Drakhar::FeverComoLastAttack()
{
	//server
	if (HasAuthority())
	{
		const FVector ActorLocation = GetActorLocation() + FVector(0.f, 0.f, 20.f);
		const FVector ForwardVector = GetActorForwardVector();
		const FVector RightVector = GetActorRightVector();

		const FVector CenterPillarLocation = ActorLocation + (ForwardVector * PillarForwardOffset);
		const FVector LeftPillarLocation = CenterPillarLocation - (RightVector * PillarSideSpacing);
		const FVector RightPillarLocation = CenterPillarLocation + (RightVector * PillarSideSpacing);

		TArray<FVector> PillarLocations;
		PillarLocations.Add(LeftPillarLocation);
		PillarLocations.Add(CenterPillarLocation);
		PillarLocations.Add(RightPillarLocation);

		TSet<AGS_Character*> DamagedSeekers;
		FCollisionQueryParams Params(NAME_None, false, this);

		TArray<FHitResult> OutHitResults;

		for (const FVector& PillarLocation : PillarLocations)
		{
			DamagedSeekers.Append(DetectPlayerInRange(PillarLocation, 0.f, PillarRadius));
		}
		
		// 각 플레이어에게 개별적으로 데미지 적용 및 혈흔 이펙트 처리
		for (const auto& DamagedSeeker : DamagedSeekers)
		{
			if (IsValid(DamagedSeeker))
			{
				// 데미지 계산 및 적용
				UGS_StatComp* DamagedCharacterStat = DamagedSeeker->GetStatComp();
				if (IsValid(DamagedCharacterStat))
				{
					float Damage = DamagedCharacterStat->CalculateDamage(this, DamagedSeeker);
					FGS_DamageEvent DamageEvent;
					DamageEvent.HitReactType = EHitReactType::DamageOnly;
					
					float ActualDamage = DamagedSeeker->TakeDamage(Damage + 20.f, DamageEvent, GetController(), this);
					
					// 실제로 데미지가 적용된 경우에만 혈흔 이펙트 재생
					if (ActualDamage > 0.0f)
					{
						FVector HitLocation = DamagedSeeker->GetActorLocation();
						FVector HitNormal = FVector::UpVector; // 피버 콤보는 위에서 아래로
						Multicast_PlayBloodEffect(HitLocation, HitNormal, 1.4f);
					}
					
					MulticastRPC_PlayAttackHitVFX(DamagedSeeker->GetActorLocation());
					DamagedSeeker->LaunchCharacter(FVector(0.f, 0.f, 500.f), true, true);
				}
			}
		}

		// 피버 모드 콤보 피니셔 사운드 재생 (타격 소리와 함께)
		// RPC 호출 제한을 피하기 위해 약간 지연 후 사운드 재생
		FTimerHandle ComboFinisherSoundTimer;
		GetWorld()->GetTimerManager().SetTimer(
			ComboFinisherSoundTimer,
			this,
			&AGS_Drakhar::PlayDelayedComboFinisherSounds,
			0.125f, // 0.15초 지연 (RPC 제한 0.1초보다 길게)
			false
		);
	}
}

void AGS_Drakhar::PlayDelayedComboFinisherSounds()
{
	if (AudioComponent)
	{
		AudioComponent->PlayComboFinisherSound();
		AudioComponent->PlayAttackHitSound();
	}
}

void AGS_Drakhar::StartFeverMode()
{
	//server
	FGS_StatRow Stat;
	Stat.ATK = 50.f;

	GetStatComp()->ChangeStat(Stat);
	MulticastRPCFeverMontagePlay();
	MulticastRPC_OnFeverModeStart();
	
	if (AudioComponent)
	{
		// 피버모드 시작 사운드 즉시 재생 (bForcePlay = true로 RPC 제한 무시)
		AudioComponent->PlayFeverModeStartSound(true);

		// FeverModeStateSound는 0.2초 후에 재생 (RPC 간격 제한 확실히 회피)
		UWorld* World = GetWorld();
		if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			SafeClearTimer(FeverStateSoundDelayTimer);
			World->GetTimerManager().SetTimer(
				FeverStateSoundDelayTimer,
				this,
				&AGS_Drakhar::PlayFeverModeStateSoundDelayed,
				0.2f,
				false
			);
		}
	}
}

void AGS_Drakhar::DecreaseFeverGauge()
{
	UWorld* FeverWorld = GetWorld();
	if (FeverWorld && FeverWorld->IsValidLowLevel() && !FeverWorld->bIsTearingDown)
	{
		FeverWorld->GetTimerManager().SetTimer(FeverTimer, this, &AGS_Drakhar::MinusFeverGaugeValue, 1.f, true);
	}
}

void AGS_Drakhar::MinusFeverGaugeValue()
{
	if (IsFeverMode)
	{
		//공격 유지 안된 경우
		if (!bIsAttckingDuringFever)
		{
			SetFeverGauge(-5.f);
		}
	}
	else
	{
		SetFeverGauge(-1.f);
	}
}

void AGS_Drakhar::BeginHealRegeneration()
{
	bIsDamaged = false;
	
	//health regeneration start
	UWorld* RegenWorld = GetWorld();
	if (RegenWorld && RegenWorld->IsValidLowLevel() && !RegenWorld->bIsTearingDown)
	{
		RegenWorld->GetTimerManager().SetTimer(HealthRegenTimer, this, &AGS_Drakhar::HealRegeneration, 1.f, true);
	}
}

void AGS_Drakhar::HealRegeneration()
{
	if (!bIsDamaged)
	{
		float CurrentHealth = GetStatComp()->GetCurrentHealth();
		GetStatComp()->SetCurrentHealth(CurrentHealth + 2.f, true);
	}
}

void AGS_Drakhar::StopHealRegeneration()
{
	SafeClearTimer(HealthRegenTimer);
}

void AGS_Drakhar::SetStaminaGaugeWidget(UGS_DrakharStaminaGauge* InDrakharStaminaGaugeWidget)
{
	UGS_DrakharStaminaGauge* DrakharStaminaGaugeWidget = Cast<UGS_DrakharStaminaGauge>(InDrakharStaminaGaugeWidget);
	if (IsValid(DrakharStaminaGaugeWidget))
	{
		//client
		DrakharStaminaGaugeWidget->InitializeGauge(GetCurrentStaminaGauge());
		OnCurrentStaminaGaugeChanged.AddUObject(DrakharStaminaGaugeWidget, &UGS_DrakharStaminaGauge::OnCurrentStaminaGaugeChanged);
	}
}

void AGS_Drakhar::StartFlyingStaminaTimer()
{
	SafeClearTimer(FlyingEndStaminaCoolTimeHandler);
	
	FlyingStaminaCoolTime -= 1.f;
	
	if (FlyingStaminaCoolTime <= 0.f)
	{
		FlyingStaminaCoolTime = 0.f;
		
		// 스테미나가 0이 되면 떨어지는 소리 재생
		if (HasAuthority() && AudioComponent)
		{
			AudioComponent->PlayLandingSound();
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("start flying stamina %f"), FlyingStaminaCoolTime);
}

void AGS_Drakhar::EndFlyingStaminaTimer()
{
	SafeClearTimer(FlyingStartStaminaCoolTimeHandler);
	
	FlyingStaminaCoolTime += 1.f;

	if (FlyingStaminaCoolTime >= MaxFlyingStaminaCoolTime)
	{
		FlyingStaminaCoolTime = MaxFlyingStaminaCoolTime;
	}
	//OnCurrentStaminaGaugeChanged.Broadcast(FlyingStaminaCoolTime);

	//UE_LOG(LogTemp, Error, TEXT("end flying stamina %f"), FlyingStaminaCoolTime);
}

void AGS_Drakhar::GenerateDraconicFuryTargets()
{
	// 피버 모드일 경우 피버 모드 위치 생성
	if (IsFeverMode)
	{
		FeverModeDraconicFurySpawnLocation = GetActorLocation() + GetActorForwardVector() * 200.f + FVector(0.f, 0.f, 600.f);
	}
	// 일반 모드일 경우 5개의 랜덤 위치 생성
	else
	{
		GetRandomDraconicFuryTarget();
	}
}

void AGS_Drakhar::GetRandomDraconicFuryTarget()
{
	DraconicFuryTargetArray.Empty();

	for (int32 i = 0; i < 5; ++i)
	{
		FVector StartLocation = GetActorLocation();
		FVector Offset = GetActorForwardVector() * 200.f + FVector(FMath::FRandRange(-300.f, 300.f),
		                                                           FMath::FRandRange(-300.f, 300.f),
		                                                           FMath::FRandRange(500.f, 600.f));

		StartLocation += Offset;

		FRotator StartRotation = GetActorRotation();
		float RandomPitch = FMath::FRandRange(-35.f, -30.f);
		StartRotation.Pitch += RandomPitch;

		FTransform StartTransform = FTransform(StartRotation, StartLocation);
		DraconicFuryTargetArray.Add(StartTransform);
	}
}

void AGS_Drakhar::OnRep_FeverGauge()
{
	OnCurrentFeverGaugeChanged.Broadcast(CurrentFeverGauge);
}

void AGS_Drakhar::OnAttackHit(AGS_Character* HitCharacter)
{
	// 공격 히트 사운드 재생
	if (AudioComponent)
	{
		AudioComponent->PlayAttackHitSound();
	}
}

void AGS_Drakhar::OnFeverGaugeUpdate(float DeltaGauge)
{
	// 피버 게이지 업데이트 로직
	if (!GetIsFeverMode())
	{
		SetFeverGauge(DeltaGauge);
	}
	else if (GetIsFeverMode())
	{
		bIsAttckingDuringFever = true;
		ResetIsAttackingDuringFeverMode();
	}
}

void AGS_Drakhar::OnQuitSkill()
{
	// 스킬 종료 시 값 리셋
	ServerRPCResetValue();
}

void AGS_Drakhar::MulticastPlayFeverModeEndEffects_Implementation()
{
	// 피버 모드 스테이트 사운드 중지
	if (AudioComponent && AudioComponent->GetOwner())
	{
		// Playing ID가 유효하면 FAkAudioDevice를 통해 중지
		int32& FeverModeStateSoundPlayingID = AudioComponent->GetFeverModeStateSoundPlayingID();
		if (FeverModeStateSoundPlayingID != AK_INVALID_PLAYING_ID)
		{
			FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
			if (AudioDevice != nullptr)
			{
				AudioDevice->StopPlayingID(FeverModeStateSoundPlayingID, AudioComponent->GetFeverModeStateFadeOutDuration());
				FeverModeStateSoundPlayingID = AK_INVALID_PLAYING_ID;
			}
		}
		
		// 피버 모드 종료 사운드 재생
		if (FeverModeEndSoundEvent)
		{
			UAkGameplayStatics::PostEvent(FeverModeEndSoundEvent, this, 0, FOnAkPostEventCallback());
		}
	}

	// 카메라 쉐이크 효과 (피버 모드 종료시 쉐이크)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FGS_CameraShakeInfo EndFeverShake = AttackSuccessShake;
		EndFeverShake.Intensity *= 0.5f;
		Client_PlayAttackSuccessShakeWithInfo(PC, EndFeverShake);
	}
}

/*
void AGS_Drakhar::MulticastPlayFeverModeEndVFX_Implementation()
{
	// 피버 모드 종료 VFX 재생
	if (FeverModeEndVFX && GetWorld())
	{
		// 캐릭터 위치에 VFX 스폰
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);

		// 나이아가라 시스템 스폰
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			FeverModeEndVFX,
			SpawnLocation,
			GetActorRotation(),
			FVector(1.0f, 1.0f, 1.0f), // 기본 스케일
			true, // 절대 스케일 사용
			true, // 절대 회전 사용
			ENCPoolMethod::None, // 풀링 사용 안함
			true  // 월드 공간에서 자동 파괴
		);

		// 여러 위치에 스폰하여 화려한 효과 연출
		for (int32 i = 0; i < 8; ++i)
		{
			float Angle = (i / 8.0f) * 2.0f * PI;
			FVector Offset = FVector(FMath::Cos(Angle) * 150.f, FMath::Sin(Angle) * 150.f, 50.f);
			FVector ParticleLocation = GetActorLocation() + Offset;

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				FeverModeEndVFX,
				ParticleLocation,
				FRotator::ZeroRotator,
				FVector(0.3f, 0.3f, 0.3f),
				true,
				true,
				ENCPoolMethod::None,
				true
			);
		}
	}

	ApplyFeverModeEndCameraEffect();
}
*/

void AGS_Drakhar::ServerRPCShootEnergy_Implementation()
{
	// TODO: 투사체 발사 로직 구현
}

// === 나이아가라 VFX 함수 구현 ===
void AGS_Drakhar::MulticastStartWingRushVFX_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->StartWingRushVFX();
}

void AGS_Drakhar::MulticastStopWingRushVFX_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->StopWingRushVFX();
}

void AGS_Drakhar::MulticastStartDustVFX_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->StartDustVFX();
}

void AGS_Drakhar::MulticastStopDustVFX_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->StopDustVFX();
}

// === 어스퀘이크 지면 균열 VFX 제어 함수 ===
void AGS_Drakhar::MulticastStartGroundCrackVFX_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->StartGroundCrackVFX();
}

void AGS_Drakhar::MulticastStopGroundCrackVFX_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->StopGroundCrackVFX();
}

// === 어스퀘이크 먼지 구름 VFX 제어 함수 ===
void AGS_Drakhar::MulticastStartDustCloudVFX_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->StartDustCloudVFX();
}

// === DraconicFury 투사체 충돌 처리 함수 구현 ===

void AGS_Drakhar::HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter)
{
	// 로컬 재생 (모든 클라이언트에서 OnHit이 호출되므로 RPC 불필요)
	if (DrakharVFXComponent) DrakharVFXComponent->HandleDraconicProjectileImpact(ImpactLocation, ImpactNormal, bHitCharacter);
	if (AudioComponent) AudioComponent->PlayDraconicProjectileImpactSoundLocal(ImpactLocation, bHitCharacter);
}

void AGS_Drakhar::MulticastPlayFeverEarthquakeImpactVFX_Implementation(const FVector& ImpactLocation)
{
	if (DrakharVFXComponent) DrakharVFXComponent->PlayFeverEarthquakeImpactVFX(ImpactLocation);
}

void AGS_Drakhar::MulticastRPC_OnFlyStart_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->OnFlyStart();
}

void AGS_Drakhar::MulticastRPC_OnFlyEnd_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->OnFlyEnd();
}

void AGS_Drakhar::MulticastRPC_OnUltimateStart_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->OnUltimateStart();
}

void AGS_Drakhar::MulticastRPC_OnEarthquakeStart_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->OnEarthquakeStart();
}

void AGS_Drakhar::MulticastRPC_OnFeverModeStart_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->OnFeverModeChanged(true);
	BP_OnFeverModeStart();
}

void AGS_Drakhar::MulticastRPC_OnFeverModeEnd_Implementation()
{
	if (DrakharVFXComponent) DrakharVFXComponent->OnFeverModeChanged(false);
	BP_OnFeverModeEnd();
}

void AGS_Drakhar::OnRep_IsFeverMode()
{
	if (DrakharVFXComponent) DrakharVFXComponent->OnFeverModeChanged(IsFeverMode);

	// 클라이언트에서도 블루프린트 이벤트 호출
	if (IsFeverMode)
	{
		BP_OnFeverModeStart();
	}
	else
	{
		BP_OnFeverModeEnd();
	}
}

void AGS_Drakhar::OnRep_FlyingStaminaCoolTime()
{
	if (FMath::IsNearlyZero(FlyingStaminaCoolTime))
	{
		StopCtrl();
	}
	if (FlyingStaminaCoolTime == MaxFlyingStaminaCoolTime)
	{
		SafeClearTimer(FlyingStartStaminaCoolTimeHandler);
		SafeClearTimer(FlyingEndStaminaCoolTimeHandler);
	}
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%f"),FlyingStaminaCoolTime));
	OnCurrentStaminaGaugeChanged.Broadcast(FlyingStaminaCoolTime);
}

void AGS_Drakhar::MulticastRPC_PlayAttackHitVFX_Implementation(FVector ImpactPoint)
{
	if (DrakharVFXComponent) DrakharVFXComponent->PlayAttackHitVFX(ImpactPoint);
}

void AGS_Drakhar::MulticastPlayEarthquakeImpactVFX_Implementation(const FVector& ImpactLocation)
{
	if (DrakharVFXComponent) DrakharVFXComponent->PlayEarthquakeImpactVFX(ImpactLocation);
}

void AGS_Drakhar::MulticastStopDustCloudVFX_Implementation()
{
	if(DrakharVFXComponent) DrakharVFXComponent->StopDustCloudVFX();
}

void AGS_Drakhar::Multicast_PlayBloodEffect_Implementation(FVector HitLocation, FVector HitNormal, float Scale)
{
	UGS_VFX_FunctionLibrary::PlayBloodEffect(this, BloodEffectSystem, HitLocation, FRotationMatrix::MakeFromZ(HitNormal).Rotator(), Scale);
}

// === 월드 컨텍스트 검증 함수 ===
bool AGS_Drakhar::IsWorldContextValid() const
{
	UWorld* World = GetWorld();
	return World &&
		   World->IsValidLowLevel() &&
		   !World->bIsTearingDown &&
		   IsValid(World) &&
		   IsValid(this);
}

// === 타이머 정리 함수 ===
void AGS_Drakhar::SafeClearTimer(FTimerHandle& TimerHandle)
{
	if (TimerHandle.IsValid())
	{
		UWorld* World = GetWorld();
		if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
		TimerHandle.Invalidate();
	}
}

// === FeverModeStateSound 딜레이 재생 콜백 ===
void AGS_Drakhar::PlayFeverModeStateSoundDelayed()
{
	if (!IsValid(this)) return;

	if (AudioComponent && IsFeverMode)
	{
		// bForcePlay = true로 RPC 제한 무시하여 확실하게 재생
		AudioComponent->PlayFeverModeStateSound(true);
	}
}

// === 피버 모드 종료 카메라 효과 함수 ===

// 카메라 검증 유틸리티 함수
bool AGS_Drakhar::ValidateCameraEffect(APlayerController*& OutPC) const
{
	if (!IsLocallyControlled())
	{
		return false;
	}

	OutPC = Cast<APlayerController>(GetController());
	if (!OutPC || !OutPC->PlayerCameraManager)
	{
		return false;
	}

	return true;
}

void AGS_Drakhar::ApplyFeverModeEndCameraEffect()
{
	APlayerController* PC = nullptr;
	if (!ValidateCameraEffect(PC))
	{
		return;
	}

	// 현재 카메라 상태 저장
	OriginalFOV = PC->PlayerCameraManager->GetFOVAngle();
	if (SpringArmComp)
	{
		OriginalArmLength = SpringArmComp->TargetArmLength;
	}

	// 줌인 단계 타겟 값 설정
	TargetFOV = OriginalFOV * FeverEndZoomInFOVMultiplier;
	TargetArmLength = OriginalArmLength * FeverEndZoomInArmMultiplier;

	// 카메라 효과 시작 - 줌인 단계
	CurrentCameraEffectPhase = ECameraEffectPhase::ZoomIn;

	// 기존 타이머 정리
	SafeClearTimer(CameraZoomTimer);

	// 통합 카메라 업데이트 타이머 시작
	UWorld* World = GetWorld();
	if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
	{
		World->GetTimerManager().SetTimer(
			CameraZoomTimer,
			this,
			&AGS_Drakhar::UpdateCameraEffect,
			CAMERA_UPDATE_INTERVAL,
			true
		);
	}
}

// 통합 카메라 업데이트 함수
void AGS_Drakhar::UpdateCameraEffect()
{
	APlayerController* PC = nullptr;
	if (!ValidateCameraEffect(PC))
	{
		SafeClearTimer(CameraZoomTimer);
		CurrentCameraEffectPhase = ECameraEffectPhase::None;
		return;
	}

	// 현재 단계에 따라 적절한 업데이트 함수 호출
	switch (CurrentCameraEffectPhase)
	{
	case ECameraEffectPhase::ZoomIn:
		UpdateCameraZoomIn();
		break;

	case ECameraEffectPhase::ZoomOut:
		UpdateCameraZoomOut();
		break;

	case ECameraEffectPhase::Restore:
		UpdateCameraRestore();
		break;

	default:
		SafeClearTimer(CameraZoomTimer);
		CurrentCameraEffectPhase = ECameraEffectPhase::None;
		break;
	}
}

// 카메라 줌인 단계 업데이트
void AGS_Drakhar::UpdateCameraZoomIn()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	// FOV 줌인
	float CurrentFOV = PC->PlayerCameraManager->GetFOVAngle();
	float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, CAMERA_UPDATE_INTERVAL, FeverEndZoomInSpeed);
	PC->PlayerCameraManager->SetFOV(NewFOV);

	// SpringArm 줌인
	if (SpringArmComp)
	{
		float CurrentArmLength = SpringArmComp->TargetArmLength;
		float NewArmLength = FMath::FInterpTo(CurrentArmLength, TargetArmLength, CAMERA_UPDATE_INTERVAL, FeverEndZoomInSpeed);
		SpringArmComp->TargetArmLength = NewArmLength;

		// 둘 다 타겟에 도달하면 다음 단계로
		if (FMath::IsNearlyEqual(NewFOV, TargetFOV, FOV_TOLERANCE) &&
			FMath::IsNearlyEqual(NewArmLength, TargetArmLength, ARM_LENGTH_TOLERANCE))
		{
			TransitionToNextCameraPhase();
		}
	}
}

// 카메라 줌아웃 단계 업데이트 ("쾅" 효과)
void AGS_Drakhar::UpdateCameraZoomOut()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	// FOV 줌아웃
	float CurrentFOV = PC->PlayerCameraManager->GetFOVAngle();
	float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, CAMERA_UPDATE_INTERVAL, FeverEndZoomOutSpeed);
	PC->PlayerCameraManager->SetFOV(NewFOV);

	// SpringArm 줌아웃
	if (SpringArmComp)
	{
		float CurrentArmLength = SpringArmComp->TargetArmLength;
		float NewArmLength = FMath::FInterpTo(CurrentArmLength, TargetArmLength, CAMERA_UPDATE_INTERVAL, FeverEndZoomOutSpeed);
		SpringArmComp->TargetArmLength = NewArmLength;

		// 둘 다 타겟에 도달하면 다음 단계로
		if (FMath::IsNearlyEqual(NewFOV, TargetFOV, FOV_TOLERANCE) &&
			FMath::IsNearlyEqual(NewArmLength, TargetArmLength, ARM_LENGTH_TOLERANCE))
		{
			TransitionToNextCameraPhase();
		}
	}
}

// 카메라 원래 상태로 복귀
void AGS_Drakhar::UpdateCameraRestore()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	// FOV 복귀
	float CurrentFOV = PC->PlayerCameraManager->GetFOVAngle();
	float NewFOV = FMath::FInterpTo(CurrentFOV, OriginalFOV, CAMERA_UPDATE_INTERVAL, FeverEndCameraRestoreSpeed);
	PC->PlayerCameraManager->SetFOV(NewFOV);

	bool bFOVCompleted = false;
	bool bArmCompleted = false;

	if (FMath::IsNearlyEqual(NewFOV, OriginalFOV, FINAL_FOV_TOLERANCE))
	{
		PC->PlayerCameraManager->SetFOV(OriginalFOV);
		bFOVCompleted = true;
	}

	// SpringArm 복귀
	if (SpringArmComp)
	{
		float CurrentArmLength = SpringArmComp->TargetArmLength;
		float NewArmLength = FMath::FInterpTo(CurrentArmLength, OriginalArmLength, CAMERA_UPDATE_INTERVAL, FeverEndCameraRestoreSpeed);
		SpringArmComp->TargetArmLength = NewArmLength;

		if (FMath::IsNearlyEqual(NewArmLength, OriginalArmLength, FINAL_ARM_LENGTH_TOLERANCE))
		{
			SpringArmComp->TargetArmLength = OriginalArmLength;
			bArmCompleted = true;
		}
	}

	// 모두 완료되면 효과 종료
	if (bFOVCompleted && bArmCompleted)
	{
		SafeClearTimer(CameraZoomTimer);
		CurrentCameraEffectPhase = ECameraEffectPhase::None;
	}
}

// 다음 카메라 효과 단계로 전환
void AGS_Drakhar::TransitionToNextCameraPhase()
{
	switch (CurrentCameraEffectPhase)
	{
	case ECameraEffectPhase::ZoomIn:
		// 줌인 완료 -> 줌아웃 단계로
		CurrentCameraEffectPhase = ECameraEffectPhase::ZoomOut;
		TargetFOV = OriginalFOV * FeverEndZoomOutFOVMultiplier;
		TargetArmLength = OriginalArmLength * FeverEndZoomOutArmMultiplier;
		break;

	case ECameraEffectPhase::ZoomOut:
		// 줌아웃 완료 -> 복귀 단계로
		CurrentCameraEffectPhase = ECameraEffectPhase::Restore;
		TargetFOV = OriginalFOV;
		TargetArmLength = OriginalArmLength;
		break;

	default:
		CurrentCameraEffectPhase = ECameraEffectPhase::None;
		break;
	}
}
