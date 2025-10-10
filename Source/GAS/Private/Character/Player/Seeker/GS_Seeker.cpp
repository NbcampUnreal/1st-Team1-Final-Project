// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Engine/GameInstance.h"
#include "Sound/GS_AudioManager.h"
#include "System/GS_PlayerState.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Components/ChildActorComponent.h"
#include "GameFramework/Character.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInterface.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Component/GS_VFXComponent.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"
#include "Character/GS_TpsController.h"
#include "Character/Skill/GS_SkillComp.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "AkAudioDevice.h"
#include "UI/Character/GS_HPTextWidgetComp.h"
#include "Sound/GS_SeekerAudioComponent.h"

// Sets default values
AGS_Seeker::AGS_Seeker()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	GetMesh()->bEnableUpdateRateOptimizations = false;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->bOnlyAllowAutonomousTickPose = false;

	// Post Process Component 생성 (Low Health)
	LowHealthPostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("LowHealthPostProcessComp"));
	LowHealthPostProcessComp->SetupAttachment(CameraComp);
	LowHealthPostProcessComp->bEnabled = false;
	LowHealthPostProcessComp->Priority = 10;

	// Post Process Component 생성 (가디언 감지 - MPP_Detect)
	DetectionPostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("DetectionPostProcessComp"));
	DetectionPostProcessComp->SetupAttachment(CameraComp);
	DetectionPostProcessComp->bEnabled = false;
	DetectionPostProcessComp->Priority = 11; // Low Health보다 높은 우선순위

	// =======================
	// VFX 컴포넌트 생성 (디버프, 힐링 등 모든 VFX)
	// =======================
	VFXComponent = CreateDefaultSubobject<UGS_VFXComponent>("VFXComponent");

	// =======================
	// 시커 오디오 컴포넌트 생성 (RTS/TPS 지원)
	// =======================
	SeekerAudioComponent = CreateDefaultSubobject<UGS_SeekerAudioComponent>("SeekerAudioComponent");

	// Fire Effect 생성 및 설정
	FeetLavaVFX_L = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FeetLavaVFX_L"));
	FeetLavaVFX_L->SetupAttachment(GetMesh(), FName("foot_l_Socket"));
	FeetLavaVFX_L->bAutoActivate = false;
	FeetLavaVFX_L->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	FeetLavaVFX_R = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FeetLavaVFX_R"));
	FeetLavaVFX_R->SetupAttachment(GetMesh(), FName("foot_r_Socket"));
	FeetLavaVFX_R->bAutoActivate = false;
	FeetLavaVFX_R->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	BodyLavaVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BodyLavaVFX"));
	BodyLavaVFX->SetupAttachment(GetMesh(), FName("spine_03"));
	BodyLavaVFX->bAutoActivate = false;
	BodyLavaVFX->SetRelativeLocation(FVector(-60.f, 0.f, 0.f));
	BodyLavaVFX->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	// 전투 BGM 트리거 생성 (시커가 몬스터를 감지)
	CombatTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("CombatTrigger"));
	CombatTrigger->SetupAttachment(RootComponent);
	CombatTrigger->SetSphereRadius(800.0f);
	CombatTrigger->SetCollisionProfileName(TEXT("SoundTrigger"));

	//함정 - 화살발사기의 화살 채널 설정(Projectile)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	//함정 - 모든 함정 채널 설정(Trap)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Ignore);

	// State
	SeekerGait = EGait::Run;
	LastSeekerGait = SeekerGait;
	CanChangeSeekerGait = true;
}

void AGS_Seeker::BeginPlay()
{
	Super::BeginPlay();

	// CombatTrigger 오버랩 이벤트 바인딩
	if (CombatTrigger)
	{
		CombatTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGS_Seeker::OnCombatTriggerBeginOverlap);
		CombatTrigger->OnComponentEndOverlap.AddDynamic(this, &AGS_Seeker::OnCombatTriggerEndOverlap);
	}

	// Generate Overlap Events 활성화 (화살 함정 충돌 처리를 위해 필요)
	if (GetMesh())
	{
		if (!GetMesh()->GetGenerateOverlapEvents())
		{
			GetMesh()->SetGenerateOverlapEvents(true);
		}
	}

	if (IsLocallyControlled())
	{
		InitializeCameraManager();

		// 스탯 컴포넌트 가져와서 델리게이트 바인딩
		if (UGS_StatComp* FoundStatComp = FindComponentByClass<UGS_StatComp>())
		{
			FoundStatComp->OnCurrentHPChanged.AddUObject(this, &AGS_Seeker::HandleLowHealthEffect);
		}

		// PlayerState 생존 상태 변경 델리게이트 바인딩
		AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
		if (PS)
		{
			PS->OnPlayerAliveStatusChangedDelegate.AddUObject(this, &AGS_Seeker::HandleAliveStatusChanged);
		}
	}
}

void AGS_Seeker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void AGS_Seeker::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (SkillInputHandlerComponent)
	{
		SkillInputHandlerComponent->SetupEnhancedInput(PlayerInputComponent);
	}
}

void AGS_Seeker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGS_Seeker, bIsLowHealthEffectActive);
	DOREPLIFETIME(AGS_Seeker, CurrentEffectStrength);
	DOREPLIFETIME(AGS_Seeker, LastSeekerGait);
	DOREPLIFETIME(AGS_Seeker, SeekerGait);
	DOREPLIFETIME(AGS_Seeker, CanChangeSeekerGait);
	DOREPLIFETIME(AGS_Seeker, CanAcceptComboInput);
	DOREPLIFETIME(AGS_Seeker, CurrentComboIndex);
	//DOREPLIFETIME(AGS_Seeker, bComboEnded);
	DOREPLIFETIME(AGS_Seeker, SeekerState);
	DOREPLIFETIME(AGS_Seeker, bIsDetectedByGuardian);
	DOREPLIFETIME(AGS_Seeker, DetectionIntensity);
}

void AGS_Seeker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(SeekerAudioComponent))
	{
		SeekerAudioComponent->SetComponentTickEnabled(false);
	}

	if (GetWorldTimerManager().IsTimerActive(LowHealthEffectTimer))
	{
		GetWorldTimerManager().ClearTimer(LowHealthEffectTimer);
	}
	
	if (IsLocallyControlled() && LowHealthPostProcessComp)
	{
		LowHealthPostProcessComp->bEnabled = false;
		LowHealthPostProcessComp->Settings.WeightedBlendables.Array.Empty();
	}

	// PlayerState 생존 상태 변경 델리게이트 해제
	if (IsLocallyControlled())
	{
		AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>();
		if (PS)
		{
			PS->OnPlayerAliveStatusChangedDelegate.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}


void AGS_Seeker::SetAimState(bool IsAim)
{
	SeekerState.IsAim = IsAim;
	
	// 시커 오디오 컴포넌트에 조준 상태 변경 알림
	if (SeekerAudioComponent)
	{
		if (IsAim)
		{
			SeekerAudioComponent->SetSeekerAudioState(ESeekerAudioState::Aiming);
		}
		else if (SeekerAudioComponent->GetCurrentAudioState() == ESeekerAudioState::Aiming)
		{
			// 조준을 해제했을 때 다른 상태로 전환
			SeekerAudioComponent->SetSeekerAudioState(ESeekerAudioState::Idle);
		}
	}
}

bool AGS_Seeker::GetAimState()
{
	return SeekerState.IsAim;
}

void AGS_Seeker::SetDrawState(bool IsDraw)
{
	FSeekerState NewState = SeekerState;
	NewState.IsDraw = IsDraw;
	SeekerState = NewState;
}

bool AGS_Seeker::GetDrawState()
{
	return SeekerState.IsDraw;
}

void AGS_Seeker::Server_SetSeekerGait_Implementation(EGait Gait)
{
	LastSeekerGait = SeekerGait;
	SeekerGait = Gait;

	if (UGS_SeekerAnimInstance* SeekerAnim = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		SeekerAnim->ChooserInputObj->Gait = SeekerGait;
	}
	
	switch (Gait)
	{
	case EGait::Walk :
		SetCharacterSpeed(0.45f);	
		break;
	case EGait::Run :
		SetCharacterSpeed(0.8f);
		break;
	case EGait::Sprint :
		SetCharacterSpeed(1.0f);
		break;
	}
}

void AGS_Seeker::SetSeekerGait(EGait Gait)
{
	LastSeekerGait = SeekerGait;
	SeekerGait = Gait;
	if (UGS_SeekerAnimInstance* SeekerAnim = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		SeekerAnim->ChooserInputObj->Gait = SeekerGait;
	}
	
	switch (Gait)
	{
	case EGait::Walk :
		SetCharacterSpeed(0.45f);	
		break;
	case EGait::Run :
		SetCharacterSpeed(0.8f);
		break;
	case EGait::Sprint :
		SetCharacterSpeed(1.0f);
		break;
	}
}

EGait AGS_Seeker::GetSeekerGait()
{
	return SeekerGait;
}

EGait AGS_Seeker::GetLastSeekerGait()
{
	return LastSeekerGait;
}

void AGS_Seeker::StateReset()
{
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			Multicast_SetMontageSlot(ESeekerMontageSlot::None);
		}
	}

	CanChangeSeekerGait = true;
	CanAcceptComboInput = true;
	SetMoveControlValue(true, true);
	SetLookControlValue(true, true);

	Multicast_SetMontageSlot(ESeekerMontageSlot::None);

	GetSkillComp()->ResetAllowedSkillsMask();
}

const FName AGS_Seeker::HPRatioParamName = TEXT("HPRatio");
const FName AGS_Seeker::EffectIntensityParamName = TEXT("EffectIntensity");

void AGS_Seeker::InitializeCameraManager()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		LocalCameraManager = PC->PlayerCameraManager;

		// Low Health 머티리얼 초기화
		if (LocalCameraManager && LowHealthEffectMaterial)
		{
			LowHealthDynamicMaterial = UMaterialInstanceDynamic::Create(LowHealthEffectMaterial, this);
			if (LowHealthDynamicMaterial)
			{
				float OutValue = 0.0f;
				if (!LowHealthDynamicMaterial->GetScalarParameterValue(HPRatioParamName, OutValue))
				{
					UE_LOG(LogTemp, Warning, TEXT("HPRatio 파라미터가 머티리얼에 존재하지 않습니다."));
				}

				LowHealthPostProcessComp->Settings.WeightedBlendables.Array.Empty();
				LowHealthPostProcessComp->Settings.AddBlendable(LowHealthDynamicMaterial, 1.0f);
			}
		}

		// MPP_Detect 머티리얼 초기화
		if (LocalCameraManager && DetectionEffectMaterial)
		{
			DetectionDynamicMaterial = UMaterialInstanceDynamic::Create(DetectionEffectMaterial, this);
			if (DetectionDynamicMaterial)
			{
				DetectionPostProcessComp->Settings.WeightedBlendables.Array.Empty();
				DetectionPostProcessComp->Settings.AddBlendable(DetectionDynamicMaterial, 1.0f);

				// 초기 강도 0으로 설정
				DetectionDynamicMaterial->SetScalarParameterValue(TEXT("DetectionIntensity"), 0.0f);
			}
		}
	}
}

void AGS_Seeker::Server_SetNextComboFlag_Implementation(bool NextCombo)
{
	bNextCombo = NextCombo;
}

void AGS_Seeker::Server_SetComboInputFlag_Implementation(bool InputCombo)
{
	CanAcceptComboInput = InputCombo;
}

void AGS_Seeker::ComboInputOpen()
{
	CanAcceptComboInput = true;
}

void AGS_Seeker::ComboInputClose()
{
	if (HasAuthority())
	{
		CanAcceptComboInput = false;
		if (bNextCombo)
		{
			ServerAttackMontage();
			Server_SetNextComboFlag(false);
		}
	}
}

void AGS_Seeker::Server_OnComboAttack_Implementation()
{
	if (!CanAcceptComboInput) // Handler 에서도 검사하고 있었는데 서버에서도 검사한다. 이중검사가 필요한가?
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_OnComboAttack, CanAcceptComboInput == false"));
		return;
	}

	if (!GetSkillComp()->IsSkillAllowed(ESkillSlot::Combo))
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_OnComboAttack, IsSkillAllowed == false"));
		return;
	}
		
	if (CurrentComboIndex == 0)
	{
		GetWorldTimerManager().ClearTimer(AttackSoundResetTimerHandle);
		ServerAttackMontage();
	}
	else
	{
		Server_SetNextComboFlag(true);
		Server_SetComboInputFlag(false); // server 함수의 호출을 막기 위해서 합친 함수를 만들어야 하나?
	}
}

void AGS_Seeker::SetMoveControlValue(bool bMoveForward, bool bMoveRight)
{
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetMoveControlValue"));
		TPSController->SetMoveControlValue(bMoveRight, bMoveForward);
	}
}

void AGS_Seeker::SetLookControlValue(bool bLookUp, bool bLookRight)
{
	if (AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController()))
	{
		TPSController->SetLookControlValue(bLookRight, bLookUp);
	}
}

void AGS_Seeker::UpdatePostProcessEffect(float EffectStrength)
{
	if (LowHealthDynamicMaterial)
	{
		LowHealthDynamicMaterial->SetScalarParameterValue(TEXT("HPRatio"), EffectStrength);
	}
}

void AGS_Seeker::ServerAttackMontage_Implementation()
{
	Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
	MulticastPlayComboSection();
}

void AGS_Seeker::MulticastPlayComboSection_Implementation()
{
	FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), CurrentComboIndex + 1));

	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		if (HasAuthority())
		{
			CurrentComboIndex++;
			CanAcceptComboInput = false;
			bNextCombo = false;
		}
		AnimInstance->Montage_Play(ComboAnimMontage);
		AnimInstance->Montage_JumpToSection(SectionName, ComboAnimMontage);
	}
}

void AGS_Seeker::HandleLowHealthEffect(UGS_StatComp* InStatComp)
{
	if (!IsLocallyControlled() || !InStatComp || !LowHealthDynamicMaterial)
	{
		return;
	}

	float CurrentHealth = InStatComp->GetCurrentHealth();
	float MaxHealth = InStatComp->GetMaxHealth();
	float HealthRatio = CurrentHealth / FMath::Max(1.0f, MaxHealth);

	bool bShouldBeLowHealth = HealthRatio <= LowHealthThresholdRatio && CurrentHealth > KINDA_SMALL_NUMBER;

	if (bShouldBeLowHealth)
	{
		// 효과 활성화
		if (!bIsLowHealthEffectActive)
		{
			CurrentEffectStrength = 0.0f;
			bIsLowHealthEffectActive = true;
			LowHealthPostProcessComp->bEnabled = true;
			
			GetWorldTimerManager().SetTimer(
			   LowHealthEffectTimer,
			   this,
			   &AGS_Seeker::UpdateLowHealthEffect,
			   0.1f,
			   true
			);
		}
		TargetEffectStrength = 1.0f - HealthRatio;
	}
	else
	{
		// HP가 임계값 이상으로 회복되면 효과 즉시 OFF
		if (bIsLowHealthEffectActive)
		{
			bIsLowHealthEffectActive = false;
			TargetEffectStrength = 0.0f;
			CurrentEffectStrength = 0.0f;
			UpdatePostProcessEffect(0.0f);
			LowHealthPostProcessComp->bEnabled = false;

			GetWorldTimerManager().ClearTimer(LowHealthEffectTimer);
		}
	}
}

void AGS_Seeker::UpdateLowHealthEffect()
{
	if (!bIsLowHealthEffectActive || !LowHealthDynamicMaterial)
	{
		GetWorldTimerManager().ClearTimer(LowHealthEffectTimer);
		return;
	}

	if (UGS_StatComp* OwnerStatComp = FindComponentByClass<UGS_StatComp>())
	{
		float HealthRatio = OwnerStatComp->GetCurrentHealth() / FMath::Max(1.0f, OwnerStatComp->GetMaxHealth());
		HealthRatio = FMath::Clamp(HealthRatio, 0.0f, 1.0f);
       
		// 목표 효과 강도 계산
		TargetEffectStrength = 1.0f - HealthRatio;

		// 부드러운 보간
		CurrentEffectStrength = FMath::FInterpTo(
			CurrentEffectStrength,
			TargetEffectStrength,
			0.1f,
			EffectInterpSpeed
		);
       
		UpdatePostProcessEffect(CurrentEffectStrength);

		// 효과가 충분히 작아지면 PostProcess 비활성화
		if (!bIsLowHealthEffectActive && CurrentEffectStrength < KINDA_SMALL_NUMBER)
		{
			LowHealthPostProcessComp->bEnabled = false;
			UpdatePostProcessEffect(0.0f); // 혹시 모를 잔상 방지
			GetWorldTimerManager().ClearTimer(LowHealthEffectTimer);
		}
	}
	else 
	{
		GetWorldTimerManager().ClearTimer(LowHealthEffectTimer);
	}
}

void AGS_Seeker::OnRep_SeekerGait()
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		if (UGS_ChooserInputObj* InputObj = AnimInstance->ChooserInputObj)
		{
			InputObj->Gait = SeekerGait;
		}
	}
}

void AGS_Seeker::Multicast_SetMontageSlot_Implementation(ESeekerMontageSlot InputMontageSlot)
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->SetCurMontageSlot(InputMontageSlot);
	}
}

void AGS_Seeker::Multicast_SetMustTurnInPlace_Implementation(bool MustTurn)
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->SetMustTurnInPlace(MustTurn);
	}
}

/*void AGS_Seeker::Multicast_SetIsFullBodySlot_Implementation(bool bFullBodySlot)
{
	if (!IsValid(this) || !GetWorld() || GetWorld()->bIsTearingDown || GetWorld()->IsInSeamlessTravel())
	{
		return;
	}

	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingFullBodyMontage = bFullBodySlot;
	}
}*/

/*void AGS_Seeker::Multicast_SetIsUpperBodySlot_Implementation(bool bUpperBodySlot)
{
	if (!IsValid(this) || !GetWorld() || GetWorld()->bIsTearingDown || GetWorld()->IsInSeamlessTravel())
	{
		return;
	}

	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->IsPlayingUpperBodyMontage = bUpperBodySlot;
	}
}*/

void AGS_Seeker::OnRep_IsLowHealthEffectActive()
{
	if (LowHealthPostProcessComp)
	{
		LowHealthPostProcessComp->bEnabled = bIsLowHealthEffectActive;
	}
}

void AGS_Seeker::OnRep_CurrentEffectStrength()
{
	UpdatePostProcessEffect(CurrentEffectStrength);
}

// ============================
// 상태 전환에 따른 음악 함수 관련
// ============================

// 몬스터 감지 시스템 (시커가 몬스터를 감지)
void AGS_Seeker::OnCombatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AGS_Monster::StaticClass()))
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(OtherActor))
		{
			AddCombatMonster(Monster);
			
			if (UGS_HPTextWidgetComp* HPWidgetComp = Monster->FindComponentByClass<UGS_HPTextWidgetComp>())
			{
				HPWidgetComp->SetVisibility(true);
			}
		}
	}
}

void AGS_Seeker::OnCombatTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(AGS_Monster::StaticClass()))
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(OtherActor))
		{
			RemoveCombatMonster(Monster);

			if (UGS_HPTextWidgetComp* HPWidgetComp = Monster->FindComponentByClass<UGS_HPTextWidgetComp>())
			{
				HPWidgetComp->SetVisibility(false);
			}
		}
	}
}

void AGS_Seeker::AddCombatMonster(AGS_Monster* Monster)
{
	if (!Monster)
	{
		return;
	}
	
	if (!NearbyMonsters.Contains(Monster))
	{
		NearbyMonsters.Add(Monster);
		
		// 첫 번째 몬스터가 추가되면 음악 시작
		if (NearbyMonsters.Num() == 1)
		{
			StartCombatMusic();
		}
	}
}

void AGS_Seeker::RemoveCombatMonster(AGS_Monster* Monster)
{
	if (!Monster)
	{
		return;
	}
	
	NearbyMonsters.Remove(Monster);
	
	// 모든 몬스터가 제거되면 음악 중지
	if (NearbyMonsters.Num() == 0)
	{
		ClientRPCStopCombatMusic();
	}
}

void AGS_Seeker::StartCombatMusic()
{
	if (!IsLocallyControlled() || NearbyMonsters.Num() == 0 || !NearbyMonsters[0])
	{
		return;
	}

	// AudioManager 가져오기
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGS_AudioManager* AudioManager = GameInstance->GetSubsystem<UGS_AudioManager>())
		{
			UAkAudioEvent* CombatStartEvent = NearbyMonsters[0]->CombatMusicEvent;
			UAkAudioEvent* CombatStopEvent = NearbyMonsters[0]->CombatMusicStopEvent;

			if (CombatStartEvent)
			{
				AudioManager->StartCombatSequence(this, CombatStartEvent, CombatStopEvent);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::StartCombatMusic - Monster has no CombatMusicEvent."));
			}
		}
	}
}

void AGS_Seeker::ClientRPCStopCombatMusic_Implementation()
{
	// AudioManager 가져오기
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGS_AudioManager* AudioManager = GameInstance->GetSubsystem<UGS_AudioManager>())
		{
			// 현재 재생 중인 전투 BGM 이벤트 가져오기 (가장 마지막에 추가된 몬스터 기준 또는 다른 로직)
			UAkAudioEvent* CombatStopEventToUse = nullptr;
			if (AudioManager->GetCurrentCombatMusicStopEvent()) // AudioManager에 저장된 StopEvent가 우선
			{
				CombatStopEventToUse = AudioManager->GetCurrentCombatMusicStopEvent();
			}
			else if (!NearbyMonsters.IsEmpty() && NearbyMonsters.Last()->CombatMusicStopEvent) // 몬스터 배열에서 가져오기
			{
				CombatStopEventToUse = NearbyMonsters.Last()->CombatMusicStopEvent;
			}

			// EndCombatSequence 호출 시 CombatStopEvent도 전달
			AudioManager->EndCombatSequence(this, CombatStopEventToUse);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGS_Seeker::StopCombatMusic() - AudioManager not found"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGS_Seeker::StopCombatMusic() - GameInstance not found"));
	}
}

void AGS_Seeker::UpdateCombatMusicState()
{
	// 유효하지 않은 몬스터들 제거
	NearbyMonsters.RemoveAll([](AGS_Monster* Monster)
	{
		return !IsValid(Monster);
	});
	
	// 몬스터가 없으면 음악 중지
	if (NearbyMonsters.Num() == 0)
	{
		ClientRPCStopCombatMusic();
	}
}

void AGS_Seeker::OnDeath()
{
	// 시커 죽음 사운드 재생
	if (SeekerAudioComponent)
	{
		SeekerAudioComponent->PlayDeathSound();
	}
	
	Super::OnDeath();
	
	ClientRPCStopCombatMusic();
	NearbyMonsters.Empty();
}

void AGS_Seeker::HandleAliveStatusChanged(AGS_PlayerState* ChangedPlayerState, bool bIsNowAlive)
{
	if (!IsLocallyControlled()) 
	{
		return;
	}

	// 자신의 PlayerState인지 확인
	AGS_PlayerState* MyPlayerState = GetPlayerState<AGS_PlayerState>();
	if (ChangedPlayerState != MyPlayerState) 
	{
		return;
	}

	if (!bIsNowAlive) // 자신이 죽었을 때
	{
		ClientRPCStopCombatMusic();
		NearbyMonsters.Empty();
	}
}

void AGS_Seeker::Server_RestKey_Implementation()
{
	SetAimState(false);
	SetDrawState(false);
	CanAcceptComboInput = true;
	CanChangeSeekerGait = true;
	Multicast_SetMontageSlot(ESeekerMontageSlot::None);
	SetMoveControlValue(true, true);
	SetLookControlValue(true, true);
}

void AGS_Seeker::Multicast_PlaySound_Implementation(UAkAudioEvent* SoundToPlay)
{
	if (SeekerAudioComponent && IsValid(SeekerAudioComponent))
	{
		SeekerAudioComponent->PlayGenericSound(SoundToPlay);
	}

}

void AGS_Seeker::OnHoverBegin()
{
	Super::OnHoverBegin();

	OnSeekerHover.Broadcast(true);
}

void AGS_Seeker::OnHoverEnd()
{
	Super::OnHoverEnd();

	OnSeekerHover.Broadcast(false);
}

FLinearColor AGS_Seeker::GetCurrentDecalColor()
{
	return FLinearColor::Red;
}

bool AGS_Seeker::ShowDecal()
{
	return true;
}

// ================
// 가디언 감지 시스템
// ================

void AGS_Seeker::OnDetectedByGuardian(bool bIsDetected)
{
	// 서버에서만 호출되어야 함
	if (HasAuthority())
	{
		// 상태 변경 시 자동으로 OnRep_IsDetectedByGuardian이 모든 클라이언트에서 호출됨
		bIsDetectedByGuardian = bIsDetected;

		// 감지 해제 시 강도도 0으로 초기화
		if (!bIsDetected)
		{
			DetectionIntensity = 0.0f;
		}
	}
}

void AGS_Seeker::SetDetectionIntensity(float Intensity)
{
	if (HasAuthority())
	{
		DetectionIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	}
}

void AGS_Seeker::OnRep_IsDetectedByGuardian()
{
	// 로컬 플레이어의 시커에만 효과 적용
	if (!IsLocallyControlled())
	{
		return;
	}

	// 시각적 효과 업데이트 (항상 실행)
	UpdateDetectionEffects();

	// 청각적 피드백
	if (!SeekerAudioComponent)
	{
		return;
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	float TimeSinceLastSound = CurrentTime - LastDetectionSoundTime;

	if (bIsDetectedByGuardian)
	{
		if (TimeSinceLastSound >= DetectionSoundCooldown)
		{
			SeekerAudioComponent->PlayDetectionWarningSound();
		}
	}
	else
	{
		SeekerAudioComponent->PlayDetectionClearedSound();
		LastDetectionSoundTime = CurrentTime;
	}
}

void AGS_Seeker::OnRep_DetectionIntensity()
{
	// 로컬 플레이어의 시커에만 포스트 프로세스 효과 적용
	if (!IsLocallyControlled())
	{
		return;
	}

	// 포스트 프로세스 효과 강도 업데이트
	UpdateDetectionPostProcessEffect(DetectionIntensity);
}

void AGS_Seeker::UpdateDetectionEffects()
{
	if (bIsDetectedByGuardian)
	{
		// 감지되었을 때 - 블루프린트에서 HUD 위젯 표시
		// BP_Seeker에서 이벤트 바인딩하여 처리
		UpdateDetectionHUD();

		// 감지 전용 포스트 프로세스 활성화
		if (DetectionPostProcessComp)
		{
			DetectionPostProcessComp->bEnabled = true;
		}
	}
	else
	{
		// 감지 해제 시 - 블루프린트에서 HUD 위젯 숨김
		UpdateDetectionHUD();

		// 감지 전용 포스트 프로세스 비활성화
		if (DetectionPostProcessComp)
		{
			DetectionPostProcessComp->bEnabled = false;
		}

		// 효과 강도 0으로 초기화
		UpdateDetectionPostProcessEffect(0.0f);
	}
}

void AGS_Seeker::UpdateDetectionPostProcessEffect(float Intensity)
{
	if (!IsLocallyControlled() || !DetectionDynamicMaterial)
	{
		return;
	}

	// MPP_Detect 머티리얼 파라미터 업데이트
	DetectionDynamicMaterial->SetScalarParameterValue(TEXT("DetectionIntensity"), Intensity);
}

void AGS_Seeker::UpdateDetectionHUD()
{
	// 실제 HUD 표시/숨김은 블루프린트에서 이벤트로 처리됨
}