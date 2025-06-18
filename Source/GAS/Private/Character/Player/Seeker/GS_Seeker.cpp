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
#include "Character/Component/GS_DebuffVFXComponent.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"
#include "Character/GS_TpsController.h"

// Sets default values
AGS_Seeker::AGS_Seeker()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	GetMesh()->bEnableUpdateRateOptimizations = false;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->bOnlyAllowAutonomousTickPose = false;

	// Post Process Component 생성 및 설정
	LowHealthPostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("LowHealthPostProcessComp"));
	LowHealthPostProcessComp->SetupAttachment(CameraComp);
	LowHealthPostProcessComp->bEnabled = false;
	LowHealthPostProcessComp->Priority = 10;

	// =======================
	// 디버프 VFX 컴포넌트 생성
	// =======================
	DebuffVFXComponent = CreateDefaultSubobject<UGS_DebuffVFXComponent>("DebuffVFXComponent");

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

void AGS_Seeker::SetAimState(bool IsAim)
{
	SeekerState.IsAim = IsAim;
}

bool AGS_Seeker::GetAimState()
{
	return SeekerState.IsAim;
}

void AGS_Seeker::SetDrawState(bool IsDraw)
{
	SeekerState.IsDraw = IsDraw;
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

void AGS_Seeker::Multicast_ComboEnd_Implementation()
{
	if (UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		StopAnimMontage(ComboAnimMontage);
		AnimInstance->IsPlayingUpperBodyMontage = false;
		CurrentComboIndex = 0;
		CanAcceptComboInput = true;
		

		AGS_TpsController* TPSController = Cast<AGS_TpsController>(GetController());
		if (IsValid(TPSController))
		{
			TPSController->SetLookControlValue(true, true);
		}

		if (HasAuthority())
		{
			if (LastSeekerGait == EGait::Run)
			{			
				SetSeekerGait(EGait::Run);
			}
			else if (LastSeekerGait == EGait::Walk)
			{
				SetSeekerGait(EGait::Walk);
			}
		}
	}
}

void AGS_Seeker::Server_ComboEnd_Implementation(bool bComboEnd)
{
	bComboEnded = bComboEnd;
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

const FName AGS_Seeker::HPRatioParamName = TEXT("HPRatio");
const FName AGS_Seeker::EffectIntensityParamName = TEXT("EffectIntensity");

void AGS_Seeker::InitializeCameraManager()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		LocalCameraManager = PC->PlayerCameraManager;
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
	}
}

void AGS_Seeker::ComboInputOpen()
{
	CanAcceptComboInput = true;
}

void AGS_Seeker::ComboInputClose()
{
	CanAcceptComboInput = false;
	if (bNextCombo)
	{
		ServerAttackMontage();  
		CanAcceptComboInput = false;
		bNextCombo = false;
	}
}

void AGS_Seeker::OnComboAttack()
{
	// SJE
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
	MulticastPlayComboSection();
}

void AGS_Seeker::MulticastPlayComboSection_Implementation()
{
	// SJE
	UE_LOG(LogTemp, Warning, TEXT("%s Multicat Play Combo Section"), *GetName());
	FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), CurrentComboIndex + 1));
	UGS_SeekerAnimInstance* AnimInstance = Cast<UGS_SeekerAnimInstance>(GetMesh()->GetAnimInstance());
	
	CurrentComboIndex++;
	if (AnimInstance && ComboAnimMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalRole: %s"), *UEnum::GetValueAsString(GetLocalRole()));

		AnimInstance->Montage_Play(ComboAnimMontage);
		AnimInstance->IsPlayingUpperBodyMontage = true;
		AnimInstance->Montage_JumpToSection(SectionName, ComboAnimMontage);
	}
	CanAcceptComboInput = false;
	bNextCombo = false;
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
		}
	}
}

void AGS_Seeker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

void AGS_Seeker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsLowHealthEffectActive && LowHealthDynamicMaterial)
	{
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
				DeltaTime,
				EffectInterpSpeed
			);
			
			UpdatePostProcessEffect(CurrentEffectStrength);

			// 효과가 충분히 작아지면 PostProcess 비활성화
			if (!bIsLowHealthEffectActive && CurrentEffectStrength < KINDA_SMALL_NUMBER)
			{
				LowHealthPostProcessComp->bEnabled = false;
				UpdatePostProcessEffect(0.0f); // 혹시 모를 잔상 방지
			}
		}
	}
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
	DOREPLIFETIME(AGS_Seeker, bComboEnded);
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

// =================
// 전투 음악 관리 함수
// =================

// 새로운 몬스터 감지 시스템 (시커가 몬스터를 감지)
void AGS_Seeker::OnCombatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AGS_Monster::StaticClass()))
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(OtherActor))
		{
			AddCombatMonster(Monster);
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
	// 죽었을 때는 IsLocallyControlled() 체크를 하지 않음
	//UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::StopCombatMusic() called for %s"), *GetName());

	// AudioManager 가져오기
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGS_AudioManager* AudioManager = GameInstance->GetSubsystem<UGS_AudioManager>())
		{
			//UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::StopCombatMusic() - Calling EndCombatSequence"));
			
			// 현재 재생 중인 전투 BGM 이벤트 가져오기 (가장 마지막에 추가된 몬스터 기준 또는 다른 로직)
			UAkAudioEvent* CombatStopEventToUse = nullptr;
			if (AudioManager->GetCurrentCombatMusicStopEvent()) // AudioManager에 저장된 StopEvent가 우선
			{
				CombatStopEventToUse = AudioManager->GetCurrentCombatMusicStopEvent();
				UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::StopCombatMusic - Using StopEvent from AudioManager: %s"), *CombatStopEventToUse->GetName());
			}
			else if (!NearbyMonsters.IsEmpty() && NearbyMonsters.Last()->CombatMusicStopEvent) // 몬스터 배열에서 가져오기
			{
				CombatStopEventToUse = NearbyMonsters.Last()->CombatMusicStopEvent;
				UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::StopCombatMusic - Using StopEvent from Last Monster: %s"), *CombatStopEventToUse->GetName());
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
	UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::OnDeath() called for %s"), *GetName());
	Super::OnDeath();
	
	// 확실하게 BGM 끄기 (백업용)
	UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::OnDeath() - Stopping combat music"));
	ClientRPCStopCombatMusic();
	NearbyMonsters.Empty();
}

void AGS_Seeker::HandleAliveStatusChanged(AGS_PlayerState* ChangedPlayerState, bool bIsNowAlive)
{
	UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::HandleAliveStatusChanged() called for %s"), *GetName());
	
	if (!IsLocallyControlled()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::HandleAliveStatusChanged() - Not locally controlled"));
		return;
	}

	// 자신의 PlayerState인지 확인
	AGS_PlayerState* MyPlayerState = GetPlayerState<AGS_PlayerState>();
	if (ChangedPlayerState != MyPlayerState) 
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::HandleAliveStatusChanged() - Not my PlayerState"));
		return;
	}

	if (!bIsNowAlive) // 자신이 죽었을 때
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::HandleAliveStatusChanged() - Player died, stopping combat music"));
		ClientRPCStopCombatMusic();
		NearbyMonsters.Empty();
	}
}

