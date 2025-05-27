// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
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

	if (SkillInputHandlerComponent)
	{
		SkillInputHandlerComponent->SetupEnhancedInput(InputComponent);
	}

	// 스탯 컴포넌트 가져와서 델리게이트 바인딩
	UGS_StatComp* FoundStatComp = FindComponentByClass<UGS_StatComp>();
	if (FoundStatComp)
	{
		// 체력 변화 델리게이트에 함수 바인딩
		FoundStatComp->OnCurrentHPChanged.AddUObject(this, &AGS_Seeker::HandleLowHealthEffect);
	}
}

// Called every frame
void AGS_Seeker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// HP 비율에 따라 효과 강도 조절 (효과가 활성화된 상태에서만)
	if (bIsLowHealthEffectActive && LowHealthPostProcessComp && LowHealthPostProcessComp->bEnabled)
	{
        UGS_StatComp* OwnerStatComp = FindComponentByClass<UGS_StatComp>();
        if (OwnerStatComp)
        {
            float HealthRatio = OwnerStatComp->GetCurrentHealth() / FMath::Max(1.0f, OwnerStatComp->GetMaxHealth());
            HealthRatio = FMath::Clamp(HealthRatio, 0.0f, 1.0f);
        	
            float EffectStrength = 1.0f - HealthRatio; // HP가 낮아질수록 효과가 강해지게 구현

            // Material Parameter Collection 'HPRatio' 파라미터 업데이트
            UMaterialParameterCollection* MPC_LowHPEffect = LoadObject<UMaterialParameterCollection>(
            	nullptr,
            	TEXT("/Game/Material/MPC_LowHPEffect.MPC_LowHPEffect")
            	);
            if (MPC_LowHPEffect)
            {
                UKismetMaterialLibrary::SetScalarParameterValue(
                	GetWorld(),
                	MPC_LowHPEffect, 
                	TEXT("HPRatio"),
                	EffectStrength
                	);
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

void AGS_Seeker::HandleLowHealthEffect(UGS_StatComp* InStatComp)
{
	// 로컬 플레이어만 처리하도록 분기
	if (!IsLocallyControlled() || !InStatComp)
	{
		return;
	}

    float CurrentHealth = InStatComp->GetCurrentHealth();
    float MaxHealth = InStatComp->GetMaxHealth();

	float HealthRatio = CurrentHealth / FMath::Max(1.0f, MaxHealth);
	HealthRatio = FMath::Clamp(HealthRatio, 0.0f, 1.0f);

	// HP가 30% 이하일 때만 효과 활성화
	bool bShouldBeLowHealth = HealthRatio <= LowHealthThresholdRatio && CurrentHealth > KINDA_SMALL_NUMBER;

	if (bShouldBeLowHealth)
	{
		// 저체력 상태: 효과 활성화 및 강도 조절
		if (LowHealthPostProcessComp && LowHealthEffectMaterial)
		{
            UMaterialInstanceDynamic* DynamicMaterial = nullptr;
            if (LowHealthPostProcessComp->Settings.WeightedBlendables.Array.Num() > 0)
            {
                 DynamicMaterial = Cast<UMaterialInstanceDynamic>(LowHealthPostProcessComp->Settings.WeightedBlendables.Array[0].Object);
            }

			if (!DynamicMaterial || DynamicMaterial->Parent != LowHealthEffectMaterial)
			{
				DynamicMaterial = UMaterialInstanceDynamic::Create(LowHealthEffectMaterial, this);
                LowHealthPostProcessComp->Settings.WeightedBlendables.Array.Empty();
				LowHealthPostProcessComp->Settings.AddBlendable(DynamicMaterial, 1.0f);
			}
			
            float EffectStrength = 1.0f - HealthRatio; // HP가 낮아질수록 효과가 강해지도록 설정

            UMaterialParameterCollection* MPC_LowHPEffect = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Material/MPC_LowHPEffect.MPC_LowHPEffect"));
            if (MPC_LowHPEffect)
            {
                UKismetMaterialLibrary::SetScalarParameterValue(
                	GetWorld(),
                	MPC_LowHPEffect,
                	TEXT("HPRatio"),
                	EffectStrength
                	);
            }
			LowHealthPostProcessComp->bEnabled = true;
			bIsLowHealthEffectActive = true;
		}
	}
	else
	{
		// 저체력 상태가 아닐 때: 효과 비활성화
		if (LowHealthPostProcessComp)
		{
			LowHealthPostProcessComp->bEnabled = false;
			bIsLowHealthEffectActive = false;

            // 효과 해제 시 Material Parameter Collection 값 초기화
            UMaterialParameterCollection* MPC_LowHPEffect = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Material/MPC_LowHPEffect.MPC_LowHPEffect"));
            if (MPC_LowHPEffect)
            {
                UKismetMaterialLibrary::SetScalarParameterValue(
                	GetWorld(),
                	MPC_LowHPEffect,
                	TEXT("HPRatio"),
                	0.0f
                	);
            }
		}
	}
}

