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
#include "Sound/GS_AudioManager.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/ArrowComponent.h"

AGS_Drakhar::AGS_Drakhar()
{
	PrimaryActorTick.bCanEverTick = false;

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
	EarthquakePower = 3000.f;
	EarthquakeRadius = 500.f;

	//Guardian State Setting
	ClientGuardianState = EGuardianState::CtrlSkillEnd;

	//boss monster tag for user widget
	Tags.Add("Guardian");

	// === Wwise 사운드 이벤트 초기화 ===
	ComboAttackSoundEvent = nullptr;
	DashSkillSoundEvent = nullptr;
	EarthquakeSkillSoundEvent = nullptr;
	DraconicFurySkillSoundEvent = nullptr;
	DraconicProjectileSoundEvent = nullptr;

	// === 날기 사운드 이벤트 초기화 ===
	FlyStartSoundEvent = nullptr;
	FlyEndSoundEvent = nullptr;

	// AkComponent 추가
	if (!FindComponentByClass<UAkComponent>())
	{
		UAkComponent* AkComp = CreateDefaultSubobject<UAkComponent>(TEXT("AkAudioComponent"));
		if (AkComp)
		{
			AkComp->SetupAttachment(RootComponent);
		}
	}

	// === 나이아가라 VFX 초기화 ===
	WingRushRibbonVFX = nullptr;
	ActiveWingRushVFXComponent = nullptr;

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

void AGS_Drakhar::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanCombo);
}

void AGS_Drakhar::Ctrl()
{
	if (IsLocallyControlled())
	{
		if (ClientGuardianState == EGuardianState::CtrlSkillEnd)
		{
			GetSkillComp()->TryActivateSkill(ESkillSlot::Ready);
			this->ServerRPCStartCtrl();
		}
	}
}

void AGS_Drakhar::CtrlStop()
{
	if (IsLocallyControlled())
	{		
		GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
		this->ServerRPCStopCtrl();
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
	}
}

void AGS_Drakhar::RightMouse()
{
	if (IsLocallyControlled())
	{
		//ultimate skill
		if (GetSkillComp()->IsSkillActive(ESkillSlot::Ready))
		{	
			GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
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
	PlayAnimMontage(ComboAttackMontage,1.f, ComboAttackSectionName);
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s  %s"),*ComboAttackMontage.GetName(), *ComboAttackSectionName.ToString()), true, true, FLinearColor::Blue, 5.f);
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
	// 콤보 마지막 공격 로직 구현
	// TODO: 필요시 특별한 콤보 마지막 공격 로직을 여기에 추가
}

void AGS_Drakhar::ServerRPCResetValue_Implementation()
{
	bCanCombo = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AGS_Drakhar::ServerRPCNewComboAttack_Implementation()
{
	MulticastRPCComboAttack();
	GetCharacterMovement()->SetMovementMode(MOVE_None);
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
}

void AGS_Drakhar::ServerRPCEndDash_Implementation()
{
	if (DamagedCharacters.IsEmpty())
	{
		return;
	}
	
	for (auto const& DamagedCharacter : DamagedCharacters)
	{
		float SkillCoefficient = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Moving)->Damage;
		float RealDamage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter,SkillCoefficient);
		
		FDamageEvent DamageEvent;
		DamagedCharacter->TakeDamage(RealDamage, DamageEvent, GetController(), this);
	}

	DamagedCharacters.Empty();
	GuardianState = EGuardianState::CtrlSkillEnd;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	bCanCombo = true;
	
	// 대시 VFX 종료
	StopWingRushVFX();
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
}

void AGS_Drakhar::DashAttackCheck()
{
	TArray<FHitResult> OutHitResults;	
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * 10.f;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeCapsule(100.f, 100.f), Params);
	
	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			if (OutHitResult.GetComponent() && OutHitResult.GetComponent()->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}

			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				DamagedCharacters.Add(DamagedCharacter);
			}
		}		
	}
	//MulticastRPCDrawDebugLine(Start,End, 100.f, 100.f, GetActorForwardVector(), bIsHitDetected);
}

void AGS_Drakhar::ServerRPCEarthquakeAttackCheck_Implementation()
{
	// 지진 스킬 사운드 재생
	PlayEarthquakeSkillSound();
	
	TSet<AGS_Character*> EarthquakeDamagedCharacters;
	TArray<FHitResult> OutHitResults;
	const FVector Start = GetActorLocation() + 100.f;
	const FVector End = Start + GetActorForwardVector() * 100.f;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);

	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(EarthquakeRadius), Params);

	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			if (OutHitResult.GetComponent() && OutHitResult.GetComponent()->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}

			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				EarthquakeDamagedCharacters.Add(DamagedCharacter);
			}
		}
		for (auto const& DamagedCharacter : EarthquakeDamagedCharacters)
		{
			float SkillCoefficient = GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::Aiming)->Damage;
			float RealDamage = DamagedCharacter->GetStatComp()->CalculateDamage(this, DamagedCharacter,SkillCoefficient);

			FDamageEvent DamageEvent;
			if (IsValid(DamagedCharacter))
			{
				DamagedCharacter->TakeDamage(RealDamage, DamageEvent, GetController(), this);
				DamagedCharacter->LaunchCharacter(GetActorForwardVector() * EarthquakePower, false, false);
			}
		}
	}
	//MulticastRPCDrawDebugLine(Start, End, 100.f, EarthquakeRadius, GetActorForwardVector(),bIsHitDetected);
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
	PlayDraconicFurySkillSound();
	
	GetRandomDraconicFuryTarget();

	int32 Index = FMath::RandRange(0, DraconicFuryTargetArray.Num() - 1);

	UE_LOG(LogTemp, Warning, TEXT("Spawn"));

	//spawn meteor
	AGS_DrakharProjectile* DrakharProjectile = GetWorld()->SpawnActor<AGS_DrakharProjectile>(DraconicProjectile, DraconicFuryTargetArray[Index].GetLocation(), DraconicFuryTargetArray[Index].Rotator());
	if (DrakharProjectile)
	{
		DrakharProjectile->SetOwner(this);
		
		// 투사체 위치에서 사운드 재생
		PlayDraconicProjectileSound(DrakharProjectile->GetActorLocation());
	}
}

void AGS_Drakhar::GetRandomDraconicFuryTarget()
{
	DraconicFuryTargetArray.Empty();

	for (int32 i = 0; i < 5; ++i)
	{
		FVector StartLocation = GetActorLocation();
		FVector Offset = GetActorForwardVector() * 200.f + FVector(FMath::FRandRange(-300.f, 300.f), FMath::FRandRange(-300.f, 300.f), FMath::FRandRange(500.f, 600.f));

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

// === 날기 사운드 Multicast RPC 함수들 구현 ===

void AGS_Drakhar::MulticastPlayFlyStartSound_Implementation()
{
	PlaySoundEvent(FlyStartSoundEvent);
}

void AGS_Drakhar::MulticastPlayFlyEndSound_Implementation()
{
	PlaySoundEvent(FlyEndSoundEvent);
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