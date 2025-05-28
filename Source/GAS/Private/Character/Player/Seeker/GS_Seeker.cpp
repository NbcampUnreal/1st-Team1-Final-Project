// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"


// Sets default values
AGS_Seeker::AGS_Seeker()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Post Process Component 생성 및 설정
	LowHealthPostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("LowHealthPostProcessComp"));
	LowHealthPostProcessComp->SetupAttachment(CameraComp);
	LowHealthPostProcessComp->bEnabled = false;
	LowHealthPostProcessComp->Priority = 10; // 다른 PP보다 높게 설정하여 우선 적용
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

// Called when the game starts or when spawned
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
				// PostProcessComponent에 머티리얼 적용
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
	HealthRatio = FMath::Clamp(HealthRatio, 0.0f, 1.0f);

	bool bShouldBeLowHealth = HealthRatio <= LowHealthThresholdRatio && CurrentHealth > KINDA_SMALL_NUMBER;

	if (bShouldBeLowHealth)
	{
		float EffectStrength = 1.0f - HealthRatio;
		UpdatePostProcessEffect(EffectStrength);
		LowHealthPostProcessComp->bEnabled = true;
		bIsLowHealthEffectActive = true;
	}
	else
	{
		LowHealthPostProcessComp->bEnabled = false;
		bIsLowHealthEffectActive = false;
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

// Called every frame
void AGS_Seeker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsLowHealthEffectActive && LowHealthDynamicMaterial)
	{
		if (UGS_StatComp* OwnerStatComp = FindComponentByClass<UGS_StatComp>())
		{
			float HealthRatio = OwnerStatComp->GetCurrentHealth() / FMath::Max(1.0f, OwnerStatComp->GetMaxHealth());
			HealthRatio = FMath::Clamp(HealthRatio, 0.0f, 1.0f);
			
			float EffectStrength = 1.0f - HealthRatio;
			UpdatePostProcessEffect(EffectStrength);
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

