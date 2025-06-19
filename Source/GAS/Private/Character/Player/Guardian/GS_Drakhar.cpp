#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Components/CapsuleComponent.h"
#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/ArrowComponent.h"
#include "Character/Component/GS_CameraShakeComponent.h"
#include "Character/Component/GS_DebuffComp.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Character/GS_DrakharFeverGauge.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = true;
	
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
	bClientCanCombo = true;

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
	ClientGuardianState = EGuardianState::CtrlSkillEnd;

	//boss monster tag for user widget
	Tags.Add("Guardian");

	//fever mode
	MaxFeverGauge = 100.f;
	CurrentFeverGauge = 0.f;

	//team id
	TeamId = FGenericTeamId(0);

	//spring arm data for flying
	DefaultSpringArmLength = SpringArmComp->TargetArmLength;
	TargetSpringArmLength = 800.f;

	// === Wwise 사운드 이벤트 초기화 ===
	ComboAttackSoundEvent = nullptr;
	DashSkillSoundEvent = nullptr;
	EarthquakeSkillSoundEvent = nullptr;
	DraconicFurySkillSoundEvent = nullptr;
	DraconicProjectileSoundEvent = nullptr;

	// === DraconicFury 충돌 사운드 이벤트 초기화 ===
	DraconicProjectileImpactSoundEvent = nullptr;
	DraconicProjectileExplosionSoundEvent = nullptr;

	// 사운드 중복 재생 방지 초기화
	bDraconicFurySoundPlayed = false;

	// === 날기 사운드 이벤트 초기화 ===
	FlyStartSoundEvent = nullptr;
	FlyEndSoundEvent = nullptr;

	// === 히트 사운드 이벤트 초기화 ===
	AttackHitSoundEvent = nullptr;

	// === 피버모드 사운드 이벤트 초기화 ===
	FeverModeStartSoundEvent = nullptr;

	// AkComponent 추가
	if (!FindComponentByClass<UAkComponent>())
	{
		UAkComponent* AkComp = CreateDefaultSubobject<UAkComponent>(TEXT("AkAudioComponent"));
		if (AkComp)
		{
			AkComp->SetupAttachment(RootComponent);
		}
	}

	WingRushRibbonVFX = nullptr;
	ActiveWingRushVFXComponent = nullptr;

	DustVFX = nullptr;
	ActiveDustVFXComponent = nullptr;

	GroundCrackVFX = nullptr;
	ActiveGroundCrackVFXComponent = nullptr;
	DustCloudVFX = nullptr;
	ActiveDustCloudVFXComponent = nullptr;

	// === DraconicFury 충돌 VFX 초기화 ===
	DraconicProjectileImpactVFX = nullptr;
	DraconicProjectileExplosionVFX = nullptr;

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
}

void AGS_Drakhar::BeginPlay()
{
	Super::BeginPlay();

	GuardianState = EGuardianState::CtrlSkillEnd;

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
}

void AGS_Drakhar::Ctrl()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			GetSkillComp()->TryActivateSkill(ESkillSlot::Ready);
			ClientGuardianState = EGuardianState::CtrlUp; // 클라이언트 상태 즉시 업데이트
			ServerRPCStartCtrl();
		}
		//for fix input bugs...
		else
		{
			ClientGuardianState = EGuardianState::CtrlSkillEnd;
		}

		TargetSpringArmLength = 700.f;
		bIsFlying = true;
	}
}

void AGS_Drakhar::CtrlStop()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
		ClientGuardianState = EGuardianState::CtrlSkillEnd; // 클라이언트 상태 즉시 업데이트
		ServerRPCStopCtrl();
		
		TargetSpringArmLength = 500.f;
		bIsFlying = true;
	}
}

void AGS_Drakhar::LeftMouse()
{
	Super::LeftMouse();

	if (!HasAuthority() && IsLocallyControlled())
	{
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready))
		{
			//earthquake
			GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}

		//not flying
		else if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			if (bClientCanCombo)
			{
				PlayComboAttackMontage();
				ServerRPCNewComboAttack();
				bClientCanCombo = false;
			}
		}
		//for fix input bugs...
		else
		{
			ClientGuardianState = EGuardianState::CtrlSkillEnd;
		}
	}
}

void AGS_Drakhar::RightMouse()
{
	if (IsLocallyControlled())
	{
		//ultimate skill (DraconicFury)
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready))
		{
			ServerRPC_BeginDraconicFury();
		}
		//dash skill
		else if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
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
		bClientCanCombo = true;
		ServerRPCResetValue();
	}
}

void AGS_Drakhar::OnRep_CanCombo()
{
	bClientCanCombo = bCanCombo;
}

void AGS_Drakhar::ComboLastAttack()
{
	if (HasAuthority())
	{
		const FVector Start = GetActorLocation();
		const float Radius = 300.f;
		const float PlusDamage = 20.f;

		TSet<AGS_Character*> DamagedPlayers = DetectPlayerInRange(Start, 200.f, Radius);
		ApplyDamageToDetectedPlayer(DamagedPlayers, PlusDamage);

		if (IsFeverMode)
		{
			FeverComoLastAttack();
		}
	}	
}

void AGS_Drakhar::ServerRPCResetValue_Implementation()
{
	bCanCombo = true;
	//GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AGS_Drakhar::ServerRPCNewComboAttack_Implementation()
{
	MulticastRPCComboAttack();
	//GetCharacterMovement()->SetMovementMode(MOVE_None);
	bCanCombo = false;

	// 콤보 공격 사운드 재생
	PlayComboAttackSound();
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

	if (DamagedCharactersFromDash.IsEmpty())
	{
		return;
	}

	for (auto const& DamagedCharacter : DamagedCharactersFromDash)
	{
		float SkillCoefficient = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Moving)->Damage;
		float RealDamage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter, SkillCoefficient);

		FDamageEvent DamageEvent;
		DamagedCharacter->TakeDamage(RealDamage, DamageEvent, GetController(), this);

		if (IsFeverMode)
		{
			DamagedCharacter->GetDebuffComp()->ApplyDebuff(EDebuffType::Bleed, this);
		}

		// === 대시 스킬 히트 사운드 재생 ===
		PlayAttackHitSound();

		FVector DrakharPos = GetActorLocation();
		FVector DamagedPos = DamagedCharacter->GetActorLocation();
		FVector OutVector = (DamagedPos - DrakharPos);
		FVector TempVector = -OutVector.Dot(DashDirection) * DashDirection;
		FVector ResultVector = TempVector + OutVector;

		DamagedCharacter->LaunchCharacter(ResultVector.GetSafeNormal() * 10000.f, true, true);
	}

	DamagedCharactersFromDash.Empty();
	bCanCombo = true;

	// 대시 VFX 종료 (Wing Rush + Dust)
	StopWingRushVFX();
	StopDustVFX();
}

void AGS_Drakhar::ServerRPCCalculateDashLocation_Implementation()
{
	DashInterpAlpha = 0.f;
	DashStartLocation = GetActorLocation();
	DashEndLocation = DashStartLocation + GetActorForwardVector() * DashPower;

	// 대시 스킬 사운드 재생
	PlayDashSkillSound();

	// 대시 VFX 시작
	StartWingRushVFX();
	StartDustVFX();
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
	// 지진 스킬 사운드 재생
	PlayEarthquakeSkillSound();

	// === 카메라 쉐이크 재생 ===
	if (CameraShakeComponent)
	{
		CameraShakeComponent->PlayCameraShake(EarthquakeShakeInfo);
	}

	// === 어스퀘이크 VFX 재생 ===
	StartGroundCrackVFX();
	StartDustCloudVFX();


	const FVector Start = GetActorLocation() + 100.f;
	TSet<AGS_Character*> EarthquakeDamagedCharacters = DetectPlayerInRange(Start, 200.f, EarthquakeRadius);
	
	for (const auto& DamagedCharacter : EarthquakeDamagedCharacters)
	{
		float SkillCoefficient = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Aiming)->Damage;
		float RealDamage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter, SkillCoefficient);

		FDamageEvent DamageEvent;
		if (IsValid(DamagedCharacter))
		{
			if (IsFeverMode)
			{
				DamagedCharacter->GetDebuffComp()->ApplyDebuff(EDebuffType::Bleed, this);
			}
			DamagedCharacter->TakeDamage(RealDamage, DamageEvent, GetController(), this);

			// === 어스퀘이크 스킬 히트 사운드 재생 ===
			PlayAttackHitSound();

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
	GuardianState = EGuardianState::CtrlUp;

	MoveSpeed = SpeedUpMoveSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	PlayFlyStartSound();
}

void AGS_Drakhar::ServerRPCStopCtrl_Implementation()
{
	GuardianState = EGuardianState::CtrlSkillEnd;

	MoveSpeed = NormalMoveSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	PlayFlyEndSound();
}

void AGS_Drakhar::ServerRPCSpawnDraconicFury_Implementation()
{
	// 스킬 사운드는 첫 번째 투사체 생성 시에만 재생
	if (!bDraconicFurySoundPlayed)
	{
		PlayDraconicFurySkillSound();
		bDraconicFurySoundPlayed = true;

		FTimerHandle ResetSoundTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetSoundTimer, [this]()
		{
			bDraconicFurySoundPlayed = false;
		}, 7.0f, false); // 리셋 (스킬 지속시간에 맞게 조정)
	}

	if (IsFeverMode)
	{
		FeverModeDraconicFurySpawnLocation = GetActorLocation() + GetActorForwardVector() * 200.f + FVector(0.f,0.f, 600.f);
		FRotator SpawnRotation = GetActorRotation();
		float RandomPitch = FMath::FRandRange(-35.f, -30.f);
		SpawnRotation.Pitch += RandomPitch;
		
		AGS_DrakharProjectile* DrakharProjectile = GetWorld()->SpawnActor<AGS_DrakharProjectile>(FeverDraconicProjectile, FeverModeDraconicFurySpawnLocation, SpawnRotation);

		
		if (DrakharProjectile)
		{
			DrakharProjectile->SetOwner(this);
		}
	}
	else
	{
		GetRandomDraconicFuryTarget();

		int32 Index = FMath::RandRange(0, DraconicFuryTargetArray.Num() - 1);
		
		//spawn meteor
		AGS_DrakharProjectile* DrakharProjectile = GetWorld()->SpawnActor<AGS_DrakharProjectile>(DraconicProjectile, DraconicFuryTargetArray[Index].GetLocation(), DraconicFuryTargetArray[Index].Rotator());
		if (DrakharProjectile)
		{
			DrakharProjectile->SetOwner(this);

			// 투사체 위치에서 사운드 재생 (각 투사체마다 개별 사운드)
			PlayDraconicProjectileSound(DrakharProjectile->GetActorLocation());
		}
	}
}

void AGS_Drakhar::SetFeverGaugeWidget(UGS_DrakharFeverGauge* InDrakharFeverGaugeWidget)
{
	UGS_DrakharFeverGauge* DrakharFeverGaugeWidget = Cast<UGS_DrakharFeverGauge>(InDrakharFeverGaugeWidget);
	if (IsValid(DrakharFeverGaugeWidget))
	{
		//client
		DrakharFeverGaugeWidget->InitializeGauge(GetCurrentFeverGauge());
		OnCurrentFeverGaugeChanged.AddUObject(DrakharFeverGaugeWidget,
		                                     &UGS_DrakharFeverGauge::OnCurrentFeverGaugeChanged);
	}
}

void AGS_Drakhar::SetFeverGauge(float InValue)
{	
	//client & server
	if (HasAuthority())
	{
		CurrentFeverGauge += InValue;

		//Stop Fever Mode
		if (CurrentFeverGauge < KINDA_SMALL_NUMBER)
		{
			CurrentFeverGauge = 0.f;

			GetWorldTimerManager().ClearTimer(FeverTimer);
			if (IsFeverMode)
			{
				FGS_StatRow Stat;
				Stat.ATK = 50.f;
				GetStatComp()->ResetStat(Stat);
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
		ApplyDamageToDetectedPlayer(DamagedSeekers, 20.f);
		for (const auto& DamagedSeeker : DamagedSeekers)
		{
			DamagedSeeker->LaunchCharacter(FVector(0.f, 0.f, 500.f), true, true);
		}
	}
}

void AGS_Drakhar::StartFeverMode()
{
	//server
	FGS_StatRow Stat;
	Stat.ATK = 50.f;
		
	GetStatComp()->ChangeStat(Stat);
	MulticastRPCFeverMontagePlay();
	
	// === 피버모드 시작 사운드 재생 ===
	PlayFeverModeStartSound();
}

void AGS_Drakhar::DecreaseFeverGauge()
{
	GetWorldTimerManager().SetTimer(FeverTimer, this, &AGS_Drakhar::MinusFeverGaugeValue, true, 1.f);
}

void AGS_Drakhar::MinusFeverGaugeValue()
{
	SetFeverGauge(-1.f);
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

// === Wwise 사운드 재생 함수 구현 ===


void AGS_Drakhar::PlayComboAttackSound()
{
	if (HasAuthority())
	{
		MulticastPlayComboAttackSound();
	}
}

void AGS_Drakhar::PlayDashSkillSound()
{
	if (HasAuthority())
	{
		MulticastPlayDashSkillSound();
	}
}

void AGS_Drakhar::PlayEarthquakeSkillSound()
{
	if (HasAuthority())
	{
		MulticastPlayEarthquakeSkillSound();
	}
}

void AGS_Drakhar::PlayDraconicFurySkillSound()
{
	if (HasAuthority())
	{
		MulticastPlayDraconicFurySkillSound();
	}
}

void AGS_Drakhar::PlayDraconicProjectileSound(const FVector& Location)
{
	if (HasAuthority())
	{
		MulticastPlayDraconicProjectileSound(Location);
	}
}

// === Multicast 사운드 RPC 함수들 구현 ===

void AGS_Drakhar::MulticastPlayComboAttackSound_Implementation()
{
	PlaySoundEvent(ComboAttackSoundEvent);
}

void AGS_Drakhar::MulticastPlayDashSkillSound_Implementation()
{
	PlaySoundEvent(DashSkillSoundEvent);
}

void AGS_Drakhar::MulticastPlayEarthquakeSkillSound_Implementation()
{
	PlaySoundEvent(EarthquakeSkillSoundEvent);
}

void AGS_Drakhar::MulticastPlayDraconicFurySkillSound_Implementation()
{
	PlaySoundEvent(DraconicFurySkillSoundEvent);
}

void AGS_Drakhar::MulticastPlayDraconicProjectileSound_Implementation(const FVector& Location)
{
	PlaySoundEvent(DraconicProjectileSoundEvent, Location);
}

// === Wwise 헬퍼 함수 구현 ===

void AGS_Drakhar::PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location)
{
	// 멀티플레이어 환경에서 전용 서버는 오디오를 처리하지 않음
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!SoundEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_Drakhar::PlaySoundEvent - SoundEvent is null"));
		return;
	}

	// Wwise AudioDevice가 초기화되었는지 확인
	if (!FAkAudioDevice::Get())
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_Drakhar::PlaySoundEvent - Wwise AudioDevice is not initialized"));
		return;
	}

	UAkComponent* AkComp = GetOrCreateAkComponent();
	if (!AkComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_Drakhar::PlaySoundEvent - Failed to get AkComponent"));
		return;
	}


	// 특정 위치에서 재생하는 경우 (투사체 등)
	if (Location != FVector::ZeroVector)
	{
		FOnAkPostEventCallback DummyCallback;
		UAkGameplayStatics::PostEventAtLocation(SoundEvent, Location, FRotator::ZeroRotator, GetWorld());
	}
	else
	{
		AkComp->PostAkEvent(SoundEvent);
	}
}

UAkComponent* AGS_Drakhar::GetOrCreateAkComponent()
{
	UAkComponent* AkComp = FindComponentByClass<UAkComponent>();

	if (!AkComp)
	{
		// 런타임에 AkComponent 생성 (생성자에서 실패한 경우를 대비)
		AkComp = NewObject<UAkComponent>(this, TEXT("RuntimeAkAudioComponent"));
		if (AkComp)
		{
			AkComp->SetupAttachment(RootComponent);
			AkComp->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("GS_Drakhar: AkComponent created at runtime"));
		}
	}

	return AkComp;
}

void AGS_Drakhar::OnRep_FeverGauge()
{
	OnCurrentFeverGaugeChanged.Broadcast(CurrentFeverGauge);
}

// === 날기 사운드 재생 함수들 구현 ===

void AGS_Drakhar::PlayFlyStartSound()
{
	if (HasAuthority())
	{
		MulticastPlayFlyStartSound();
	}
}

void AGS_Drakhar::PlayFlyEndSound()
{
	if (HasAuthority())
	{
		MulticastPlayFlyEndSound();
	}
}

void AGS_Drakhar::PlayAttackHitSound()
{
	if (HasAuthority())
	{
		MulticastPlayAttackHitSound();
	}
}

void AGS_Drakhar::PlayFeverModeStartSound()
{
	if (HasAuthority())
	{
		MulticastPlayFeverModeStartSound();
	}
}

// === 날기 사운드 Multicast RPC 함수들 구현 ===

void AGS_Drakhar::MulticastPlayFlyStartSound_Implementation()
{
	PlaySoundEvent(FlyStartSoundEvent);
}

void AGS_Drakhar::MulticastPlayFlyEndSound_Implementation()
{
	PlaySoundEvent(FlyEndSoundEvent);
}

void AGS_Drakhar::MulticastPlayAttackHitSound_Implementation()
{
	PlaySoundEvent(AttackHitSoundEvent);
}

void AGS_Drakhar::MulticastPlayFeverModeStartSound_Implementation()
{
	PlaySoundEvent(FeverModeStartSoundEvent);
}

void AGS_Drakhar::ServerRPCShootEnergy_Implementation()
{
	// TODO: 투사체 발사 로직 구현
}

// === 나이아가라 VFX 함수 구현 ===

void AGS_Drakhar::StartWingRushVFX()
{
	if (HasAuthority())
	{
		MulticastStartWingRushVFX();
	}
}

void AGS_Drakhar::StopWingRushVFX()
{
	if (HasAuthority())
	{
		MulticastStopWingRushVFX();
	}
}

void AGS_Drakhar::MulticastStartWingRushVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 이미 활성화된 VFX가 있다면 정리
	if (ActiveWingRushVFXComponent && IsValid(ActiveWingRushVFXComponent))
	{
		ActiveWingRushVFXComponent->DestroyComponent();
		ActiveWingRushVFXComponent = nullptr;
	}

	if (!WingRushRibbonVFX)
	{
		return;
	}

	// 화살표 컴포넌트를 사용한 스폰
	if (WingRushVFXSpawnPoint && IsValid(WingRushVFXSpawnPoint))
	{
		ActiveWingRushVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			WingRushRibbonVFX,
			WingRushVFXSpawnPoint,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// 폴백: 발 소켓에 직접 부착
	if (!ActiveWingRushVFXComponent)
	{
		ActiveWingRushVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			WingRushRibbonVFX,
			GetMesh(),
			FName("foot_l"),
			FVector(-20.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (ActiveWingRushVFXComponent)
	{
		// VFX 파라미터 설정
		FVector CurrentDashDirection = (DashEndLocation - DashStartLocation).GetSafeNormal();
		if (!CurrentDashDirection.IsZero())
		{
			ActiveWingRushVFXComponent->SetVectorParameter(FName("DashDirection"), CurrentDashDirection);
		}

		float DashSpeed = DashPower / DashDuration;
		ActiveWingRushVFXComponent->SetFloatParameter(FName("DashSpeed"), DashSpeed);
		ActiveWingRushVFXComponent->SetFloatParameter(FName("Scale"), 2.0f);
		ActiveWingRushVFXComponent->SetWorldScale3D(FVector(3.0f, 3.0f, 3.0f));
	}
}

void AGS_Drakhar::MulticastStopWingRushVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ActiveWingRushVFXComponent && IsValid(ActiveWingRushVFXComponent))
	{
		ActiveWingRushVFXComponent->Deactivate();

		FTimerHandle VFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(VFXCleanupTimer, [this]()
		{
			if (ActiveWingRushVFXComponent && IsValid(ActiveWingRushVFXComponent))
			{
				ActiveWingRushVFXComponent->DestroyComponent();
			}
			ActiveWingRushVFXComponent = nullptr;
		}, 2.0f, false);
	}
}

// === DustVFX 제어 함수 ===

void AGS_Drakhar::StartDustVFX()
{
	if (HasAuthority())
	{
		MulticastStartDustVFX();
	}
}

void AGS_Drakhar::StopDustVFX()
{
	if (HasAuthority())
	{
		MulticastStopDustVFX();
	}
}

// === DustVFX Multicast 구현 함수 ===
void AGS_Drakhar::MulticastStartDustVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ActiveDustVFXComponent && IsValid(ActiveDustVFXComponent))
	{
		ActiveDustVFXComponent->DestroyComponent();
		ActiveDustVFXComponent = nullptr;
	}

	if (!DustVFX)
	{
		return;
	}

	if (WingRushVFXSpawnPoint && IsValid(WingRushVFXSpawnPoint))
	{
		ActiveDustVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			DustVFX,
			WingRushVFXSpawnPoint,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// 폴백: 발 소켓에 직접 부착 (WingRush와 동일한 위치)
	if (!ActiveDustVFXComponent)
	{
		ActiveDustVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			DustVFX,
			GetMesh(),
			FName("foot_l"),
			FVector(-20.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (ActiveDustVFXComponent)
	{
		// DustVFX 파라미터 설정
		FVector CurrentDashDirection = (DashEndLocation - DashStartLocation).GetSafeNormal();
		if (!CurrentDashDirection.IsZero())
		{
			ActiveDustVFXComponent->SetVectorParameter(FName("DashDirection"), CurrentDashDirection);
		}

		float DashSpeed = DashPower / DashDuration;
		ActiveDustVFXComponent->SetFloatParameter(FName("DashSpeed"), DashSpeed);
		ActiveDustVFXComponent->SetFloatParameter(FName("Intensity"), 3.0f);
		ActiveDustVFXComponent->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
	}
}

void AGS_Drakhar::MulticastStopDustVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ActiveDustVFXComponent && IsValid(ActiveDustVFXComponent))
	{
		ActiveDustVFXComponent->Deactivate();

		FTimerHandle DustVFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(DustVFXCleanupTimer, [this]()
		{
			if (ActiveDustVFXComponent && IsValid(ActiveDustVFXComponent))
			{
				ActiveDustVFXComponent->DestroyComponent();
			}
			ActiveDustVFXComponent = nullptr;
		}, 1.5f, false);
	}
}

void AGS_Drakhar::ServerRPC_BeginDraconicFury_Implementation()
{
	if (GetSkillComp()->IsSkillActive(ESkillSlot::Ultimate))
	{
		return;
	}

	GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);

	FTimerHandle DraconicFuryEndTimer;
	GetWorld()->GetTimerManager().SetTimer(
		DraconicFuryEndTimer,
		this,
		&AGS_Drakhar::EndDraconicFury,
		DraconicAttackPersistenceTime,
		false);
}

void AGS_Drakhar::EndDraconicFury()
{
	GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
	GuardianState = EGuardianState::ForceLanded;

	MoveSpeed = NormalMoveSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	PlayFlyEndSound();
}

// === 어스퀘이크 지면 균열 VFX 제어 함수 ===

void AGS_Drakhar::StartGroundCrackVFX()
{
	if (HasAuthority())
	{
		MulticastStartGroundCrackVFX();
	}
}

void AGS_Drakhar::StopGroundCrackVFX()
{
	if (HasAuthority())
	{
		MulticastStopGroundCrackVFX();
	}
}

void AGS_Drakhar::MulticastStartGroundCrackVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 이미 활성화된 VFX가 있다면 정리
	if (ActiveGroundCrackVFXComponent && IsValid(ActiveGroundCrackVFXComponent))
	{
		ActiveGroundCrackVFXComponent->DestroyComponent();
		ActiveGroundCrackVFXComponent = nullptr;
	}

	if (!GroundCrackVFX)
	{
		return;
	}

	// 어스퀘이크 스폰 포인트를 사용한 스폰
	if (EarthquakeVFXSpawnPoint && IsValid(EarthquakeVFXSpawnPoint))
	{
		ActiveGroundCrackVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			GroundCrackVFX,
			EarthquakeVFXSpawnPoint,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// 폴백: 캐릭터 발 아래에 직접 스폰
	if (!ActiveGroundCrackVFXComponent)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, -90.f);
		ActiveGroundCrackVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			GroundCrackVFX,
			SpawnLocation,
			GetActorRotation(),
			FVector(1.0f, 1.0f, 1.0f),
			true
		);
	}

	if (ActiveGroundCrackVFXComponent)
	{
		// 지면 균열 VFX 파라미터 설정
		ActiveGroundCrackVFXComponent->SetFloatParameter(FName("CrackIntensity"), EarthquakeShakeInfo.Intensity);
		ActiveGroundCrackVFXComponent->SetFloatParameter(FName("CrackRadius"), EarthquakeShakeInfo.MaxDistance * 0.5f);
		ActiveGroundCrackVFXComponent->SetWorldScale3D(FVector(2.0f, 2.0f, 1.0f));
	}
}

void AGS_Drakhar::MulticastStopGroundCrackVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ActiveGroundCrackVFXComponent && IsValid(ActiveGroundCrackVFXComponent))
	{
		ActiveGroundCrackVFXComponent->Deactivate();

		FTimerHandle GroundCrackVFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(GroundCrackVFXCleanupTimer, [this]()
		{
			if (ActiveGroundCrackVFXComponent && IsValid(ActiveGroundCrackVFXComponent))
			{
				ActiveGroundCrackVFXComponent->DestroyComponent();
			}
			ActiveGroundCrackVFXComponent = nullptr;
		}, 3.0f, false); // 3초 후 정리 (균열이 천천히 사라지도록)
	}
}

// === 어스퀘이크 먼지 구름 VFX 제어 함수 ===

void AGS_Drakhar::StartDustCloudVFX()
{
	if (HasAuthority())
	{
		MulticastStartDustCloudVFX();
	}
}

void AGS_Drakhar::StopDustCloudVFX()
{
	if (HasAuthority())
	{
		MulticastStopDustCloudVFX();
	}
}

void AGS_Drakhar::MulticastStartDustCloudVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 이미 활성화된 VFX가 있다면 정리
	if (ActiveDustCloudVFXComponent && IsValid(ActiveDustCloudVFXComponent))
	{
		ActiveDustCloudVFXComponent->DestroyComponent();
		ActiveDustCloudVFXComponent = nullptr;
	}

	if (!DustCloudVFX)
	{
		return;
	}

	// 어스퀘이크 스폰 포인트를 사용한 스폰
	if (EarthquakeVFXSpawnPoint && IsValid(EarthquakeVFXSpawnPoint))
	{
		ActiveDustCloudVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			DustCloudVFX,
			EarthquakeVFXSpawnPoint,
			NAME_None,
			FVector(0.f, 0.f, 50.f), // 지면에서 약간 위로
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// 폴백: 캐릭터 주변에 직접 스폰
	if (!ActiveDustCloudVFXComponent)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, -40.f);
		ActiveDustCloudVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DustCloudVFX,
			SpawnLocation,
			GetActorRotation(),
			FVector(1.5f, 1.5f, 1.5f),
			true
		);
	}

	if (ActiveDustCloudVFXComponent)
	{
		// 먼지 구름 VFX 파라미터 설정
		ActiveDustCloudVFXComponent->SetFloatParameter(FName("DustIntensity"), EarthquakeShakeInfo.Intensity * 1.5f);
		ActiveDustCloudVFXComponent->SetFloatParameter(FName("DustRadius"), EarthquakeShakeInfo.MaxDistance * 0.3f);
		ActiveDustCloudVFXComponent->SetFloatParameter(FName("WindStrength"), 5.0f);
		ActiveDustCloudVFXComponent->SetWorldScale3D(FVector(3.0f, 3.0f, 2.0f));

		// 먼지 구름은 2초 후 자동으로 종료
		FTimerHandle DustCloudAutoStopTimer;
		GetWorld()->GetTimerManager().SetTimer(DustCloudAutoStopTimer, [this]()
		{
			StopDustCloudVFX();
		}, 2.0f, false);
	}
}

// === DraconicFury 투사체 충돌 처리 함수 구현 ===

void AGS_Drakhar::HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal,
                                                 bool bHitCharacter)
{
	if (HasAuthority())
	{
		MulticastPlayDraconicProjectileImpactEffects(ImpactLocation, ImpactNormal, bHitCharacter);
	}
}

void AGS_Drakhar::MulticastPlayDraconicProjectileImpactEffects_Implementation(
	const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// === 사운드 이펙트 재생 ===
	UAkAudioEvent* SoundToPlay = bHitCharacter
		                             ? DraconicProjectileExplosionSoundEvent
		                             : DraconicProjectileImpactSoundEvent;
	if (SoundToPlay)
	{
		PlaySoundEvent(SoundToPlay, ImpactLocation);
	}

	// === 시각 이펙트 재생 ===
	UNiagaraSystem* VFXToPlay = bHitCharacter ? DraconicProjectileExplosionVFX : DraconicProjectileImpactVFX;
	if (VFXToPlay && GetWorld())
	{
		// 충돌 지점에서 VFX 재생
		UNiagaraComponent* ImpactVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			ImpactLocation,
			FRotationMatrix::MakeFromZ(ImpactNormal).Rotator(), // 충돌면 법선 방향으로 회전
			FVector(1.0f, 1.0f, 1.0f),
			true,
			true,
			ENCPoolMethod::None,
			true
		);

		if (ImpactVFXComponent)
		{
			// VFX 파라미터 설정
			ImpactVFXComponent->SetVectorParameter(FName("ImpactNormal"), ImpactNormal);
			ImpactVFXComponent->SetFloatParameter(FName("ImpactIntensity"), bHitCharacter ? 2.0f : 1.0f);

			// 캐릭터 타격 시 더 큰 스케일
			float VFXScale = bHitCharacter ? 1.5f : 1.0f;
			ImpactVFXComponent->SetWorldScale3D(FVector(VFXScale, VFXScale, VFXScale));

			// VFX 색상 설정 (캐릭터 타격 시 빨간색, 지형 타격 시 주황색)
			FLinearColor ImpactColor = bHitCharacter ? FLinearColor::Red : FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
			ImpactVFXComponent->SetColorParameter(FName("ImpactColor"), ImpactColor);
		}
	}

	// === 카메라 쉐이크 (가까운 플레이어만) ===
	if (bHitCharacter && CameraShakeComponent)
	{
		// 강한 충격 카메라 쉐이크 설정
		FGS_CameraShakeInfo ImpactShakeInfo;
		ImpactShakeInfo.Intensity = 4.0f;
		ImpactShakeInfo.MaxDistance = 1000.0f;
		ImpactShakeInfo.MinDistance = 100.0f;
		ImpactShakeInfo.PropagationSpeed = 500000.0f;
		ImpactShakeInfo.bUseFalloff = true;

		// 충돌 지점을 중심으로 카메라 쉐이크 재생
		CameraShakeComponent->PlayCameraShakeAtLocation(ImpactShakeInfo, ImpactLocation);
	}
}

void AGS_Drakhar::MulticastStopDustCloudVFX_Implementation()
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (ActiveDustCloudVFXComponent && IsValid(ActiveDustCloudVFXComponent))
	{
		ActiveDustCloudVFXComponent->Deactivate();

		FTimerHandle DustCloudVFXCleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(DustCloudVFXCleanupTimer, [this]()
		{
			if (ActiveDustCloudVFXComponent && IsValid(ActiveDustCloudVFXComponent))
			{
				ActiveDustCloudVFXComponent->DestroyComponent();
			}
			ActiveDustCloudVFXComponent = nullptr;
		}, 1.5f, false); // 1.5초 후 정리
	}
}
