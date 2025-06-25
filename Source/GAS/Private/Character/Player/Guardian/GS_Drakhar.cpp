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
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ArrowComponent.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Component/GS_FootManagerComponent.h"
#include "Character/Skill/Guardian/Drakhar/GS_EarthquakeEffect.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Character/GS_DrakharFeverGauge.h"
#include "Character/Component/GS_DrakharVFXComponent.h"
#include "Character/Component/GS_DrakharSFXComponent.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = true;
	
	VFXComponent = CreateDefaultSubobject<UGS_DrakharVFXComponent>(TEXT("VFXComponent"));
	SFXComponent = CreateDefaultSubobject<UGS_DrakharSFXComponent>(TEXT("SFXComponent"));
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
	ClientGuardianState = EGuardianState::CtrlEnd;
	ClientGuardianDoSkillState = EGuardianDoSkill::None;
	
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

	// === Wwise 사운드 이벤트 초기화 -> 이제 Drakhar 블루프린트에서 직접 설정.
	ComboAttackSoundEvent = nullptr;
	DashSkillSoundEvent = nullptr;
	EarthquakeSkillSoundEvent = nullptr;
	DraconicFurySkillSoundEvent = nullptr;
	DraconicProjectileSoundEvent = nullptr;
	DraconicProjectileImpactSoundEvent = nullptr;
	DraconicProjectileExplosionSoundEvent = nullptr;
	AttackHitSoundEvent = nullptr;
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
}

void AGS_Drakhar::BeginPlay()
{
	Super::BeginPlay();

	GuardianState = EGuardianState::CtrlEnd;

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
}

void AGS_Drakhar::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(FeverTimer);
	GetWorldTimerManager().ClearTimer(ResetAttackTimer);
	GetWorldTimerManager().ClearTimer(HealthRegenTimer);
	GetWorldTimerManager().ClearTimer(HealthDelayTimer);
}

void AGS_Drakhar::OnDamageStart()
{
	bIsDamaged = true;

	StopHealRegeneration();
	
	//timer start
	GetWorld()->GetTimerManager().SetTimer(HealthDelayTimer,this,&AGS_Drakhar::BeginHealRegeneration,5.f,false);
}

void AGS_Drakhar::Ctrl()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		//not flying
		if (ClientGuardianState == EGuardianState::CtrlEnd)
		{
			//if execute flying skill, prevent change state
			if (ClientGuardianDoSkillState != EGuardianDoSkill::None)
			{
				return;
			}
			
			ClientGuardianState = EGuardianState::CtrlUp;
			TargetSpringArmLength = 800.f;
			bIsFlying = true;
			
			//server logic
			GetSkillComp()->TryActivateSkill(ESkillSlot::Ready);
			ServerRPCStartCtrl();
		}
	}
}

void AGS_Drakhar::CtrlStop()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		//stop flying
		if (ClientGuardianDoSkillState == EGuardianDoSkill::None)
		{
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
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready) && ClientGuardianDoSkillState == EGuardianDoSkill::None)
		{
			//check earthquake skill
			GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}
		
		//not flying & not using skills
		else if (ClientGuardianDoSkillState == EGuardianDoSkill::None)
		{
			if (bClientCanCombo)
			{
				PlayComboAttackMontage();
				ServerRPCNewComboAttack();
				bClientCanCombo = false;
			}
		}
	}
}

void AGS_Drakhar::RightMouse()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		//flying & not using flying skill
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready) && ClientGuardianDoSkillState == EGuardianDoSkill::None)
		{
			//ultimate skill check
			ServerRPC_BeginDraconicFury();
		}
		//not flying & not using flying skills
		else if (ClientGuardianState == EGuardianState::CtrlEnd && ClientGuardianDoSkillState == EGuardianDoSkill::None)
		{
			//dash skill check
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

		for(AGS_Character* DamagedPlayer : DamagedPlayers)
		{
			if(IsValid(DamagedPlayer))
			{
				MulticastRPC_PlayAttackHitVFX(DamagedPlayer->GetActorLocation());
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
	//GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AGS_Drakhar::ServerRPCNewComboAttack_Implementation()
{
	MulticastRPCComboAttack();
	//GetCharacterMovement()->SetMovementMode(MOVE_None);
	bCanCombo = false;

	MulticastPlayComboAttackSound();
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

		MulticastRPC_PlayAttackHitVFX(DamagedCharacter->GetActorLocation());
		MulticastPlayAttackHitSound();

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

	MulticastPlayDashSkillSound();
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
	MulticastPlayEarthquakeSkillSound();

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

		FDamageEvent DamageEvent;
		if (IsValid(DamagedCharacter))
		{
			if (IsFeverMode)
			{
				DamagedCharacter->GetDebuffComp()->ApplyDebuff(EDebuffType::Bleed, this);
				MulticastPlayFeverEarthquakeImpactVFX(DamagedCharacter->GetActorLocation());
			}
			else
			{
				MulticastPlayEarthquakeImpactVFX(DamagedCharacter->GetActorLocation());
			}
			
			DamagedCharacter->TakeDamage(RealDamage, DamageEvent, GetController(), this);
			// === 어스퀘이크 스킬 히트 사운드 재생 ===
			MulticastRPC_PlayAttackHitVFX(DamagedCharacter->GetActorLocation());
			MulticastPlayAttackHitSound();

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
}

void AGS_Drakhar::StopCtrl()
{
	//[Client] reset flying skill values
	if (!HasAuthority())
	{
		ServerRPCStopCtrl();
		GetSkillComp()->TryDeactiveSkill(ESkillSlot::Ready);
		
		ClientGuardianState = EGuardianState::CtrlEnd;
		ClientGuardianDoSkillState = EGuardianDoSkill::None;
		TargetSpringArmLength = 500.f;
		bIsFlying = true;
	}
}
 
void AGS_Drakhar::ServerRPCStopCtrl_Implementation()
{
	GuardianState = EGuardianState::CtrlEnd;
	MoveSpeed = NormalMoveSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AGS_Drakhar::ServerRPCSpawnDraconicFury_Implementation()
{
	MulticastPlayDraconicFurySkillSound();
	
	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	if (IsFeverMode)
	{
		FeverModeDraconicFurySpawnLocation = GetActorLocation() + GetActorForwardVector() * 200.f + FVector(0.f,0.f, 600.f);
		FRotator SpawnRotation = GetActorRotation();
		float RandomPitch = FMath::FRandRange(-35.f, -30.f);
		SpawnRotation.Pitch += RandomPitch;
		AGS_DrakharProjectile* DrakharProjectile = GetWorld()->SpawnActor<AGS_DrakharProjectile>(FeverDraconicProjectile, FeverModeDraconicFurySpawnLocation, SpawnRotation, Params);
	}
	else
	{
		GetRandomDraconicFuryTarget();

		int32 Index = FMath::RandRange(0, DraconicFuryTargetArray.Num() - 1);
		AGS_DrakharProjectile* DrakharProjectile = GetWorld()->SpawnActor<AGS_DrakharProjectile>(DraconicProjectile, DraconicFuryTargetArray[Index].GetLocation(), DraconicFuryTargetArray[Index].Rotator(), Params);
		
		if (DrakharProjectile)
		{
			MulticastPlayDraconicProjectileSound(DrakharProjectile->GetActorLocation());
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

			GetWorldTimerManager().ClearTimer(FeverTimer);
			if (IsFeverMode)
			{
				FGS_StatRow Stat;
				Stat.ATK = 50.f;
				GetStatComp()->ResetStat(Stat);
				MulticastRPC_OnFeverModeEnd();
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
	GetWorldTimerManager().ClearTimer(ResetAttackTimer);
	GetWorldTimerManager().SetTimer(ResetAttackTimer, this, &AGS_Drakhar::StartIsAttackingTimer, 3.f, false);
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
		ApplyDamageToDetectedPlayer(DamagedSeekers, 20.f);
		for (const auto& DamagedSeeker : DamagedSeekers)
		{
			MulticastRPC_PlayAttackHitVFX(DamagedSeeker->GetActorLocation());
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
	MulticastPlayFeverModeStartSound();
	MulticastRPC_OnFeverModeStart();
}

void AGS_Drakhar::DecreaseFeverGauge()
{
	GetWorldTimerManager().SetTimer(FeverTimer, this, &AGS_Drakhar::MinusFeverGaugeValue, 1.f, true);
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
	GetWorld()->GetTimerManager().SetTimer(HealthRegenTimer, this, &AGS_Drakhar::HealRegeneration,1.f,true);
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
	GetWorld()->GetTimerManager().ClearTimer(HealthRegenTimer);
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

// === Multicast 사운드 RPC 함수들 구현 ===

void AGS_Drakhar::MulticastPlayComboAttackSound_Implementation()
{
	if (SFXComponent) SFXComponent->PlayComboAttackSound();
}

void AGS_Drakhar::MulticastPlayDashSkillSound_Implementation()
{
	if (SFXComponent) SFXComponent->PlayDashSkillSound();
}

void AGS_Drakhar::MulticastPlayEarthquakeSkillSound_Implementation()
{
	if (SFXComponent) SFXComponent->PlayEarthquakeSkillSound();
}

void AGS_Drakhar::MulticastPlayDraconicFurySkillSound_Implementation()
{
	if (SFXComponent) SFXComponent->PlayDraconicFurySkillSound();
}

void AGS_Drakhar::MulticastPlayDraconicProjectileSound_Implementation(const FVector& Location)
{
	if (SFXComponent) SFXComponent->PlayDraconicProjectileSound(Location);
}

void AGS_Drakhar::OnRep_FeverGauge()
{
	OnCurrentFeverGaugeChanged.Broadcast(CurrentFeverGauge);
}

void AGS_Drakhar::MulticastPlayAttackHitSound_Implementation()
{
	if (!SFXComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("MulticastPlayAttackHitSound: SFXComponent is null"));
		return;
	}
	
	SFXComponent->PlayAttackHitSound();
}

void AGS_Drakhar::MulticastPlayFeverModeStartSound_Implementation()
{
	if (SFXComponent) SFXComponent->PlayFeverModeStartSound();
}

void AGS_Drakhar::ServerRPCShootEnergy_Implementation()
{
	// TODO: 투사체 발사 로직 구현
}

// === 나이아가라 VFX 함수 구현 ===
void AGS_Drakhar::MulticastStartWingRushVFX_Implementation()
{
	if (VFXComponent) VFXComponent->StartWingRushVFX();
}

void AGS_Drakhar::MulticastStopWingRushVFX_Implementation()
{
	if (VFXComponent) VFXComponent->StopWingRushVFX();
}

void AGS_Drakhar::MulticastStartDustVFX_Implementation()
{
	if (VFXComponent) VFXComponent->StartDustVFX();
}

void AGS_Drakhar::MulticastStopDustVFX_Implementation()
{
	if (VFXComponent) VFXComponent->StopDustVFX();
}

void AGS_Drakhar::ServerRPC_BeginDraconicFury_Implementation()
{
	if (GetSkillComp()->IsSkillActive(ESkillSlot::Ultimate))
	{
		return;
	}

	GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
	MulticastRPC_OnUltimateStart();

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
	UE_LOG(LogTemp, Warning, TEXT("Draconic Fury Skill End"));
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT] Draconic Fury Skill End")));
	
	GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
	GuardianState = EGuardianState::CtrlEnd;

	MoveSpeed = NormalMoveSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

// === 어스퀘이크 지면 균열 VFX 제어 함수 ===
void AGS_Drakhar::MulticastStartGroundCrackVFX_Implementation()
{
	if (VFXComponent) VFXComponent->StartGroundCrackVFX();
}

void AGS_Drakhar::MulticastStopGroundCrackVFX_Implementation()
{
	if (VFXComponent) VFXComponent->StopGroundCrackVFX();
}

// === 어스퀘이크 먼지 구름 VFX 제어 함수 ===
void AGS_Drakhar::MulticastStartDustCloudVFX_Implementation()
{
	if (VFXComponent) VFXComponent->StartDustCloudVFX();
}

// === DraconicFury 투사체 충돌 처리 함수 구현 ===

void AGS_Drakhar::HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter)
{
	if (HasAuthority())
	{
		MulticastPlayDraconicProjectileImpactEffects(ImpactLocation, ImpactNormal, bHitCharacter);
	}
}

void AGS_Drakhar::MulticastPlayDraconicProjectileImpactEffects_Implementation(
	const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter)
{
	if (VFXComponent) VFXComponent->HandleDraconicProjectileImpact(ImpactLocation, ImpactNormal, bHitCharacter);
	if (SFXComponent) SFXComponent->HandleDraconicProjectileImpact(ImpactLocation, bHitCharacter);
}

void AGS_Drakhar::MulticastPlayFeverEarthquakeImpactVFX_Implementation(const FVector& ImpactLocation)
{
	if (VFXComponent) VFXComponent->PlayFeverEarthquakeImpactVFX(ImpactLocation);
}

void AGS_Drakhar::MulticastRPC_OnFlyStart_Implementation()
{
	if (VFXComponent) VFXComponent->OnFlyStart();
}

void AGS_Drakhar::MulticastRPC_OnFlyEnd_Implementation()
{
	if (VFXComponent) VFXComponent->OnFlyEnd();
}

void AGS_Drakhar::MulticastRPC_OnUltimateStart_Implementation()
{
	if (VFXComponent) VFXComponent->OnUltimateStart();
}

void AGS_Drakhar::MulticastRPC_OnEarthquakeStart_Implementation()
{
	if (VFXComponent) VFXComponent->OnEarthquakeStart();
}

void AGS_Drakhar::MulticastRPC_OnFeverModeStart_Implementation()
{
	if (VFXComponent) VFXComponent->OnFeverModeChanged(true);
	BP_OnFeverModeStart();
}

void AGS_Drakhar::MulticastRPC_OnFeverModeEnd_Implementation()
{
	if (VFXComponent) VFXComponent->OnFeverModeChanged(false);
	BP_OnFeverModeEnd();
}

void AGS_Drakhar::OnRep_IsFeverMode()
{
	if (VFXComponent) VFXComponent->OnFeverModeChanged(IsFeverMode);
	
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

void AGS_Drakhar::MulticastRPC_PlayAttackHitVFX_Implementation(FVector ImpactPoint)
{
	if (VFXComponent) VFXComponent->PlayAttackHitVFX(ImpactPoint);
}

void AGS_Drakhar::MulticastPlayEarthquakeImpactVFX_Implementation(const FVector& ImpactLocation)
{
	if (VFXComponent) VFXComponent->PlayEarthquakeImpactVFX(ImpactLocation);
}

void AGS_Drakhar::MulticastStopDustCloudVFX_Implementation()
{
	if(VFXComponent) VFXComponent->StopDustCloudVFX();
}
