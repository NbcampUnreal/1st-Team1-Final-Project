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
#include "AkGameplayStatics.h"
#include "Character/Player/Monster/GS_Monster.h"

// Sets default values
AGS_Seeker::AGS_Seeker()
{
	PrimaryActorTick.bCanEverTick = true;

	// 전투 음악 관련 초기화
	CurrentCombatMusicID = 0;
	ActiveCombatMusicStopEvent = nullptr;

	// Post Process Component 생성 및 설정
	LowHealthPostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("LowHealthPostProcessComp"));
	LowHealthPostProcessComp->SetupAttachment(CameraComp);
	LowHealthPostProcessComp->bEnabled = false;
	LowHealthPostProcessComp->Priority = 10;

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

/*EGait AGS_Seeker::GetGait()
{
	return Gait;
}*/

void AGS_Seeker::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		InitializeCameraManager();
		
		// 스탯 컴포넌트 가져와서 델리게이트 바인딩
		if (UGS_StatComp* FoundStatComp = FindComponentByClass<UGS_StatComp>())
		{
			FoundStatComp->OnCurrentHPChanged.AddUObject(this, &AGS_Seeker::HandleLowHealthEffect);
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

void AGS_Seeker::UpdatePostProcessEffect(float EffectStrength)
{
	if (LowHealthDynamicMaterial)
	{
		LowHealthDynamicMaterial->SetScalarParameterValue(TEXT("HPRatio"), EffectStrength);
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
		StopCombatMusic();
	}
}

void AGS_Seeker::StartCombatMusic()
{
	// 로컬 클라이언트에서만 실행
	if (!IsLocallyControlled())
	{
		return;
	}
	
	// 이미 재생 중이면 중복 재생 방지
	if (CurrentCombatMusicID != 0)
	{
		return;
	}
	
	// 몬스터에서 음악 이벤트 가져오기
	if (NearbyMonsters.Num() > 0 && NearbyMonsters[0])
	{
		if (UAkAudioEvent* CombatMusicEvent = NearbyMonsters[0]->CombatMusicEvent)
		{
			// 전투 음악 재생
			CurrentCombatMusicID = UAkGameplayStatics::PostEvent(CombatMusicEvent, this, 0, FOnAkPostEventCallback());
			ActiveCombatMusicStopEvent = NearbyMonsters[0]->CombatMusicStopEvent;
		}
	}
}

void AGS_Seeker::StopCombatMusic()
{
	// 로컬 클라이언트에서만 실행
	if (!IsLocallyControlled())
	{
		return;
	}
	
	if (CurrentCombatMusicID == 0)
	{
		return;
	}
	
	// 저장된 Stop Event를 사용하여 Fade Out과 함께 음악 중지
	if (ActiveCombatMusicStopEvent)
	{
		UAkGameplayStatics::PostEvent(ActiveCombatMusicStopEvent, this, 0, FOnAkPostEventCallback());
	}
	else
	{
		// Stop Event가 없는 경우, 중지하고 경고 로그
		UE_LOG(LogTemp, Warning, TEXT("AGS_Seeker::StopCombatMusic - ActiveCombatMusicStopEvent is null. Stopping music immediately."));
		UAkGameplayStatics::StopActor(this);
	}

	CurrentCombatMusicID = 0;
	ActiveCombatMusicStopEvent = nullptr;
}

void AGS_Seeker::UpdateCombatMusicState()
{
	// 유효하지 않은 몬스터들 제거
	NearbyMonsters.RemoveAll([](AGS_Monster* Monster)
	{
		return !IsValid(Monster);
	});
	
	// 몬스터가 없으면 음악 중지
	if (NearbyMonsters.Num() == 0 && CurrentCombatMusicID != 0)
	{
		StopCombatMusic();
	}
}

